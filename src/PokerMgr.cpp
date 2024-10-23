#include "PokerMgr.h"

PokerMgr::PokerMgr()
{
    table.clear();
    status = POKER_STATUS_INACTIVE;
    sidepots.clear();
    deck.clear();
    round = 0;
    button = 1;
    turn = 0;
    flop = {0, 0, 0, 0, 0};
    delay = 1000;
}

PokerMgr::~PokerMgr()
{
    table.clear();
    status = POKER_STATUS_INACTIVE;
    deck.clear();
    round = 0;
    button = 1;
    turn = 0;
    flop = {0, 0, 0, 0, 0};
    delay = 1000;
}

JoinResult PokerMgr::PlayerJoin(Player *player, uint32 gold)
{
    if (!player)
        return POKER_JOIN_ERROR_NO_PLAYER;
    if (gold < POKER_MIN_GOLD || gold > POKER_MAX_GOLD)
        return POKER_JOIN_ERROR_MONEY_OUT_OF_RANGE;
    if (GetSeat(player) > 0)
        return POKER_JOIN_ERROR_SEATED;
    if (player->GetMoney() < gold * GOLD)
        return POKER_JOIN_ERROR_NO_ENOUGH_MONEY;
    uint32 seat = GetSeatAvailable();
    if (seat == 0)
        return POKER_JOIN_ERROR_NO_SEATS;
    table[seat] = new PokerPlayer(player);
    player->SetMoney(player->GetMoney() - gold * GOLD);
    table[seat]->SetChips(gold * GOLD);
    return POKER_JOIN_OK;
}

void PokerMgr::PlayerLeave(Player *player, bool logout)
{
    uint32 seat = sPokerMgr->GetSeat(player);
    if (seat > 0)
    {
        if (table[seat]->GetChips() == 0)
            return;
        uint32 allowedMoney = MAX_MONEY_AMOUNT - player->GetMoney();
        if (!logout)
        {
            if (table[seat]->GetChips() <= allowedMoney)
            {
                player->SetMoney(player->GetMoney() + table[seat]->GetChips());
                table[seat]->SetChips(0);
            }
            else
            {
                player->SetMoney(player->GetMoney() + allowedMoney);
                table[seat]->SetChips(table[seat]->GetChips() - allowedMoney);
            }
        }
        if (table[seat]->GetChips() == 0)
            return;
        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
        std::string subject = "WoW Poker Lerduzz";
        std::ostringstream body;
        body << (logout ? "Te has desconectado durante una partida" : "Has abandonado la mesa");
        body << " de poker.\n\t\n\tEn este correo te enviamos el dinero que no se te pudo entregar directamente.";
        while (table[seat]->GetChips() > 0)
        {
            MailDraft draft(subject, body.str().c_str());
            MailSender sender(MAIL_NORMAL, player->GetGUID().GetCounter(), MAIL_STATIONERY_GM);
            if (table[seat]->GetChips() > MAX_MONEY_AMOUNT)
            {
                draft.AddMoney(MAX_MONEY_AMOUNT);
                table[seat]->SetChips(table[seat]->GetChips() - MAX_MONEY_AMOUNT);
            }
            else
            {
                draft.AddMoney(table[seat]->GetChips());
                table[seat]->SetChips(0);
            }
            draft.SendMailTo(trans, MailReceiver(player, player->GetGUID().GetCounter()), sender);
        }
        CharacterDatabase.CommitTransaction(trans);
    }
    BroadcastToTableLeaved(seat, logout);
    table.erase(seat);
}

uint32 PokerMgr::GetSeat(Player *player)
{
    for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
    {
        if (it->second && it->second->GetPlayer() && it->second->GetPlayer() == player)
            return it->first;
    }
    return 0;
}

PokerPlayer *PokerMgr::GetSeatInfo(uint32 seat)
{
    if (table.find(seat) != table.end())
        return table[seat];
    return nullptr;
}

void PokerMgr::InformPlayerJoined(Player *player)
{
    int32 delta = 5 - GetSeat(player);
    for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
    {
        if (it->second && it->second->GetPlayer())
        {
            int32 fakeseat = it->first + delta;
            if (fakeseat > POKER_MAX_SEATS)
                fakeseat -= POKER_MAX_SEATS;
            if (fakeseat < 1)
                fakeseat += POKER_MAX_SEATS;
            std::ostringstream resp;
            resp << POKER_PREFIX << "s_" << fakeseat << "_" << it->second->GetPlayer()->GetName() << "_";
            resp << it->second->GetChips() << "_" << it->second->GetBet() << "_" << (it->second->GetPlayer()->GetFaction() == 1 ? "A" : "H");
            player->Whisper(resp.str(), LANG_ADDON, player);
        }
    }
}

void PokerMgr::BroadcastToTable(std::string msg)
{
    for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
    {
        if (it->second && it->second->GetPlayer())
            it->second->GetPlayer()->Whisper(msg, LANG_ADDON, it->second->GetPlayer());
    }
}

void PokerMgr::BroadcastToTableJoined(uint32 seat)
{
    for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
    {
        if (it->second && it->second->GetPlayer() && it->first != seat)
        {
            int32 delta = 5 - it->first;
            int32 fakeseat = seat + delta;
            if (fakeseat > POKER_MAX_SEATS)
                fakeseat -= POKER_MAX_SEATS;
            if (fakeseat < 1)
                fakeseat += POKER_MAX_SEATS;
            std::ostringstream resp;
            resp << POKER_PREFIX << "s_" << fakeseat << "_" << table[seat]->GetPlayer()->GetName() << "_";
            resp << table[seat]->GetChips() << "_" << table[seat]->GetBet() << "_" << (table[seat]->GetPlayer()->GetFaction() == 1 ? "A" : "H");
            it->second->GetPlayer()->Whisper(resp.str(), LANG_ADDON, it->second->GetPlayer());
        }
    }
}

void PokerMgr::BroadcastToTableLeaved(uint32 seat, bool logout)
{
    for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
    {
        if (it->second && it->second->GetPlayer() && (!logout || it->first != seat))
        {
            int32 delta = 5 - it->first;
            int32 fakeseat = seat + delta;
            if (fakeseat > POKER_MAX_SEATS)
                fakeseat -= POKER_MAX_SEATS;
            if (fakeseat < 1)
                fakeseat += POKER_MAX_SEATS;
            std::ostringstream resp;
            resp << POKER_PREFIX << "q_" << fakeseat;
            it->second->GetPlayer()->Whisper(resp.str(), LANG_ADDON, it->second->GetPlayer());
        }
    }
}

void PokerMgr::BroadcastToTableDeal(uint32 seat)
{
    for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
    {
        if (it->second && it->second->GetPlayer() && it->first != seat)
        {
            int32 delta = 5 - it->first;
            int32 fakeseat = seat + delta;
            if (fakeseat > POKER_MAX_SEATS)
                fakeseat -= POKER_MAX_SEATS;
            if (fakeseat < 1)
                fakeseat += POKER_MAX_SEATS;
            std::ostringstream resp;
            resp << POKER_PREFIX << "deal_" << fakeseat;
            it->second->GetPlayer()->Whisper(resp.str(), LANG_ADDON, it->second->GetPlayer());
        }
    }
}

void PokerMgr::BroadcastToTablePlayerTurn(uint32 seat, uint32 maxBet)
{
    for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
    {
        if (it->second && it->second->GetPlayer())
        {
            int32 delta = 5 - it->first;
            int32 fakeseat = seat + delta;
            if (fakeseat > POKER_MAX_SEATS)
                fakeseat -= POKER_MAX_SEATS;
            if (fakeseat < 1)
                fakeseat += POKER_MAX_SEATS;
            std::ostringstream resp;
            resp << POKER_PREFIX << "go_" << fakeseat << "_" << maxBet;
            it->second->GetPlayer()->Whisper(resp.str(), LANG_ADDON, it->second->GetPlayer());
        }
    }
}

void PokerMgr::BroadcastToTablePlayerStatus(uint32 seat, std::string status)
{
    for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
    {
        if (it->second && it->second->GetPlayer())
        {
            int32 delta = 5 - it->first;
            int32 fakeseat = seat + delta;
            if (fakeseat > POKER_MAX_SEATS)
                fakeseat -= POKER_MAX_SEATS;
            if (fakeseat < 1)
                fakeseat += POKER_MAX_SEATS;
            std::ostringstream resp;
            resp << POKER_PREFIX << "st_" << fakeseat << "_" << table[seat]->GetChips();
            resp << "_" << table[seat]->GetBet() << "_" << status << "_1";
            it->second->GetPlayer()->Whisper(resp.str(), LANG_ADDON, it->second->GetPlayer());
        }
    }
}

void PokerMgr::BroadcastToTableShowCards(uint32 seat, std::string status)
{
    for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
    {
        if (it->second && it->second->GetPlayer() && it->first != seat)
        {
            int32 delta = 5 - it->first;
            int32 fakeseat = seat + delta;
            if (fakeseat > POKER_MAX_SEATS)
                fakeseat -= POKER_MAX_SEATS;
            if (fakeseat < 1)
                fakeseat += POKER_MAX_SEATS;
            std::ostringstream resp;
            resp << POKER_PREFIX << "show_" << table[seat]->GetHole1() << "_";
            resp << table[seat]->GetHole2() << "_" << fakeseat << "_" << status;
            it->second->GetPlayer()->Whisper(resp.str(), LANG_ADDON, it->second->GetPlayer());
        }
    }
}

void PokerMgr::BroadcastToTablePlayerStatusFolded(uint32 seat)
{
    for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
    {
        if (it->second && it->second->GetPlayer())
        {
            int32 delta = 5 - it->first;
            int32 fakeseat = seat + delta;
            if (fakeseat > POKER_MAX_SEATS)
                fakeseat -= POKER_MAX_SEATS;
            if (fakeseat < 1)
                fakeseat += POKER_MAX_SEATS;
            std::ostringstream resp;
            resp << POKER_PREFIX << "st_" << fakeseat << "_" << table[seat]->GetChips();
            resp << "_" << table[seat]->GetBet() << "_Folded_0.5";
            it->second->GetPlayer()->Whisper(resp.str(), LANG_ADDON, it->second->GetPlayer());
        }
    }
}

void PokerMgr::BroadcastToTableButton()
{
    for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
    {
        if (it->second && it->second->GetPlayer())
        {
            int32 delta = 5 - it->first;
            int32 fakebutton = button + delta;
            if (fakebutton > POKER_MAX_SEATS)
                fakebutton -= POKER_MAX_SEATS;
            if (fakebutton < 1)
                fakebutton += POKER_MAX_SEATS;
            std::ostringstream resp;
            resp << POKER_PREFIX << "b_" << fakebutton;
            it->second->GetPlayer()->Whisper(resp.str(), LANG_ADDON, it->second->GetPlayer());
        }
    }
}

void PokerMgr::BroadcastToTableWins(uint32 seat)
{
    for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
    {
        if (it->second && it->second->GetPlayer())
        {
            int32 delta = 5 - it->first;
            int32 fakeseat = seat + delta;
            if (fakeseat > POKER_MAX_SEATS)
                fakeseat -= POKER_MAX_SEATS;
            if (fakeseat < 1)
                fakeseat += POKER_MAX_SEATS;
            std::ostringstream resp;
            resp << POKER_PREFIX << "showdown_" << fakeseat << "_" << table[seat]->GetPlayer()->GetName() << " ha ganado."; // TODO: Traducir del lado del cliente.
            it->second->GetPlayer()->Whisper(resp.str(), LANG_ADDON, it->second->GetPlayer());
        }
    }
}

void PokerMgr::NextLevel()
{
    status = static_cast<PokerStatus>(static_cast<int>(status) + 1);
    switch (status)
    {
        case POKER_STATUS_PRE_FLOP:
        {
            DealHoleCards();
            break;
        }
        case POKER_STATUS_FLOP:
        {
            ShowFlopCards();
            break;
        }
        case POKER_STATUS_TURN:
        {
            DealTurn();
            break;
        }
        case POKER_STATUS_RIVER:
        {
            DealRiver();
            break;
        }
        case POKER_STATUS_SHOW:
        {
            ShowDown();
            break;
        }
    }
}

void PokerMgr::PlayerBet(uint32 seat, uint32 size, std::string status)
{
    if (table[seat]->GetChips() <= size)
    {
        size = table[seat]->GetChips();
        status = "All In";
    }

    table[seat]->SetChips(table[seat]->GetChips() - size);
    table[seat]->SetBet(table[seat]->GetBet() + size);

    BroadcastToTablePlayerStatus(seat, status);

    if (table[seat]->GetChips() == 0)
    {
        bool found = false;
        for (std::list<SidePot>::iterator it = sidepots.begin(); it != sidepots.end(); ++it)
            if (it->bet == table[seat]->GetBet())
                found = true;
        if (!found)
        {
            SidePot tmp = SidePot();
            tmp.bet = table[seat]->GetBet();
            tmp.pot = GetSidePot(table[seat]->GetBet());
            sidepots.push_back(tmp);
        }
    }
    for (std::list<SidePot>::iterator it = sidepots.begin(); it != sidepots.end(); ++it)
        it->pot = GetSidePot(it->bet);
}

void PokerMgr::PlayerAction(uint32 seat, uint32 delta)
{
    if (seat != turn)
        return;

    uint32 maxBet = HighestBet();

    if (table[seat]->GetBet() + delta < maxBet)
        delta = maxBet - table[seat]->GetBet();

    if (table[seat]->IsForcedBet())
    {
        if (delta > table[seat]->GetChips())
        {
            delta = table[seat]->GetChips();

            bool found = false;
            for (std::list<SidePot>::iterator it = sidepots.begin(); it != sidepots.end(); ++it)
                if (it->bet == delta)
                    found = true;
            if (!found)
            {
                SidePot tmp = SidePot();
                tmp.bet = delta;
                tmp.pot = GetSidePot(delta);
                sidepots.push_back(tmp);
            }
            for (std::list<SidePot>::iterator it = sidepots.begin(); it != sidepots.end(); ++it)
                it->pot = GetSidePot(it->bet);
        }
    }

    if (delta == 0)
        PlayerBet(seat, 0, "Checked");
    else if (delta > 0)
    {
        if (table[seat]->GetBet() + delta == maxBet)
            PlayerBet(seat, delta, "Called");
        else
        {
            if (delta >= table[seat]->GetChips())
            {
                delta = table[seat]->GetChips();
                PlayerBet(seat, delta, "All In");
            }
            else
                PlayerBet(seat, delta, "Raised");
        }
    }

    table[seat]->SetForcedBet(false);
    GoNextPlayerTurn();
}

void PokerMgr::FoldPlayer(uint32 seat)
{
    if (seat != turn)
        return;

    PokerPlayer *pp = sPokerMgr->GetSeatInfo(seat);
    if (pp && pp->IsDealt())
    {
        BroadcastToTablePlayerStatusFolded(seat);
        pp->SetDealt(false);
        pp->SetForcedBet(false);

        if (turn == seat)
            GoNextPlayerTurn();
        else
            if (GetPlayingPlayers() == 1)
                GoNextPlayerTurn();
    }
}

void PokerMgr::OnWorldUpdate(uint32 diff)
{
    if (delay >= diff)
    {
        delay -= diff;
        return;
    }
    if (status == POKER_STATUS_SHOW)
    {
        status = POKER_STATUS_INACTIVE;
        delay = 10000;
        return;
    }
    if (status == POKER_STATUS_INACTIVE)
        if (GetPlayablePlayers() > 1)
            sPokerMgr->NextLevel();
    delay = 1000;
}

uint32 PokerMgr::GetSeatAvailable()
{
    if (table.size() == POKER_MAX_SEATS)
        return 0;
    std::array<uint32, 9> seats = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint32 index = 0;
    for (uint32 i = 1; i <= 9; i++)
    {
        if (table.find(i) == table.end())
            seats[index++] = i;
    }
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(seats.begin(), seats.end(), std::default_random_engine(seed));
    for (uint32 i = 0; i < 9; i++)
    {
        if (seats[i] != 0)
            return seats[i];
    }
    return 0;
}

uint32 PokerMgr::WhosButtonAfter(uint32 start)
{
    for (uint32 i = 1; i <= 9; i++)
    {
        uint32 j = i + start;
        if (j > 9) j -= 9;
        if (j > 9) j -= 9;
        if (table.find(j) != table.end())
            if (table[j]->GetChips() > 0)
                return j;
    }
    return start;
}

uint32 PokerMgr::WhosBetAfter(uint32 start)
{
    uint32 maxBet = HighestBet();
    for (uint32 i = 1; i <= 9; i++)
    {
        uint32 j = i + start;
        if (j > 9) j -= 9;
        if (j > 9) j -= 9;
        if (table.find(j) != table.end())
            if (table[j]->GetChips() > 0 && table[j]->IsDealt())
                if (table[j]->GetBet() < maxBet || table[j]->IsForcedBet())
                    return j;
    }
    return 0;
}

void PokerMgr::SetupBets()
{
    for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
    {
        if (it->second && it->second->GetPlayer() && it->second->IsDealt())
            it->second->SetForcedBet(true);
    }
}

void PokerMgr::PostBlinds()
{
    uint32 pc = GetPlayingPlayers();
    uint32 smallBlind = POKER_BET_SIZE * GOLD / 2;
    uint32 bigBlind = POKER_BET_SIZE * GOLD;

    uint32 next = button;

    if (pc == 1)
    {
        PlayerBet(button, bigBlind, "Blinds");
        next = button;
    }
    else if (pc == 2)
    {
        uint32 j = WhosButtonAfter(button);
        PlayerBet(j, bigBlind, "Blinds");
        
        PlayerBet(button, smallBlind, "Blinds");
        
        next = j;
    }
    else if (pc > 2)
    {
        uint32 j = WhosButtonAfter(button);
        PlayerBet(j, smallBlind, "Small Blind");
        
        j = WhosButtonAfter(j);
        PlayerBet(j, bigBlind, "Big Blind");
        
        next = j;
    }
    turn = next;
    GoNextPlayerTurn();
}

uint32 PokerMgr::GetPlayingPlayers()
{
    uint32 count = 0;
    for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
    {
        if (it->second && it->second->GetPlayer() && it->second->IsDealt())
            count++;
    }
    return count;
}

uint32 PokerMgr::GetPlayablePlayers()
{
    uint32 count = 0;
    for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
        if (it->second && it->second->GetPlayer() && it->second->GetChips() > 0)
            count++;
    return count;
}

uint32 PokerMgr::HighestBet()
{
    uint32 maxBet = 0;
    for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
    {
        if (it->second && it->second->GetPlayer() && it->second->IsDealt())
            if (it->second->GetBet() > maxBet)
                maxBet = it->second->GetBet();
    }
    return maxBet;
}

uint32 PokerMgr::GetSidePot(uint32 bet)
{
    uint32 total = 0;
    uint32 r;
    for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
    {
        if (it->second && it->second->GetPlayer())
        {
            r = it->second->GetBet();
            if (r > bet)
                r = bet;
            total += r;
        }
    }
    return total;
}

uint32 PokerMgr::GetTotalPot()
{
    uint32 total = 0;
    for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
        if (it->second && it->second->GetPlayer())
            total += it->second->GetBet();
    return total;
}

void PokerMgr::GoNextPlayerTurn()
{
    turn = WhosBetAfter(turn);
    if (turn == 0)
    {
        NextLevel();
        return;
    }
    if (GetPlayingPlayers() == 1)
    {
        NextLevel();
        return;
    }
    uint32 maxBet = HighestBet();
    BroadcastToTablePlayerTurn(turn, maxBet);
}

PokerHandRank PokerMgr::FindHandForPlayer(uint32 seat)
{
    PokerHandRank hand = PokerHandRank();
    hand.hand = POKER_HAND_HIGH_CARD;
    if (table.find(seat) != table.end() && status > POKER_STATUS_INACTIVE)
    {
        std::list<uint32> inCards = { table[seat]->GetHole1(), table[seat]->GetHole2() };
        if (status >= POKER_STATUS_FLOP)
        {
            inCards.push_back(flop[0]);
            inCards.push_back(flop[1]);
            inCards.push_back(flop[2]);
        }
        if (status >= POKER_STATUS_TURN)
            inCards.push_back(flop[3]);
        if (status >= POKER_STATUS_RIVER)
            inCards.push_back(flop[4]);
        hand = sPokerHandMgr->BestRank(inCards);
    }
    return hand;
}

void PokerMgr::ShowCards(uint32 seat, std::string status)
{
    if (table.find(seat) == table.end())
        return;

	if (table[seat]->GetHole1() == 0 || table[seat]->GetHole2() == 0) 
        return;

	// FHS_BroadCastToTable("show_"..Seats[j].hole1 .."_"..Seats[j].hole2.."_"..j.."_"..status,j);
    BroadcastToTableShowCards(seat, status);
}

void PokerMgr::DealHoleCards()
{
    button = WhosButtonAfter(button);
    BroadcastToTableButton();

    deck.clear();
    std::array<uint32, 52> deck_arr;
    for (uint32 i = 1; i <= 52; i++)
    {
        deck_arr[i - 1] = i;
    }
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(deck_arr.begin(), deck_arr.end(), std::default_random_engine(seed));
    for (uint32 i = 1; i <= 52; i++)
    {
        deck.push_back(deck_arr[i - 1]);
    }

    std::ostringstream msg1;
    msg1 << POKER_PREFIX << "betsize_" << POKER_BET_SIZE;
    BroadcastToTable(msg1.str());

    sidepots.clear();

    round++;
    std::ostringstream msg2;
    msg2 << POKER_PREFIX << "round0_" << round;
    BroadcastToTable(msg2.str());

    for (uint32 i = 1; i <= 9; i++)
    {
        uint32 j = i + button + 1;
        if (j > 9) j -= 9;
        if (j > 9) j -= 9;
        if (table.find(j) != table.end())
        {
            table[j]->SetBet(0);
            if (table[j]->GetChips() > 0)
            {
                table[j]->SetHole1(deck.front());
                deck.pop_front();
                table[j]->SetHole2(deck.front());
                deck.pop_front();

                table[j]->SetDealt(true);

                std::ostringstream msg3;
                msg3 << POKER_PREFIX << "hole_" << table[j]->GetHole1() << "_" << table[j]->GetHole2();
                table[j]->GetPlayer()->Whisper(msg3.str(), LANG_ADDON, table[j]->GetPlayer());
                BroadcastToTableDeal(j);
            }
        }
    }

    std::ostringstream msg4;
    msg4 << POKER_PREFIX << "flop0";
    BroadcastToTable(msg4.str());

    SetupBets();
    PostBlinds();
}

void PokerMgr::ShowFlopCards()
{
    for (uint32 i = 0; i < 3; i++)
    {
        flop[i] = deck.front();
        deck.pop_front();
    }
    std::ostringstream msg;
    msg << POKER_PREFIX << "flop1_" << flop[0] << "_" << flop[1] << "_" << flop[2];        
    BroadcastToTable(msg.str());

    SetupBets();
    turn = button;
    GoNextPlayerTurn();
}

void PokerMgr::DealTurn()
{
    flop[3] = deck.front();
    deck.pop_front();

    std::ostringstream msg;
    msg << POKER_PREFIX << "turn_" << flop[3];        
    BroadcastToTable(msg.str());

    SetupBets();
    turn = button;
    GoNextPlayerTurn();
}

void PokerMgr::DealRiver()
{
    flop[4] = deck.front();
    deck.pop_front();

    std::ostringstream msg;
    msg << POKER_PREFIX << "river_" << flop[4];        
    BroadcastToTable(msg.str());

    SetupBets();
    turn = button;
    GoNextPlayerTurn();
}

void PokerMgr::ShowDown()
{
    std::list<uint32> winners;
    uint32 maxBet = HighestBet();
    uint32 totalPot = GetTotalPot();
    SidePot tmpPot;
    uint32 pot;

    if (sidepots.size() == 0)
    {
        tmpPot = SidePot();
        tmpPot.bet = maxBet;
        tmpPot.pot = totalPot;
        sidepots.push_back(tmpPot);
    }

    bool found = false;
    for (std::list<SidePot>::iterator it = sidepots.begin(); it != sidepots.end(); ++it)
        if (it->bet == maxBet)
            found = true;
    if (!found)
    {
        tmpPot = SidePot();
        tmpPot.bet = maxBet;
        tmpPot.pot = totalPot;
        sidepots.push_back(tmpPot);
    }

    sidepots.sort([](SidePot a, SidePot b){ return a.bet < b.bet; });

    uint32 tmpTotal;
    uint32 tmpPrev;
    for (std::list<SidePot>::iterator it = sidepots.begin(); it != sidepots.end(); ++it)
    {
        if (it == sidepots.begin())
            tmpTotal = it->pot;
        else
        {
            tmpPrev = it->pot;
            it->pot -= tmpTotal;
            tmpTotal = tmpPrev;
        }
    }

    if (GetPlayingPlayers() == 1)
    {
        for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
        {
            if (it->second && it->second->GetPlayer() && it->second->IsDealt())
                winners.push_back(it->first);
        }

        for (std::list<SidePot>::iterator it = sidepots.begin(); it != sidepots.end(); ++it)
        {
            uint32 winnerCount = 0;
            for (std::list<uint32>::iterator itw = winners.begin(); itw != winners.end(); ++itw)
                if (table[*itw]->GetBet() >= it->bet)
                    winnerCount++;

            if (winnerCount > 0)
            {
                pot = it->pot / winnerCount;
                for (std::list<uint32>::iterator itw = winners.begin(); itw != winners.end(); ++itw)
                {
                    if (table[*itw]->GetBet() >= it->bet)
                    {
                        table[*itw]->SetChips(table[*itw]->GetChips() + pot);
                        table[*itw]->SetDealt(false);
                    }
                }
            }
            else
            {
                winnerCount = 0;
                for (PokerTable::iterator itt = table.begin(); itt != table.end(); ++itt)
                    if (itt->second && itt->second->GetPlayer() && itt->second->GetBet() >= it->bet)
                        winnerCount++;
                for (PokerTable::iterator itt = table.begin(); itt != table.end(); ++itt)
                {
                    pot = it->pot / winnerCount;
                    if (itt->second && itt->second->GetPlayer() && itt->second->GetBet() >= it->bet)
                    {
                        itt->second->SetChips(itt->second->GetChips() + pot);
                        itt->second->SetDealt(false);
                        // FHS_BroadCastToTable("st_"..j.."_"..Seats[j].chips.."_"..Seats[j].bet.."_"..Seats[j].status.."_0.5")
                        BroadcastToTablePlayerStatus(itt->first, "Returned");
                        // FHS_ShowCard(j,pot.." returned")
                        ShowCards(itt->first, "Returned");
                    }
                }
            }
        }
        BroadcastToTablePlayerStatus(winners.front(), "Winner!");
        BroadcastToTableWins(winners.front());
    }
    else
    {
        PokerHandRank best = PokerHandRank();
        best.hand = POKER_HAND_HIGH_CARD;
        for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
        {
            if (it->second && it->second->GetPlayer() && it->second->IsDealt())
            {
                it->second->SetHandRank(FindHandForPlayer(it->first));
                if (best.cards.empty() || sPokerHandMgr->HandRankCompare(it->second->GetHandRank(), best) > 0)
                    best = it->second->GetHandRank();
            }
        }
        for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
            if (it->second && it->second->GetPlayer() && it->second->IsDealt())
                if (sPokerHandMgr->HandRankCompare(it->second->GetHandRank(), best) == 0)
                    winners.push_back(it->first);
        std::ostringstream text;
        text << POKER_PREFIX << "showdown_0_";
        if (winners.size() > 0)
        {
            if (winners.size() == 1)
				text << table[winners.front()]->GetPlayer()->GetName() << " ha ganado (" << sPokerHandMgr->GetHandRankDescription(table[winners.front()]->GetHandRank()) << ").";
			else
				text << " Dividir (" << sPokerHandMgr->GetHandRankDescription(table[winners.front()]->GetHandRank()) << "). ";

            for (std::list<SidePot>::iterator it = sidepots.begin(); it != sidepots.end(); ++it)
            {
                uint32 winnerCount = 0;
                for (std::list<uint32>::iterator itw = winners.begin(); itw != winners.end(); ++itw)
                    if (table[*itw]->GetBet() >= it->bet)
                        winnerCount++;

                if (winnerCount > 0)
                {
                    pot = it->pot / winnerCount;
                    for (std::list<uint32>::iterator itw = winners.begin(); itw != winners.end(); ++itw)
                    {
                        if (table[*itw]->GetBet() >= it->bet)
                        {
                            table[*itw]->SetChips(table[*itw]->GetChips() + pot);
                            table[*itw]->SetDealt(false);
                            // TODO: FHS_BroadCastToTable("st_"..Winners[j].."_"..ThisSeat.chips.."_"..ThisSeat.bet.."_"..ThisSeat.status.."_1")
							BroadcastToTablePlayerStatus(*itw, "Winner!");
                            // TODO: FHS_ShowCard(Winners[j],"Winner!")
                            ShowCards(*itw, "Winner!");
                        }
                    }
                }
                else
                {
                    winnerCount = 0;
                    for (PokerTable::iterator itt = table.begin(); itt != table.end(); ++itt)
                        if (itt->second && itt->second->GetPlayer() && itt->second->GetBet() >= it->bet)
                            winnerCount++;
                    for (PokerTable::iterator itt = table.begin(); itt != table.end(); ++itt)
                    {
                        pot = it->pot / winnerCount;
                        if (itt->second && itt->second->GetPlayer() && itt->second->GetBet() >= it->bet)
                        {
                            itt->second->SetChips(itt->second->GetChips() + pot);
                            itt->second->SetDealt(false);
                            // FHS_BroadCastToTable("st_"..j.."_"..Seats[j].chips.."_"..Seats[j].bet.."_"..Seats[j].status.."_0.5")
                            BroadcastToTablePlayerStatus(itt->first, "Returned");
                            // FHS_ShowCard(j,pot.." returned")
                            ShowCards(itt->first, "Returned");
                        }
                    }
                }
            }
        }
        else
        {
            text << "No hubo ganadores.";
            for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
                if (it->second && it->second->GetPlayer() && it->second->GetBet() > 0)
                {
                    it->second->SetChips(it->second->GetChips() + it->second->GetBet());
                    it->second->SetDealt(false);
                    // TODO: FHS_BroadCastToTable("st_"..j.."_"..Seats[j].chips.."_"..Seats[j].bet.."_"..Seats[j].status.."_1")
                    BroadcastToTablePlayerStatus(it->first, "Returned");
                }
        }
        BroadcastToTable(text.str());
        for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
            if (it->second && it->second->GetPlayer() && it->second->IsDealt())
                ShowCards(it->first, "Showdown");
    }
}

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
    nextRoundCountdown = 0;
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
    nextRoundCountdown = 0;
}

JoinResult PokerMgr::PlayerJoin(Player *player, uint32 gold)
{
    if (!player)
        return POKER_JOIN_ERROR_NO_PLAYER;
    if (GetSeat(player) > 0)
        return POKER_JOIN_SEATED;
    if (player->GetMoney() < gold * GOLD)
        return POKER_JOIN_ERROR_NO_ENOUGH_MONEY;
    if (gold < POKER_MIN_GOLD || gold > POKER_MAX_GOLD)
        return POKER_JOIN_ERROR_MONEY_OUT_OF_RANGE;
    if (POKER_MAX_GOLD_TABLE - GetTotalMoney() < gold)
        return POKER_JOIN_ERROR_MONEY_TABLE_FULL;
    uint32 seat = GetSeatAvailable();
    if (seat == 0)
        return POKER_JOIN_ERROR_NO_SEATS;
    table[seat] = new PokerPlayer(player);
    player->SetMoney(player->GetMoney() - gold * GOLD);
    table[seat]->SetMoney(gold);
    return POKER_JOIN_OK;
}

void PokerMgr::PlayerLeave(Player *player, bool logout)
{
    uint32 seat = sPokerMgr->GetSeat(player);
    if (seat > 0)
    {
        if (table[seat]->GetMoney() > 0)
        {
            if (!logout && player->GetMoney() < POKER_MAX_GOLD_REWARD * GOLD)
            {
                uint32 allowedMoney = POKER_MAX_GOLD_REWARD - player->GetMoney() / GOLD;
                if (table[seat]->GetMoney() <= allowedMoney)
                {
                    player->SetMoney(player->GetMoney() + table[seat]->GetMoney() * GOLD);
                    table[seat]->SetMoney(0);
                }
                else
                {
                    player->SetMoney(POKER_MAX_GOLD_REWARD * GOLD);
                    table[seat]->SetMoney(table[seat]->GetMoney() - allowedMoney);
                }
            }
            if (table[seat]->GetMoney() > 0)
            {
                CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
                MailSender sender(MAIL_NORMAL, player->GetGUID().GetCounter(), MAIL_STATIONERY_GM);
                MailReceiver receiver(player, player->GetGUID().GetCounter());
                std::string subject = "WoW Poker Lerduzz";
                std::ostringstream body;
                body << (logout ? "Te has desconectado durante una partida" : "Has abandonado la mesa");
                body << " de poker.\n\nEn este correo te enviamos el dinero que no se te pudo entregar directamente.";
                while (table[seat]->GetMoney() > 0)
                {
                    MailDraft draft(subject, body.str().c_str());
                    if (table[seat]->GetMoney() > POKER_MAX_GOLD_REWARD)
                    {
                        draft.AddMoney(POKER_MAX_GOLD_REWARD * GOLD);
                        table[seat]->SetMoney(table[seat]->GetMoney() - POKER_MAX_GOLD_REWARD);
                    }
                    else
                    {
                        draft.AddMoney(table[seat]->GetMoney() * GOLD);
                        table[seat]->SetMoney(0);
                    }
                    draft.SendMailTo(trans, receiver, sender);
                }
                CharacterDatabase.CommitTransaction(trans);
            }
        }
        SendMessageToTable("q", "", logout ? seat : 0, seat);
        if (table[seat]->GetBet() > 0 && status > POKER_STATUS_INACTIVE)
        {
            SidePot tmpPot;
            if (sidepots.size() == 0)
            {
                tmpPot = SidePot();
                tmpPot.bet = 0;
                tmpPot.pot = 0;
                sidepots.push_back(tmpPot);
            }
            bool found = false;
            for (std::list<SidePot>::iterator it = sidepots.begin(); it != sidepots.end(); ++it)
                if (it->bet == 0)
                    found = true;
            if (!found)
            {
                tmpPot = SidePot();
                tmpPot.bet = 0;
                tmpPot.pot = 0;
                sidepots.push_back(tmpPot);
            }
            for (std::list<SidePot>::iterator it = sidepots.begin(); it != sidepots.end(); ++it)
                if (it->bet == 0)
                {
                    it->pot += table[seat]->GetBet();
                    break;
                }
        }
        table.erase(seat);
        if (turn == seat)
            GoNextPlayerTurn();
    }
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

void PokerMgr::SendMessageToTable(std::string msgStart, std::string msgEnd, uint32 exclude, uint32 seat, bool sendHand, float alpha)
{
    for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
    {
        if (it->second && it->second->GetPlayer() && (exclude == 0 || exclude != it->first))
        {
            std::ostringstream resp;
            resp << POKER_PREFIX << msgStart;
            if (seat > 0) resp << "_" << GetFakeSeat(it->first, seat);
            resp << msgEnd;
            if (sendHand) resp << "_" << sPokerHandMgr->GetHandRankDescription(FindHandForPlayer(seat > 0 ? seat : it->first));
            if (alpha > 0) resp << "_" << alpha;
            it->second->GetPlayer()->Whisper(resp.str(), LANG_ADDON, it->second->GetPlayer());
        }
    }
}

void PokerMgr::InformPlayerJoined(uint32 seat, JoinResult jR)
{
    for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
    {
        if (it->second && it->second->GetPlayer())
        {
            std::ostringstream resp;
            resp << POKER_PREFIX << "s_" << GetFakeSeat(seat, it->first) << "_" << it->second->GetPlayer()->GetName() << "_";
            resp << it->second->GetMoney() << "_" << it->second->GetBet() << "_" << (it->second->GetPlayer()->GetFaction() == 1 ? "A" : "H");
            table[seat]->GetPlayer()->Whisper(resp.str(), LANG_ADDON, table[seat]->GetPlayer());
        }
    }
    if (jR == POKER_JOIN_OK)
    {
        std::ostringstream resp;
        resp << "_" << table[seat]->GetPlayer()->GetName() << "_" << table[seat]->GetMoney();
        resp << "_" << table[seat]->GetBet() << "_" << (table[seat]->GetPlayer()->GetFaction() == 1 ? "A" : "H");
        SendMessageToTable("s", resp.str(), seat, seat);
    }
    if (status > POKER_STATUS_INACTIVE && status < POKER_STATUS_SHOW)
    {
        std::ostringstream msg01;
        msg01 << POKER_PREFIX << "b_" << button;
        table[seat]->GetPlayer()->Whisper(msg01.str(), LANG_ADDON, table[seat]->GetPlayer());
        std::ostringstream msg02;
        msg02 << POKER_PREFIX << "betsize_" << POKER_BET_SIZE;
        table[seat]->GetPlayer()->Whisper(msg02.str(), LANG_ADDON, table[seat]->GetPlayer());
        std::ostringstream msg03;
        msg03 << POKER_PREFIX << "round0_" << round;
        table[seat]->GetPlayer()->Whisper(msg03.str(), LANG_ADDON, table[seat]->GetPlayer());
        for (uint32 i = 1; i <= 9; i++)
        {
            uint32 j = i + button + 1;
            while (j > 9) j -= 9;
            if (table.find(j) != table.end())
            {
                if (table[j]->IsDealt())
                {
                    if (j != seat)
                    {
                        std::ostringstream msg04;
                        msg04 << POKER_PREFIX << "deal_" << GetFakeSeat(seat, j);
                        table[seat]->GetPlayer()->Whisper(msg04.str(), LANG_ADDON, table[seat]->GetPlayer());
                    }
                    else
                    {
                        std::ostringstream msg05;
                        msg05 << POKER_PREFIX << "hole_" << table[seat]->GetHole1() << "_" << table[seat]->GetHole2() << "_" << sPokerHandMgr->GetHandRankDescription(FindHandForPlayer(seat));
                        table[seat]->GetPlayer()->Whisper(msg05.str(), LANG_ADDON, table[seat]->GetPlayer());
                    }
                    std::ostringstream msg06;
                    msg06 << POKER_PREFIX << "st_" << GetFakeSeat(seat, j) << "_" << table[j]->GetMoney() << "_" << table[j]->GetBet() << "_Playing_1";
                    table[seat]->GetPlayer()->Whisper(msg06.str(), LANG_ADDON, table[seat]->GetPlayer());
                }
                else
                {
                    std::ostringstream msg07;
                    msg07 << POKER_PREFIX << "st_" << GetFakeSeat(seat, j) << "_" << table[j]->GetMoney() << "_" << table[j]->GetBet() << "_Default_0.5";
                    table[seat]->GetPlayer()->Whisper(msg07.str(), LANG_ADDON, table[seat]->GetPlayer());
                }
            }
        }
        std::ostringstream msg08;
        msg08 << POKER_PREFIX << "flop0";
        table[seat]->GetPlayer()->Whisper(msg08.str(), LANG_ADDON, table[seat]->GetPlayer());
        if (status > POKER_STATUS_PRE_FLOP)
        {
            std::ostringstream msg09;
            msg09 << POKER_PREFIX << "flop1_" << flop[0] << "_" << flop[1] << "_" << flop[2];        
            table[seat]->GetPlayer()->Whisper(msg09.str(), LANG_ADDON, table[seat]->GetPlayer());
            if (status > POKER_STATUS_FLOP)
            {
                std::ostringstream msg10;
                msg10 << POKER_PREFIX << "turn_" << flop[3];        
                table[seat]->GetPlayer()->Whisper(msg10.str(), LANG_ADDON, table[seat]->GetPlayer());
                if (status > POKER_STATUS_TURN)
                {
                    std::ostringstream msg11;
                    msg11 << POKER_PREFIX << "river_" << flop[4];        
                    table[seat]->GetPlayer()->Whisper(msg11.str(), LANG_ADDON, table[seat]->GetPlayer());
                }
            }
        }
        std::ostringstream msg12;
        msg12 << POKER_PREFIX << "go_" << GetFakeSeat(seat, turn) << "_" << HighestBet();
        table[seat]->GetPlayer()->Whisper(msg12.str(), LANG_ADDON, table[seat]->GetPlayer());
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
    if (table[seat]->GetMoney() <= size)
    {
        size = table[seat]->GetMoney();
        status = "All In";
    }

    table[seat]->SetMoney(table[seat]->GetMoney() - size);
    table[seat]->SetBet(table[seat]->GetBet() + size);

    std::ostringstream respSt;
    respSt << "_" << table[seat]->GetMoney() << "_" << table[seat]->GetBet() << "_" << status;
    SendMessageToTable("st", respSt.str(), 0, seat, false, 1.0f);

    if (table[seat]->GetMoney() == 0)
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
        if (delta > table[seat]->GetMoney())
        {
            delta = table[seat]->GetMoney();

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
            if (delta >= table[seat]->GetMoney())
            {
                delta = table[seat]->GetMoney();
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
        std::ostringstream respSt;
        respSt << "_" << pp->GetMoney() << "_" << pp->GetBet() << "_Folded";
        SendMessageToTable("st", respSt.str(), 0, seat, false, 0.5f);
        pp->SetDealt(false);
        pp->SetForcedBet(false);

        if (turn == seat)
            GoNextPlayerTurn();
        else
            if (GetPlayingPlayers() == 1)
                GoNextPlayerTurn();
    }
}

void PokerMgr::ShowCards(uint32 seat)
{
    if (status != POKER_STATUS_SHOW)
        return;
    if (table.find(seat) == table.end())
        return;
	if (table[seat]->GetHole1() == 0 || table[seat]->GetHole2() == 0) 
        return;
    std::ostringstream resp;
    resp << "show_" << table[seat]->GetHole1() << "_" << table[seat]->GetHole2();
    SendMessageToTable(resp.str(), "", seat, seat, true);
}

void PokerMgr::OnWorldUpdate(uint32 diff)
{
    if (delay >= diff)
    {
        delay -= diff;
        return;
    }
    if (status == POKER_STATUS_SHOW && nextRoundCountdown == 0)
        nextRoundCountdown = 10;
    if (nextRoundCountdown > 0)
    {
        nextRoundCountdown--;
        if (nextRoundCountdown == 0)
        {
            status = POKER_STATUS_INACTIVE;
            SendMessageToTable("round0_0");
        }
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
        if (table.find(i) == table.end())
            seats[index++] = i;
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(seats.begin(), seats.end(), std::default_random_engine(seed));
    for (uint32 i = 0; i < 9; i++)
        if (seats[i] != 0)
            return seats[i];
    return 0;
}

uint32 PokerMgr::GetFakeSeat(uint32 me, uint32 other)
{
    int32 delta = 5 - (int32)me;
    int32 fakeSeat = (int32)other + delta;
    while (fakeSeat > (int32)POKER_MAX_SEATS) fakeSeat -= (int32)POKER_MAX_SEATS;
    while (fakeSeat < 1) fakeSeat += (int32)POKER_MAX_SEATS;
    return (uint32) fakeSeat;
}

uint32 PokerMgr::WhosButtonAfter(uint32 start)
{
    for (uint32 i = 1; i <= 9; i++)
    {
        uint32 j = i + start;
        if (j > 9) j -= 9;
        if (j > 9) j -= 9;
        if (table.find(j) != table.end())
            if (table[j]->GetMoney() > 0)
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
            if (table[j]->GetMoney() > 0 && table[j]->IsDealt())
                if (table[j]->GetBet() < maxBet || table[j]->IsForcedBet())
                    return j;
    }
    return 0;
}

void PokerMgr::CleanTable()
{
    for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
    {
        if (it->second && it->second->GetPlayer())
        {
            it->second->SetBet(0);
            it->second->SetDealt(false);
            it->second->SetForcedBet(false);
            it->second->SetHole1(0);
            it->second->SetHole2(0);
        }
    }
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
    uint32 smallBlind = POKER_BET_SIZE / 2;
    uint32 bigBlind = POKER_BET_SIZE;

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
        if (it->second && it->second->GetPlayer() && it->second->GetMoney() > 0)
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
    if (bet == 0)
    {
        for (std::list<SidePot>::iterator it = sidepots.begin(); it != sidepots.end(); ++it)
            if (it->bet == bet)
            {
                total = it->pot;
                break;
            }
        return total;
    }
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

uint32 PokerMgr::GetTotalMoney()
{
    uint64 total = 0;
    for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
        if (it->second && it->second->GetPlayer())
            total += it->second->GetMoney() + it->second->GetBet();
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
    std::ostringstream hB;
    hB << "_" << HighestBet();
    SendMessageToTable("go", hB.str(), 0, turn);
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

void PokerMgr::DealHoleCards()
{
    button = WhosButtonAfter(button);
    SendMessageToTable("b", "", 0, button);

    deck.clear();
    std::array<uint32, 52> deck_arr;
    for (uint32 i = 1; i <= 52; i++)
        deck_arr[i - 1] = i;
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(deck_arr.begin(), deck_arr.end(), std::default_random_engine(seed));
    for (uint32 i = 1; i <= 52; i++)
        deck.push_back(deck_arr[i - 1]);

    std::ostringstream msg1;
    msg1 << "betsize_" << POKER_BET_SIZE;
    SendMessageToTable(msg1.str());

    sidepots.clear();

    round++;
    std::ostringstream msg2;
    msg2 << "round0_" << round;
    SendMessageToTable(msg2.str());

    for (uint32 i = 1; i <= 9; i++)
    {
        uint32 j = i + button + 1;
        while (j > 9) j -= 9;
        if (table.find(j) != table.end())
        {
            table[j]->SetBet(0);
            if (table[j]->GetMoney() > 0)
            {
                table[j]->SetHole1(deck.front());
                deck.pop_front();
                table[j]->SetHole2(deck.front());
                deck.pop_front();

                table[j]->SetDealt(true);

                std::ostringstream msg3;
                msg3 << POKER_PREFIX << "hole_" << table[j]->GetHole1() << "_" << table[j]->GetHole2() << "_" << sPokerHandMgr->GetHandRankDescription(FindHandForPlayer(j));
                table[j]->GetPlayer()->Whisper(msg3.str(), LANG_ADDON, table[j]->GetPlayer());
                SendMessageToTable("deal", "", j, j);
            }
            else
            {
                table[j]->SetDealt(false);
                table[j]->SetHole1(0);
                table[j]->SetHole2(0);
            }
        }
    }
    SendMessageToTable("flop0");

    SetupBets();
    PostBlinds();

    SendMessageToTable("hand", "", 0, 0, true);
}

void PokerMgr::ShowFlopCards()
{
    for (uint32 i = 0; i < 3; i++)
    {
        flop[i] = deck.front();
        deck.pop_front();
    }
    std::ostringstream msg;
    msg << "flop1_" << flop[0] << "_" << flop[1] << "_" << flop[2];        
    SendMessageToTable(msg.str());

    SetupBets();
    turn = button;
    GoNextPlayerTurn();

    SendMessageToTable("hand", "", 0, 0, true);
}

void PokerMgr::DealTurn()
{
    flop[3] = deck.front();
    deck.pop_front();

    std::ostringstream msg;
    msg << "turn_" << flop[3];        
    SendMessageToTable(msg.str());

    SetupBets();
    turn = button;
    GoNextPlayerTurn();

    SendMessageToTable("hand", "", 0, 0, true);
}

void PokerMgr::DealRiver()
{
    flop[4] = deck.front();
    deck.pop_front();

    std::ostringstream msg;
    msg << "river_" << flop[4];        
    SendMessageToTable(msg.str());

    SetupBets();
    turn = button;
    GoNextPlayerTurn();

    SendMessageToTable("hand", "", 0, 0, true);
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
    for (std::list<SidePot>::iterator it = sidepots.begin(); it != sidepots.end(); ++it)
        it->pot = GetSidePot(it->bet);
    sidepots.sort([](SidePot a, SidePot b){ return a.bet < b.bet; });
    uint32 tmpTotal;
    uint32 tmpPrev;
    for (std::list<SidePot>::iterator it = sidepots.begin(); it != sidepots.end(); ++it)
    {
        if (it == sidepots.begin())
        {
            if (it->bet == 0) it++;
            if (it == sidepots.end()) break;
            tmpTotal = it->pot;
        }
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
            if (it->second && it->second->GetPlayer() && it->second->IsDealt())
                winners.push_back(it->first);
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
                        table[*itw]->SetMoney(table[*itw]->GetMoney() + pot);
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
                if (winnerCount > 0)
                    for (PokerTable::iterator itt = table.begin(); itt != table.end(); ++itt)
                    {
                        pot = it->pot / winnerCount;
                        if (itt->second && itt->second->GetPlayer() && itt->second->GetBet() >= it->bet)
                        {
                            itt->second->SetMoney(itt->second->GetMoney() + pot);
                            itt->second->SetDealt(false);
                            std::ostringstream respSt;
                            respSt << "_" << itt->second->GetMoney() << "_" << itt->second->GetBet() << "_Returned";
                            SendMessageToTable("st", respSt.str(), 0, itt->first, false, 0.5f);
                            ShowCards(itt->first);
                        }
                    }
            }
        }
        std::ostringstream respSt;
        respSt << "_" << table[winners.front()]->GetMoney() << "_" << table[winners.front()]->GetBet() << "_Winner!";
        SendMessageToTable("st", respSt.str(), 0, winners.front(), false, 1.0f);
        SendMessageToTable("showdown", "_wins", 0, winners.front());
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
        text << "showdown_0_";
        if (winners.size() > 0)
        {
            if (winners.size() == 1)
				text << "wins";
			else
				text << "split";

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
                            table[*itw]->SetMoney(table[*itw]->GetMoney() + pot);
                            table[*itw]->SetDealt(false);
                            std::ostringstream respSt;
                            respSt << "_" << table[*itw]->GetMoney() << "_" << table[*itw]->GetBet() << "_Winner!";
                            SendMessageToTable("st", respSt.str(), 0, *itw, false, 1.0f);
                            ShowCards(*itw);
                        }
                    }
                }
                else
                {
                    winnerCount = 0;
                    for (PokerTable::iterator itt = table.begin(); itt != table.end(); ++itt)
                        if (itt->second && itt->second->GetPlayer() && itt->second->GetBet() >= it->bet)
                            winnerCount++;
                    if (winnerCount > 0)
                        for (PokerTable::iterator itt = table.begin(); itt != table.end(); ++itt)
                        {
                            pot = it->pot / winnerCount;
                            if (itt->second && itt->second->GetPlayer() && itt->second->GetBet() >= it->bet)
                            {
                                itt->second->SetMoney(itt->second->GetMoney() + pot);
                                itt->second->SetDealt(false);
                                std::ostringstream respSt;
                                respSt << "_" << itt->second->GetMoney() << "_" << itt->second->GetBet() << "_Returned";
                                SendMessageToTable("st", respSt.str(), 0, itt->first, false, 0.5f);
                                ShowCards(itt->first);
                            }
                        }
                }
            }
        }
        else
        {
            text << "No winners";
            for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
                if (it->second && it->second->GetPlayer() && it->second->GetBet() > 0)
                {
                    it->second->SetMoney(it->second->GetMoney() + it->second->GetBet());
                    it->second->SetDealt(false);
                    std::ostringstream respSt;
                    respSt << "_" << it->second->GetMoney() << "_" << it->second->GetBet() << "_Returned";
                    SendMessageToTable("st", respSt.str(), 0, it->first, false, 0.5f);
                }
        }
        SendMessageToTable(text.str());
        for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
            if (it->second && it->second->GetPlayer() && it->second->IsDealt())
                ShowCards(it->first);
    }
    CleanTable();

    SendMessageToTable("hand", "", 0, 0, true);
}

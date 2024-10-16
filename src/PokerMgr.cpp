#include "PokerMgr.h"

PokerMgr::PokerMgr()
{
    table.clear();
    status = POKER_STATUS_INACTIVE;
    deck.clear();
    round = 0;
    button = 1;
    turn = 0;
}

PokerMgr::~PokerMgr()
{
    table.clear();
    status = POKER_STATUS_INACTIVE;
    deck.clear();
    round = 0;
    button = 1;
    turn = 0;
}

bool PokerMgr::PlayerJoin(Player *player, uint32 gold)
{
    if (!player)
        return false;
    if (gold < POKER_MIN_GOLD || gold > POKER_MAX_GOLD)
        return false;
    if (GetSeat(player) > 0)
        return false;
    // TODO: Comprobar que el jugador tenga suficiente oro.
    uint32 seat = GetSeatAvailable();
    if (seat == 0)
        return false;
    table[seat] = new PokerPlayer(player);
    table[seat]->SetChips(gold);
    // TODO: Restar el oro asignado al jugador.
    return true;
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
            resp << it->second->GetChips() << "_" << it->second->GetBet();
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
            resp << table[seat]->GetChips() << "_" << table[seat]->GetBet();
            it->second->GetPlayer()->Whisper(resp.str(), LANG_ADDON, it->second->GetPlayer());
        }
    }
    
    // TODO: DEBUG!
    if (table.size() >= 2)
        sPokerMgr->NextLevel();
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
    }
}

void PokerMgr::PlayerBet(uint32 seat, uint32 size, std::string status)
{
    if (table[seat]->GetChips() < size)
    {
        size = table[seat]->GetChips();
        status = "All In";
    }

    table[seat]->SetChips(table[seat]->GetChips() - size);
    table[seat]->SetBet(table[seat]->GetBet() + size);

    BroadcastToTablePlayerStatus(seat, status);

    // TODO: Sidepot ...
    /*
    --Pots
	if (Seats[j].chips==0) then
		--Mark the curent pot as a side pot.
		found=0;
		bets=FHS_SidePot(Seats[j].bet);
		for r=1,getn(SidePot) do
			if (SidePot[r].bet==Seats[j].bet) then found=1; end;
		end;
		if (found==0) then 
			SidePot[getn(SidePot)+1]={bet=Seats[j].bet,pot=bets}; 
		end;
	end;

	--Check the existing sidepots, if our bet is < a sidepot, that sidepot needs to be rebuilt
	for j=1,getn(SidePot) do
	--if (Seats[j].bet<=SidePot[j].bet) then
			SidePot[j].pot=FHS_SidePot(SidePot[j].bet);
	--	end;
	end;
    */
}

uint32 PokerMgr::GetSeatAvailable()
{
    if (table.size() == POKER_MAX_SEATS)
        return 0;
    uint32 seat = 9;
    while (seat > 0)
    {
        if (table.find(seat) == table.end())
            break;
        seat--;
    }    
    return seat;
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

void PokerMgr::PostBlinds()
{
    uint32 pc = GetPlayingPlayers();
    uint32 smallBlind = 10;
    uint32 bigBlind = 20;

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
    }

    uint32 maxBet = HighestBet();
    BroadcastToTablePlayerTurn(turn, maxBet);

    // TODO: Init playtime limit.
    // PlayerTurnEndTime = GetTime() + AFKTimeLimit;
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
    msg1 << POKER_PREFIX << "betsize_" << 20;        
    BroadcastToTable(msg1.str());

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
            if (table[j]->GetChips() > 0 && table[j]->IsIn())
            {
                table[j]->SetHole1(deck.front());
                deck.pop_front();
                table[j]->SetHole2(deck.front());
                deck.pop_front();

                table[j]->SetDealt(true);
                table[j]->SetBet(0);
                table[j]->SetForcedBet(true);

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

    PostBlinds();
}

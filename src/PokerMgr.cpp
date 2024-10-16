#include "PokerMgr.h"

PokerMgr::PokerMgr()
{
    table.clear();
    status = POKER_STATUS_INACTIVE;
    deck.clear();
    round = 0;
    button = 1;
}

PokerMgr::~PokerMgr()
{
    table.clear();
    status = POKER_STATUS_INACTIVE;
    deck.clear();
    round = 0;
    button = 1;
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
    if (status == POKER_STATUS_PRE_FLOP)
    {
        button = WhosButtonAfter(button);
        // FHS_BroadCastToTable("b_"..TheButton,-1)
        BroadcastToTableButton();

        deck.clear();
        std::array<uint32, 52> deck_arr;
        for (uint32 i = 1; i <= 52; i++)
        {
            // deck.push_back(i);
            deck_arr[i - 1] = i;
        }
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(deck_arr.begin(), deck_arr.end(), std::default_random_engine(seed));
        for (uint32 i = 1; i <= 52; i++)
        {
            deck.push_back(deck_arr[i - 1]);
        }

        // FHS_Set_CurrentBlind(FHS_IncrementBlind(Blinds))
        // FHS_BroadCastToTable("betsize_"..Blinds)
        std::ostringstream msg1;
        msg1 << POKER_PREFIX << "betsize_" << 20;        
        BroadcastToTable(msg1.str());

        // RoundCount=RoundCount+1
        round++;
	    // FHS_BroadCastToTable("round0_"..RoundCount,-1)
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

                    // FHS_SendMessage("hole_"..Seats[j].hole1 .."_"..Seats[j].hole2,Seats[j].name)
                    std::ostringstream msg3;
                    msg3 << POKER_PREFIX << "hole_" << table[j]->GetHole1() << "_" << table[j]->GetHole2();
                    table[j]->GetPlayer()->Whisper(msg3.str(), LANG_ADDON, table[j]->GetPlayer());

				    // FHS_BroadCastToTable("deal_"..j,j)
                    BroadcastToTableDeal(j);
                }
            }
        }

        // TODO: Repartir las 5 cartas de la mesa.
        // DealerFlop={}
	    // for i= 1,5 do
	    // 	DealerFlop[i]=Shuffle[DealerCard]
	    // 	DealerCard=DealerCard+1
	    // end

        // FHS_BroadCastToTable("flop0",-1)
        std::ostringstream msg4;
        msg4 << POKER_PREFIX << "flop0";
        BroadcastToTable(msg4.str());


        // TODO: Marcar jugadores activos para que tengan turno y colocar ciegas.
        // FHS_SetupBets():: Aparentemente innecesario ya que se efectua al repartirle las cartas.
	    // FHS_PostBlinds()

        // TODO: Separar esto en una funcion independiente: function FHS_DealHoleCards().
    }
}

void PokerMgr::PlayerBet(uint32 seat, uint32 size, std::string status)
{
    // TODO: no implementado todavia.
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
        {
            if (table[j]->GetChips() > 0)
            {
                return j;
            }
        }
    }
    return start;
}

void PokerMgr::PostBlinds()
{
    size_t pc = table.size();
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
        next = button;

        PlayerBet(button, smallBlind, "Blinds");
		next = j;
    }
    else if (pc > 2)
    {

    }
}

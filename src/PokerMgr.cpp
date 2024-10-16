#include "PokerMgr.h"

PokerMgr::PokerMgr()
{
    table.clear();
    status = POKER_STATUS_INACTIVE;
}

PokerMgr::~PokerMgr()
{
    table.clear();
    status = POKER_STATUS_INACTIVE;
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
    for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
    {
        if (it->second && it->second->GetPlayer())
        {
            std::ostringstream resp;
            resp << POKER_PREFIX << "s_" << it->first << "_" << it->second->GetPlayer()->GetName() << "_";
            resp << it->second->GetChips() << "_" << it->second->GetBet();
            player->Whisper(resp.str(), LANG_ADDON, player);
        }
    }
}

void PokerMgr::BroadcastToTable(uint32 seat)
{
    for (PokerTable::iterator it = table.begin(); it != table.end(); ++it)
    {
        if (it->second && it->second->GetPlayer() && it->first != seat)
        {
            std::ostringstream resp;
            resp << POKER_PREFIX << "s_" << seat << "_" << table[seat]->GetPlayer()->GetName() << "_";
            resp << table[seat]->GetChips() << "_" << table[seat]->GetBet();
            it->second->GetPlayer()->Whisper(resp.str(), LANG_ADDON, it->second->GetPlayer());
        }
    }
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

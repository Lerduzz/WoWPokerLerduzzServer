#include "PokerMgr.h"

PokerMgr::PokerMgr()
{
    mesa.clear();
    estado = POKER_INACTIVE;
}

PokerMgr::~PokerMgr()
{
    mesa.clear();
    estado = POKER_INACTIVE;
}

bool PokerMgr::JugadorEntrando(Player *player, uint32 gold)
{
    if (!player)
        return false;
    if (gold < POKER_ORO_MINIMO || gold > POKER_ORO_MAXIMO)
        return false;
    if (ObtenerAsiento(player) > 0)
        return false;
    uint32 asiento = AsientoDisponible();
    if (asiento == 0)
        return false;
    mesa[asiento] = player;
    return true;
}

uint32 PokerMgr::ObtenerAsiento(Player *player)
{
    for (PokerMesa::iterator it = mesa.begin(); it != mesa.end(); ++it)
    {
        if (it->second && it->second == player)
            return it->first;
    }
    return 0;
}

uint32 PokerMgr::AsientoDisponible()
{
    uint32 asiento = 9;
    while (asiento > 0)
    {
        if (mesa.find(asiento) == mesa.end())
            break;
        asiento--;
    }    
    return asiento;
}

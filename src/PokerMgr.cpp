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

void PokerMgr::InformarJugador(Player *player)
{
    for (PokerMesa::iterator it = mesa.begin(); it != mesa.end(); ++it)
    {
        if (it->second)
        {
            std::ostringstream resp;
            resp << POKER_PREFIX << "s_" << it->first << "_" << it->second->GetName() << "_";
            resp << 800 /*CHIPS*/ << "_" << 200 /*BET*/;
            player->Whisper(resp.str(), LANG_ADDON, player);
        }
    }
}

void PokerMgr::InformarMesa(uint32 seat)
{
    for (PokerMesa::iterator it = mesa.begin(); it != mesa.end(); ++it)
    {
        if (it->second && it->first != seat)
        {
            std::ostringstream resp;
            resp << POKER_PREFIX << "s_" << seat << "_" << mesa[seat]->GetName() << "_";
            resp << 800 /*CHIPS*/ << "_" << 200 /*BET*/;
            it->second->Whisper(resp.str(), LANG_ADDON, it->second);
        }
    }
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

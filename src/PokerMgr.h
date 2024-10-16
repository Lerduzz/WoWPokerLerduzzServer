#ifndef SC_POKER_MGR_H
#define SC_POKER_MGR_H

#include "PokerPlayer.h"

typedef std::map<uint32, PokerPlayer *> PokerTable;

enum PokerStatus
{
    POKER_STATUS_INACTIVE = 0,
    POKER_STATUS_FLOP,
    POKER_STATUS_TURN,
    POKER_STATUS_RIVER,
    POKER_STATUS_SHOW
};

const int POKER_MIN_GOLD = 500;
const int POKER_MAX_GOLD = 5000;
const int POKER_MAX_SEATS = 9;
const std::string POKER_PREFIX = "PokerLerduzz\tFHS_v8.1.0_";

class PokerMgr
{
public:
    PokerMgr();
    ~PokerMgr();

    static PokerMgr *instance()
    {
        static PokerMgr *instance = new PokerMgr();
        return instance;
    }

    /**
     * Gestiona la entrada de un jugador a la mesa.
     * 
     * @return Verdadero si se pudo unir o Falso de lo contrario.
     */
    bool PlayerJoin(Player *player, uint32 gold);

    /**
     * Determina el asiento que le corresponde en la mesa a un jugador determinado.
     * 
     * @return El numero del asiento o 0 si el jugador no se encuentra en la mesa.
     */
    uint32 GetSeat(Player *player);

    /**
     * Informa al jugador sobre el estado de todos los jugadores de la mesa.
     */
    void InformPlayerJoined(Player *player);

    /**
     * Informa a la mesa sobre la llegada de un nuevo jugador.
     */
    void BroadcastToTable(uint32 seat);

private:
    /**
     * Obtiene un asiento disponible en la mesa.
     * 
     * @return El numero del asiento o 0 si la mesa se encuentra llena.
     */
    uint32 GetSeatAvailable();

    PokerTable table;
    PokerStatus status;

};

#define sPokerMgr PokerMgr::instance()

#endif

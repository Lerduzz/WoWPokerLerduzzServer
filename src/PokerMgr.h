#ifndef SC_POKER_MGR_H
#define SC_POKER_MGR_H

#include "Player.h"

typedef std::map<uint32, Player *> PokerMesa;

enum PokerEstado
{
    POKER_INACTIVE = 0,
    POKER_FLOP,
    POKER_TURN,
    POKER_RIVER,
    POKER_SHOW
};

const int POKER_ORO_MINIMO = 500;
const int POKER_ORO_MAXIMO = 5000;
const int POKER_CANTIDAD_ASIENTOS = 9;
const std::string POKER_PREFIX = "PokerLerduzz\tFHS_v8.1.0_";

class PokerMgr
{
    PokerMgr();
    ~PokerMgr();

public:
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
    bool JugadorEntrando(Player *player, uint32 gold);

    /**
     * Determina el asiento que le corresponde en la mesa a un jugador determinado.
     * 
     * @return El numero del asiento o 0 si el jugador no se encuentra en la mesa.
     */
    uint32 ObtenerAsiento(Player *player);

    /**
     * Informa al jugador sobre el estado de todos los jugadores de la mesa.
     */
    void InformarJugador(Player *player);

    /**
     * Informa a la mesa sobre la llegada de un nuevo jugador.
     */
    void InformarMesa(uint32 seat);

private:
    /**
     * Obtiene un asiento disponible en la mesa.
     * 
     * @return El numero del asiento o 0 si la mesa se encuentra llena.
     */
    uint32 AsientoDisponible();

    PokerMesa mesa;
    PokerEstado estado;

};

#define sPokerMgr PokerMgr::instance()

#endif

#ifndef SC_POKER_MGR_H
#define SC_POKER_MGR_H

#include "PokerPlayer.h"
#include <random>       // std::default_random_engine

typedef std::map<uint32, PokerPlayer *> PokerTable;

enum PokerStatus
{
    POKER_STATUS_INACTIVE = 0,
    POKER_STATUS_PRE_FLOP,
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
     * Obtiene la informacion de una silla de la mesa.
     *
     * @return El numero del asiento o 0 si el jugador no se encuentra en la mesa.
     */
    PokerPlayer *GetSeatInfo(uint32 seat);

    /**
     * Informa al jugador sobre el estado de todos los jugadores de la mesa.
     */
    void InformPlayerJoined(Player *player);

    /**
     * Envia mensaje a todos los jugadores.
     */
    void BroadcastToTable(std::string msg);

    /**
     * Informa a la mesa sobre la llegada de un nuevo jugador.
     */
    void BroadcastToTableJoined(uint32 seat);

    /**
     * Informa a la mesa sobre carta repartida.
     */
    void BroadcastToTableDeal(uint32 seat);

    /**
     * Informa a la mesa sobre el jugador a quien le toca.
     */
    void BroadcastToTablePlayerTurn(uint32 seat, uint32 maxBet);

    /**
     * Informa a la mesa sobre el estado del jugador que ha apostado.
     */
    void BroadcastToTablePlayerStatus(uint32 seat, std::string status);

    /**
     * Informa a la mesa sobre quien es el del boton.
     */
    void BroadcastToTableButton();

    /**
     * Logica principal del juego.
     */
    void NextLevel();

    /**
     * Coloca la apuesta de un jugador.
     */
    void PlayerBet(uint32 seat, uint32 size, std::string status);

    /**
     * Manejar la accion del jugador en turno.
     */
    void PlayerAction(uint32 seat, uint32 delta);

private:
    /**
     * Obtiene un asiento disponible en la mesa.
     *
     * @return El numero del asiento o 0 si la mesa se encuentra llena.
     */
    uint32 GetSeatAvailable();

    /**
     * Determina que jugador debe tener el boton despues.
     *
     * @return Numero del asiento del jugador.
     */
    uint32 WhosButtonAfter(uint32 start);

    /**
     * Determina que jugador debe apostar despues.
     *
     * @return Numero del asiento del jugador.
     */
    uint32 WhosBetAfter(uint32 start);

    /**
     * Marca para que todos los jugadores tengan turno.
     */
    void SetupBets();

    /**
     * Coloca las ciegas.
     */
    void PostBlinds();

    /**
     * Obtener jugadores activos de la ronda.
     *
     * @return Cantidad de jugadores sentados y con cartas.
     */
    uint32 GetPlayingPlayers();

    /**
     * Determina la mayor apuesta de los jugadores de la mesa.
     */
    uint32 HighestBet();

    /**
     * Avanza el turno de apostar al siguiente jugador.
     */
    void GoNextPlayerTurn();

    /**
     * Nivel 1: POKER_STATUS_PRE_FLOP.
     */
    void DealHoleCards();

    /**
     * Nivel 2: POKER_STATUS_FLOP.
     */
    void ShowFlopCards();

    /**
     * Nivel 3: POKER_STATUS_TURN.
     */
    void DealTurn();

    /**
     * Nivel 4: POKER_STATUS_RIVER.
     */
    void DealRiver();

    /**
     * Nivel 5: POKER_STATUS_SHOW.
     */
    void ShowDown();

    PokerTable table;
    PokerStatus status;

    std::list<uint32> deck;
    std::array<uint32, 5> flop;

    uint32 round;
    uint32 button;

    uint32 turn;

};

#define sPokerMgr PokerMgr::instance()

#endif

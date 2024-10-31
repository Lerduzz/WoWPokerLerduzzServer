#ifndef SC_POKER_MGR_H
#define SC_POKER_MGR_H

#include "PokerPlayer.h"
#include "Config.h"
#include <random>

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

enum JoinResult
{
    POKER_JOIN_OK = 0,
    POKER_JOIN_ERROR_NO_PLAYER,
    POKER_JOIN_ERROR_MONEY_OUT_OF_RANGE,
    POKER_JOIN_ERROR_MONEY_TABLE_FULL,
    POKER_JOIN_ERROR_NO_ENOUGH_MONEY,
    POKER_JOIN_SEATED,
    POKER_JOIN_ERROR_NO_SEATS
};

struct SidePot
{
    uint32 bet;
    uint32 pot;
};

const uint32 POKER_MAX_SEATS = 9;
const std::string POKER_PREFIX = "WoWPokerLerduzz\tWPL_v1.0.0_";

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
     * @return Resultado del intento de union.
     */
    JoinResult PlayerJoin(Player *player, uint32 gold);

    /**
     * Gestiona la salida de un jugador de la mesa.
     */
    void PlayerLeave(Player *player, bool logout = false);

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
     * Funcion principal para mantener las mesas actualizadas.
     */
    void SendMessageToTable(std::string msgStart, std::string msgEnd = "", uint32 exclude = 0, uint32 seat = 0, bool sendHand = false, float alpha = 0);

    /**
     * Maneja la llegada de un nuevo jugador a la mesa.
     */
    void InformPlayerJoined(uint32 seat, JoinResult jR);

    /**
     * Logica principal del juego.
     */
    void NextLevel();

    /**
     * Coloca la apuesta de un jugador.
     */
    void PlayerBet(uint32 seat, uint32 size, std::string status);

    /**
     * Manejar la apuesta del jugador.
     */
    void PlayerAction(uint32 seat, uint32 delta);

    /**
     * Manejar la retirada del jugador.
     */
    void FoldPlayer(uint32 seat);

    /**
     * Hace que un jugador muestre sus cartas.
     */
    void ShowCards(uint32 seat);

    /**
     * Gestionar actualizacion del mundo.
     */
    void OnWorldUpdate(uint32 diff);

    /**
     * Verifica el oro pendiente para entregarlo por correo.
     */
    void SendPendingMoney(Player *player);

    /**
     * Carga la configuracion del poker.
     */
    void LoadConfig(bool reload);

    /**
     * Obtener configuracion de oro minimo.
     */
    uint32 GetConfMinGold();

    /**
     * Obtener configuracion de oro maximo.
     */
    uint32 GetConfMaxGold();

private:
    /**
     * Obtiene un asiento disponible en la mesa.
     *
     * @return El numero del asiento o 0 si la mesa se encuentra llena.
     */
    uint32 GetSeatAvailable();

    /**
     * Obtiene el asiento relativo del jugador en la mesa.
     *
     * @return El numero del asiento de 'other' en la mesa de 'me'.
     */
    uint32 GetFakeSeat(uint32 me, uint32 other);

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
     * Limpia todas las apuestas, no devuelve el oro ni anuncia a los clientes.
     */
    void CleanTable();

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
     * Obtener jugadores activos de la mesa.
     *
     * @return Cantidad de jugadores sentados.
     */
    uint32 GetPlayablePlayers();

    /**
     * Determina la mayor apuesta de los jugadores de la mesa.
     *
     * @return Mayor apuesta.
     */
    uint32 HighestBet();

    /**
     * Calcula el sidepot total de una apuesta.
     *
     * @return Sidepot total.
     */
    uint32 GetSidePot(uint32 bet);

    /**
     * Calcula el bote total.
     *
     * @return Bote total.
     */
    uint32 GetTotalPot();

    /**
     * Calcula el dinero total.
     *
     * @return Dinero total en la mesa.
     */
    uint32 GetTotalMoney();

    /**
     * Avanza el turno de apostar al siguiente jugador.
     */
    void GoNextPlayerTurn();

    /**
     * Determinar la mano del jugador.
     *
     * @return Rango y cartas de la mano.
     */
    PokerHandRank FindHandForPlayer(uint32 seat);

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
    std::list<SidePot> sidepots;

    std::list<uint32> deck;
    std::array<uint32, 5> flop;

    uint32 round;
    uint32 button;

    uint32 turn;

    uint32 delay;
    uint32 nextRoundCountdown;

    uint32 confBetSize;
    uint32 confMinGold;
    uint32 confMaxGoldJoin;
    uint32 confMaxGoldTable;
    uint32 confMaxGoldReward;
    uint32 confCountdownTurn;
    uint32 confCountdownRound;

};

#define sPokerMgr PokerMgr::instance()

#endif

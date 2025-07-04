#ifndef SC_POKER_HAND_MGR_H
#define SC_POKER_HAND_MGR_H

#include "Common.h"
#include "Log.h"
#include <map>

enum PokerSuit
{
    POKER_SUIT_NONE = 0,
    POKER_SUIT_CLUBS,
    POKER_SUIT_DIAMONDS,
    POKER_SUIT_HEARTS,
    POKER_SUIT_SPADES
};

enum PokerRank
{
    POKER_RANK_NONE = 0,
    POKER_RANK_TWO = 2,
    POKER_RANK_THREE,
    POKER_RANK_FOUR,
    POKER_RANK_FIVE,
    POKER_RANK_SIX,
    POKER_RANK_SEVEN,
    POKER_RANK_EIGHT,
    POKER_RANK_NINE,
    POKER_RANK_TEN,
    POKER_RANK_JACK,
    POKER_RANK_QUEEN,
    POKER_RANK_KING,
    POKER_RANK_ACE
};

enum PokerHand
{
    POKER_HAND_HIGH_CARD = 0,
    POKER_HAND_ONE_PAIR,
    POKER_HAND_TWO_PAIR,
    POKER_HAND_THREE_OF_A_KIND,
    POKER_HAND_STRAIGHT,
    POKER_HAND_FLUSH,
    POKER_HAND_FULL_FOUSE,
    POKER_HAND_FOUR_OF_A_KIND,
    POKER_HAND_STRAIGHT_FLUSH,
    POKER_HAND_ROYAL_FLUSH
};

struct PokerCard
{
    PokerSuit suit;
    PokerRank rank;
};

struct PokerHandRank
{
    PokerHand hand;
    std::list<PokerCard> cards;
};

class PokerHandMgr
{
public:
    PokerHandMgr() {};
    ~PokerHandMgr() {};

    static PokerHandMgr *instance()
    {
        static PokerHandMgr *instance = new PokerHandMgr();
        return instance;
    }

    /**
     * Determina la mejor mano posible formada por un conjunto de cartas.
     *
     * @return PokerHandRank().
     */
    PokerHandRank BestRank(std::list<uint32> cards);

    /**
     * Compara dos manos de poker para determinar la mejor.
     *
     * @return 0 Si son iguales.
     * @return 1 Si la mano A es mejor que la mano B.
     * @return -1 Si la mano B es mejor que la mano A.
     */
    int32 HandRankCompare(PokerHandRank handA, PokerHandRank handB);

    /**
     * Obtener una descripcion de la mano.
     */
    std::string GetHandRankDescription(PokerHandRank hand);

private:
    /**
     * Determina el palo de una carta (1 - 52).
     *
     * @return PokerSuit.
     */
    PokerSuit GetCardSuit(uint32 card);

    /**
     * Determina el rango de una carta (1 - 52).
     *
     * @return PokerRank.
     */
    PokerRank GetCardRank(uint32 card);

    /**
     * Determina si un conjunto de cartas forman una escalera real.
     *
     * @param cards Lista de cartas ordenadas por rango descendente.
     * @return POKER_HAND_ROYAL_FLUSH.
     */
    PokerHandRank IsRoyalFlush(std::list<PokerCard> cards);

    /**
     * Determina si un conjunto de cartas forman una escalera de color.
     *
     * @param cards Lista de cartas ordenadas por rango descendente y que ya forman un color.
     * @return POKER_HAND_STRAIGHT_FLUSH.
     */
    PokerHandRank IsStraightFlush(std::list<PokerCard> cardsFlush);

    /**
     * Determina si un conjunto de cartas forman un poker.
     *
     * @param cards Lista de cartas ordenadas por rango descendente.
     * @return POKER_HAND_FOUR_OF_A_KIND.
     */
    PokerHandRank IsFourOfAKind(std::list<PokerCard> cards);

    /**
     * Determina si un conjunto de cartas forman un full.
     *
     * @param cards Lista de cartas ordenadas por rango descendente.
     * @return POKER_HAND_FULL_FOUSE.
     */
    PokerHandRank IsFullHouse(std::list<PokerCard> cards);

    /**
     * Determina si un conjunto de cartas forman un color.
     *
     * @param cards Lista de cartas ordenadas por rango descendente.
     * @return POKER_HAND_FLUSH.
     */
    PokerHandRank IsFlush(std::list<PokerCard> cards);

    /**
     * Determina si un conjunto de cartas forman una escalera.
     *
     * @param cards Lista de cartas ordenadas por rango descendente.
     * @return POKER_HAND_STRAIGHT.
     */
    PokerHandRank IsStraight(std::list<PokerCard> cards);

    /**
     * Determina si un conjunto de cartas forman un trio.
     *
     * @param cards Lista de cartas ordenadas por rango descendente.
     * @return POKER_HAND_THREE_OF_A_KIND.
     */
    PokerHandRank IsThreeOfAKind(std::list<PokerCard> cards);

    /**
     * Determina si un conjunto de cartas forman dos parejas.
     *
     * @param cards Lista de cartas ordenadas por rango descendente.
     * @return POKER_HAND_TWO_PAIR.
     */
    PokerHandRank IsTwoPair(std::list<PokerCard> cards);

    /**
     * Determina si un conjunto de cartas forman una pareja.
     *
     * @param cards Lista de cartas ordenadas por rango descendente.
     * @return POKER_HAND_ONE_PAIR.
     */
    PokerHandRank IsOnePair(std::list<PokerCard> cards);

};

#define sPokerHandMgr PokerHandMgr::instance()

#endif

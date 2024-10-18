#ifndef SC_POKER_HAND_MGR_H
#define SC_POKER_HAND_MGR_H

#include "Common.h"
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <functional>
#include <cmath>

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
    PokerCard best1;
    PokerCard best2;
    PokerCard best3;
    PokerCard best4;
    PokerCard best5;
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

    PokerHandRank BestRank(const std::vector<uint32>& cards);

private:    
    PokerSuit GetCardSuit(uint32 card);
    PokerRank GetCardRank(uint32 card);

};

#define sPokerHandMgr PokerHandMgr::instance()

#endif

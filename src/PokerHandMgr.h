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

class HandIterator {
public:
    HandIterator(const std::vector<uint32>& cards) : cards(cards), listCards(5) {
        for (uint32 i = 0; i < std::min(5, (int)cards.size()); ++i) {
            listCards[i] = i;
        }
    }

    std::pair<std::vector<uint32>, std::vector<uint32>> operator()() {
        if (cards.size() <= 5) return {{}, {}};

        for (uint32 i = 4; i >= 0; --i) {
            if (listCards[i] + 5 - i < cards.size()) {
                ++listCards[i];
                for (uint32 j = i + 1; j < 5; ++j) {
                    listCards[j] = listCards[j-1] + 1;
                }
                std::vector<uint32> hand;
                for (uint32 j : listCards) hand.push_back(cards[j]);
                return { listCards, hand };
            }
        }
        return {{}, {}};
    }

private:
    const std::vector<uint32>& cards;
    std::vector<uint32> listCards;

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

    std::string BestRank(const std::vector<uint32>& cards);
    std::string HandDescription(const std::string& rank);

private:    
    std::string Rank(const std::vector<uint32>& cards);
    PokerSuit GetCardSuit(uint32 card);
    uint32 GetCardRank(uint32 card);

};

#define sPokerHandMgr PokerHandMgr::instance()

#endif

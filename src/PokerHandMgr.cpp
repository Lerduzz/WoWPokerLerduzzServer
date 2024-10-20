#include "PokerHandMgr.h"

PokerHandRank PokerHandMgr::BestRank(std::list<uint32> cards)
{
    std::list<PokerCard> cardList;
    for (uint32 card : cards) {
        PokerCard tmp = PokerCard();
        tmp.suit = GetCardSuit(card);
        tmp.rank = GetCardRank(card);
        cardList.push_back(tmp);
    }
    cardList.sort([](PokerCard a, PokerCard b) { return a.rank > b.rank; });

    PokerHandRank result = IsRoyalFlush(cardList);
    if (result.hand == POKER_HAND_ROYAL_FLUSH)
        return result;

    PokerHandRank flush = IsFlush(cardList);

    result = IsStraightFlush(flush.cards);
    if (result.hand == POKER_HAND_STRAIGHT_FLUSH)
        return result;

    result = IsFourOfAKind(cardList);
    if (result.hand == POKER_HAND_FOUR_OF_A_KIND)
        return result;

    // TODO: POKER_HAND_FULL_FOUSE.

    if (flush.hand == POKER_HAND_FLUSH)
    {
        while (flush.cards.size() > 5)
            flush.cards.pop_back();
        return flush;
    }

    result = IsStraight(cardList);
    if (result.hand == POKER_HAND_STRAIGHT)
        return result;

    // TODO: POKER_HAND_THREE_OF_A_KIND.

    // TODO: POKER_HAND_TWO_PAIR.

    // TODO: POKER_HAND_ONE_PAIR.

    result.hand = POKER_HAND_HIGH_CARD;
    result.cards = cardList;
    return result;
}

PokerSuit PokerHandMgr::GetCardSuit(uint32 card)
{
    if (card >= 1 && card <= 13)
        return POKER_SUIT_CLUBS;
    if (card >= 14 && card <= 26)
        return POKER_SUIT_DIAMONDS;
    if (card >= 27 && card <= 39)
        return POKER_SUIT_HEARTS;
    if (card >= 40 && card <= 52)
        return POKER_SUIT_SPADES;
    return POKER_SUIT_NONE;
}

PokerRank PokerHandMgr::GetCardRank(uint32 card)
{
    std::array<PokerRank, 14> ranks = {
        POKER_RANK_NONE,
        POKER_RANK_ACE,
        POKER_RANK_TWO,
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
        POKER_RANK_KING
    };
    uint32 index = ((card - 1) % 13) + 1;
    return ranks[index];
}

PokerHandRank PokerHandMgr::IsRoyalFlush(std::list<PokerCard> cards)
{
    PokerHandRank result = PokerHandRank();
    result.hand = POKER_HAND_HIGH_CARD;
    if (cards.size() >= 5)
    {
        std::list<PokerCard> resultList;
        for (PokerCard card : cards)
            if (card.rank >= POKER_RANK_TEN)
                resultList.push_back(card);
        if (resultList.size() >= 5)
        {
            std::map<PokerSuit, uint32> suitCount;
            for (PokerCard card : resultList)
                suitCount[card.suit]++;
            for (std::map<PokerSuit, uint32>::iterator it = suitCount.begin(); it != suitCount.end(); ++it)
            {
                if (it->second == 5)
                {
                    if (resultList.size() > it->second)
                        resultList.remove_if([ps = it->first](PokerCard pc) { return pc.suit != ps; });
                    result.hand = POKER_HAND_ROYAL_FLUSH;
                    result.cards = resultList;
                    break;
                }
            }
        }
    }
    return result;
}

PokerHandRank PokerHandMgr::IsStraightFlush(std::list<PokerCard> cardsFlush)
{
    PokerHandRank result = IsStraight(cardsFlush);
    if (result.hand == POKER_HAND_STRAIGHT)
        result.hand = POKER_HAND_STRAIGHT_FLUSH;
    return result;
}

PokerHandRank PokerHandMgr::IsFourOfAKind(std::list<PokerCard> cards)
{
    PokerHandRank result = PokerHandRank();
    result.hand = POKER_HAND_HIGH_CARD;
    if (cards.size() >= 4)
    {
        std::map<PokerRank, uint32> rankCount;
        for (PokerCard card : cards)
            rankCount[card.rank]++;
        for (std::map<PokerRank, uint32>::iterator it = rankCount.begin(); it != rankCount.end(); ++it)
        {
            if (it->second == 4)
            {
                std::list<PokerCard> resultList;
                std::list<PokerCard> kickerList;
                for (PokerCard card : cards)
                    if (card.rank == it->first)
                        resultList.push_back(card);
                    else
                        kickerList.push_back(card);
                for (PokerCard kicker : kickerList)
                    if (resultList.size() < 5)
                        resultList.push_back(kicker);
                    else
                        break;
                result.hand = POKER_HAND_FOUR_OF_A_KIND;
                result.cards = resultList;
                break;
            }
        }
    }
    return result;
}

PokerHandRank PokerHandMgr::IsFlush(std::list<PokerCard> cards)
{
    PokerHandRank result = PokerHandRank();
    result.hand = POKER_HAND_HIGH_CARD;
    if (cards.size() >= 5)
    {
        std::list<PokerCard> resultList;
        for (PokerCard card : cards)
            resultList.push_back(card);
        std::map<PokerSuit, uint32> suitCount;
        for (PokerCard card : resultList)
            suitCount[card.suit]++;
        for (std::map<PokerSuit, uint32>::iterator it = suitCount.begin(); it != suitCount.end(); ++it)
        {
            if (it->second >= 5)
            {
                if (resultList.size() > it->second)
                    resultList.remove_if([ps = it->first](PokerCard pc) { return pc.suit != ps; });
                result.hand = POKER_HAND_FLUSH;
                result.cards = resultList;
                break;
            }
        }
    }
    return result;
}

PokerHandRank PokerHandMgr::IsStraight(std::list<PokerCard> cards)
{
    PokerHandRank result = PokerHandRank();
    result.hand = POKER_HAND_HIGH_CARD;
    if (cards.size() >= 5)
    {
        std::list<PokerCard> resultList;
        for (PokerCard card : cards)
            if (resultList.empty() || card.rank != resultList.back().rank)
                resultList.push_back(card);
        if (resultList.size() >= 5)
        {
            PokerCard start;
            PokerCard prev;
            PokerCard end;
            uint32 count;
            for (PokerCard card : resultList)
            {
                if (card.rank == resultList.front().rank)
                {
                    start = card;
                    count = 1;
                }
                else if ((card.rank == resultList.back().rank))
                {
                    if (card.rank + 1 == prev.rank)
                    {
                        count++;
                        end = card;
                    }
                    else
                    {
                        if (count >= 5)
                        {
                            end = prev;
                            break;
                        }
                        else
                        {
                            count = 1;
                            start = card;
                        }
                    }
                }
                else
                {
                    if (card.rank + 1 == prev.rank)
                        count++;
                    else
                    {
                        if (count >= 5)
                        {
                            end = prev;
                            break;
                        }
                        else
                        {
                            count = 1;
                            start = card;
                        }
                    }
                }
                prev = card;
            }
            if (count >= 5)
            {
                resultList.remove_if([st = start, ed = end](PokerCard pc) { return pc.rank > st.rank || pc.rank < ed.rank; });
                while (resultList.size() > 5)
                    resultList.pop_back();
                result.hand = POKER_HAND_STRAIGHT;
                result.cards = resultList;
            }
            else if (count == 4 && resultList.front().rank == POKER_RANK_ACE && resultList.back().rank == POKER_RANK_TWO)
            {
                PokerCard ace = resultList.front();
                resultList.pop_front();
                resultList.push_back(ace);
                resultList.remove_if([](PokerCard pc) { return pc.rank > POKER_RANK_FIVE && pc.rank != POKER_RANK_ACE; });
                result.hand = POKER_HAND_STRAIGHT;
                result.cards = resultList;
            }
        }
    }
    return result;
}

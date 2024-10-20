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

    PokerHandRank result = IsRoyalFlush(cardList);
    if (result.hand == POKER_HAND_ROYAL_FLUSH)
        return result;    

    result.hand = POKER_HAND_HIGH_CARD;
    result.cards = cardList;
    return result;
}

/*
std::string PokerHandMgr::BestRank(const std::vector<uint32> &cards)
{
    std::string rank = "000000";
    std::string newRank;
    HandIterator it(cards);

    while (true) {
        auto [_, hand] = it();
        if (hand.empty()) break;
        newRank = Rank(hand);
        if (newRank > rank) rank = newRank;
    }

    return rank;
}

std::string PokerHandMgr::HandDescription(const std::string &rank)
{
    std::array<std::string, 14> POKER_CARD_RANK = {
        "--",
        "Two",
        "Three",
        "Four",
        "Five",
        "Six",
        "Seven",
        "Eight",
        "Nine",
        "Ten",
        "Jack",
        "Queen",
        "King",
        "Ace"
    };
    std::array<std::string, 14> POKER_CARD_RANK_PLURAL = {
        "--",
        "Twos",
        "Threes",
        "Fours",
        "Fives",
        "Sixes",
        "Sevens",
        "Eights",
        "Nines",
        "Tens",
        "Jacks",
        "Queens",
        "Kings",
        "Aces"
    };

    if (rank.empty()) return "";

    int handType = rank[0] - '0';
    int card1 = std::stoi(rank.substr(1, 2));
    int card2 = std::stoi(rank.substr(3, 2));

    std::vector<std::string> descriptions = {
        "High Card",      // L["High Card"] + ": " + POKER_CARD_RANK[card1],
        "1 Pair",         // L["1 Pair"] + ": " + POKER_CARD_RANK_PLURAL[card1],
        "2 Pair",         // L["2 Pair: %s, %s"],
        "3 of a Kind",    // L["3 of a Kind"] + ": " + POKER_CARD_RANK_PLURAL[card1],
        "Straight",       // L["Straight: %s (high)"],
        "Flush",          // L["Flush: %s (high)"],
        "Full House",     // L["Full House: %s, %s"],
        "4 of a Kind",    // L["4 of a Kind"] + ": " + POKER_CARD_RANK_PLURAL[card1],
        "Straight Flush", // L["Straight Flush: %s (high)"],
        "Royal Flush"     // L["Royal Flush"]
    };

    if (handType == 8 && card1 == 14) handType = 9;

    std::string desc = descriptions[handType];
    if (handType == 2) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer), desc.c_str(), POKER_CARD_RANK_PLURAL[card1].c_str(), POKER_CARD_RANK_PLURAL[card2].c_str());
        desc = buffer;
    } else if (handType == 4 || handType == 5 || handType == 8) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer), desc.c_str(), POKER_CARD_RANK[card1].c_str());
        desc = buffer;
    } else if (handType == 6) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer), desc.c_str(), POKER_CARD_RANK_PLURAL[card1].c_str(), POKER_CARD_RANK_PLURAL[card2].c_str());
        desc = buffer;
    }

    return desc;
}

std::string PokerHandMgr::Rank(const std::vector<uint32> &cards)
{
    std::map<int, int> rankCount;
    for (uint32 card : cards) {
        ++rankCount[GetCardRank(card)];
    }

    std::vector<std::string> sortedGroups(5, "");
    for (const auto& [k, v] : rankCount) {
        sortedGroups.push_back(std::to_string(v) + (k < 10 ? "0" : "") + std::to_string(k));
    }
    std::sort(sortedGroups.begin(), sortedGroups.end(), std::greater<>());

    bool flush = cards.size() == 5 &&
                 GetCardSuit(cards[0]) == GetCardSuit(cards[1]) &&
                 GetCardSuit(cards[0]) == GetCardSuit(cards[2]) &&
                 GetCardSuit(cards[0]) == GetCardSuit(cards[3]) &&
                 GetCardSuit(cards[0]) == GetCardSuit(cards[4]);

    std::vector<std::string> ranks(5, "");
    for (int i = 0; i < std::min(5, (int)cards.size()); ++i) {
        ranks[i] = (GetCardRank(cards[i]) < 10 ? "0" : "") + std::to_string(GetCardRank(cards[i]));
    }
    std::sort(ranks.begin(), ranks.end(), std::greater<>());

    bool straight = false;
    if (sortedGroups[0][0] < '2' && cards.size() == 5) {
        if (ranks[0] == "14" && ranks[1] == "05" && ranks[4] == "02") {
            ranks = {"05", "04", "03", "02", "14"};
            straight = true;
        } else {
            straight = std::stoi(ranks[0]) == std::stoi(ranks[4]) + 4;
        }
    }

    // -- now find best hand
    // -- 5 of a kind
    // if (sortedGroups[1]>"500") return "9"..string.sub(sortedGroups[1],2) end
    if (!sortedGroups.empty() && sortedGroups[0] > "500") return "9" + sortedGroups[0].substr(1);

    // -- straight flush
    // if (straight and flush) then return "8"..string.sub(sortedGroups[1],2) end
    if (straight && flush) return "8" + sortedGroups[0].substr(1);

    // -- 4 of a kind
    // if (sortedGroups[1]>"400") then return "7"..string.sub(sortedGroups[1],2)..string.sub(sortedGroups[2],2) end
    if (sortedGroups[0] > "400") return "7" + sortedGroups[0].substr(1, 1) + sortedGroups[1].substr(1, 1);

    // -- full house
    // if (sortedGroups[1]>"300" and sortedGroups[2]>"200") then return "6"..string.sub(sortedGroups[1],2)..string.sub(sortedGroups[2],2) end
    if (sortedGroups[0] > "300" && sortedGroups[1] > "200") return "6" + sortedGroups[0].substr(1, 1) + sortedGroups[1].substr(1, 1);

    // -- flush
    // if (flush) then return "5"..table.concat(ranks) end
    if (flush) {
        std::string result = "5";
        for (const auto& rank : ranks) {
            result += rank;
        }
        return result;
    }

    // -- straight
    // if (straight) then return "4"..ranks[1] end --string.sub(sortedGroups[1],2) end
    if (straight) return "4" + ranks[1];

    // -- 3 of a kind
    // if (sortedGroups[1]>"300") then return "3"..string.sub(sortedGroups[1],2)..string.sub(sortedGroups[2],2)..string.sub(sortedGroups[3],2) end
    if (sortedGroups[0] > "300") return "3" + sortedGroups[0].substr(1, 1) + sortedGroups[1].substr(1, 1) + sortedGroups[2].substr(1, 1);

    // -- two pair
    // if (sortedGroups[1]>"200" and sortedGroups[2]>"200") then return "2"..string.sub(sortedGroups[1],2)..string.sub(sortedGroups[2],2)..string.sub(sortedGroups[3],2) end
    if (sortedGroups[0] > "200" && sortedGroups[1] > "200") return "2" + sortedGroups[0].substr(1, 1) + sortedGroups[1].substr(1, 1) + sortedGroups[2].substr(1, 1);

    // -- one pair
    // if (sortedGroups[1]>"200") then return "1"..string.sub(sortedGroups[1],2)..string.sub(sortedGroups[2],2)..string.sub(sortedGroups[3],2)..string.sub(sortedGroups[4],2) end
    if (sortedGroups[0] > "200") return "1" + sortedGroups[0].substr(1, 1) + sortedGroups[1].substr(1, 1) + sortedGroups[2].substr(1, 1) + sortedGroups[3].substr(1, 1);

    return "0" + ranks[0] + ranks[1] + ranks[2] + ranks[3] + ranks[4];
}*/

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
        cards.sort([](PokerCard a, PokerCard b) { return a.rank > b.rank; });
        std::list<PokerCard> resultList;
        for (PokerCard card : cards)
        {
            if (card.rank < POKER_RANK_TEN)
                continue;
            if (resultList.empty() || card.rank != resultList.back().rank)
                resultList.push_back(card);
        }
        if (resultList.size() == 5)
        {
            PokerSuit st = resultList.front().suit;
            bool same = true;
            for (PokerCard card : resultList)
            {
                if (card.suit != st)
                {
                    same = false;
                    break;
                }
            }
            if (same)
            {
                result.hand = POKER_HAND_ROYAL_FLUSH;
                result.cards = resultList;
            }
        }
    }
    return result;
}

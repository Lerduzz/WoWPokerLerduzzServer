#ifndef SC_POKER_PLAYER_H
#define SC_POKER_PLAYER_H

#include "Player.h"
#include "PokerHandMgr.h"

class PokerPlayer
{
public:
    PokerPlayer(Player *player);
    ~PokerPlayer() {};

    Player *GetPlayer();
    uint32 GetChips();
    uint32 GetBet();

    uint32 GetHole1();
    uint32 GetHole2();

    bool IsForcedBet();
    bool IsDealt();

    PokerHandRank GetHandRank();

    void SetChips(uint32 chips);
    void SetBet(uint32 bet);

    void SetHole1(uint32 hole1);
    void SetHole2(uint32 hole2);

    void SetForcedBet(bool forcedBet);
    void SetDealt(bool dealt);

    void SetHandRank(PokerHandRank handRank);

private:
    Player *_player;
    uint32 _chips;
    uint32 _bet;

    uint32 _hole1;
    uint32 _hole2;

    bool _forcedBet;
    bool _dealt;

    PokerHandRank _handRank;

};

#endif

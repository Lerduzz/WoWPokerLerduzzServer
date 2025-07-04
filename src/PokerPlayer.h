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
    uint32 GetMoney();
    uint32 GetBet();

    uint32 GetHole1();
    uint32 GetHole2();

    bool IsForcedBet();
    bool IsDealt();

    uint32 GetTurnCountdown();

    PokerHandRank GetHandRank();

    void SetMoney(uint32 money);
    void SetBet(uint32 bet);

    void SetHole1(uint32 hole1);
    void SetHole2(uint32 hole2);

    void SetForcedBet(bool forcedBet);
    void SetDealt(bool dealt);

    void SetTurnCountdown(uint32 turnCountdown);

    void SetHandRank(PokerHandRank handRank);

    inline void ResetAFK() { _afkCount = 0; };
    inline void AddAFK() { _afkCount += _afkCount < 3 ? 1 : 0; };
    inline bool IsAFK() { return _afkCount >= 3; };

private:
    Player *_player;
    uint32 _money;
    uint32 _bet;

    uint32 _hole1;
    uint32 _hole2;

    bool _forcedBet;
    bool _dealt;

    uint32 _turnCountdown;

    PokerHandRank _handRank;

    uint32 _afkCount;

};

#endif

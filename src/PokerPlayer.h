#ifndef SC_POKER_PLAYER_H
#define SC_POKER_PLAYER_H

#include "Player.h"

class PokerPlayer
{
public:
    PokerPlayer(Player *player);
    ~PokerPlayer() {};

    Player *GetPlayer();
    uint32 GetChips();
    uint32 GetBet();

    void SetChips(uint32 chips);
    void SetBet(uint32 bet);

private:
    Player *_player;
    uint32 _chips;
    uint32 _bet;

};

#endif

#include "PokerPlayer.h"

PokerPlayer::PokerPlayer(Player *player)
{
    _player = player;
    _chips = 0;
    _bet = 0;
}

Player *PokerPlayer::GetPlayer()
{
    return _player;
}

uint32 PokerPlayer::GetChips()
{
    return _chips;
}

uint32 PokerPlayer::GetBet()
{
    return _bet;
}

void PokerPlayer::SetChips(uint32 chips)
{
    _chips = chips;
}

void PokerPlayer::SetBet(uint32 bet)
{
    _bet = bet;
}

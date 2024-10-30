#include "PokerPlayer.h"

PokerPlayer::PokerPlayer(Player *player)
{
    _player = player;
    _money = 0;
    _bet = 0;

    _hole1 = 0;
    _hole2 = 0;

    _forcedBet = false;
    _dealt = false;

    _afkCount = 0;
    _turnCountdown = 0;
}

Player *PokerPlayer::GetPlayer()
{
    return _player;
}

uint32 PokerPlayer::GetMoney()
{
    return _money;
}

uint32 PokerPlayer::GetBet()
{
    return _bet;
}

uint32 PokerPlayer::GetHole1()
{
    return _hole1;
}

uint32 PokerPlayer::GetHole2()
{
    return _hole2;
}

bool PokerPlayer::IsForcedBet()
{
    return _forcedBet;
}

bool PokerPlayer::IsDealt()
{
    return _dealt;
}

uint32 PokerPlayer::GetTurnCountdown()
{
    return _turnCountdown;
}

PokerHandRank PokerPlayer::GetHandRank()
{
    return _handRank;
}

void PokerPlayer::SetMoney(uint32 money)
{
    _money = money;
}

void PokerPlayer::SetBet(uint32 bet)
{
    _bet = bet;
}

void PokerPlayer::SetHole1(uint32 hole1)
{
    _hole1 = hole1;
}

void PokerPlayer::SetHole2(uint32 hole2)
{
    _hole2 = hole2;
}

void PokerPlayer::SetForcedBet(bool forcedBet)
{
    _forcedBet = forcedBet;
}

void PokerPlayer::SetDealt(bool dealt)
{
    _dealt = dealt;
}

void PokerPlayer::SetTurnCountdown(uint32 turnCountdown)
{
    _turnCountdown = turnCountdown;
}

void PokerPlayer::SetHandRank(PokerHandRank handRank)
{
    _handRank = handRank;
}

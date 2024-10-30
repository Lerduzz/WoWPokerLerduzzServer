#include "Chat.h"
#include "PokerMgr.h"
#include "ScriptMgr.h"

class WPL_Player : public PlayerScript
{
public:
    WPL_Player() : PlayerScript("WPL_Player") { }

    void OnChat(Player* player, uint32 /*type*/, uint32 lang, std::string& msg, Player* receiver) override
    {
        if (!player || !receiver || player != receiver || lang != LANG_ADDON)
            return;
        size_t prefix_length = POKER_PREFIX.length();
        size_t msg_length = msg.length();
        if (msg_length <= prefix_length || msg.substr(0, prefix_length) != POKER_PREFIX)
            return;
        std::string message = msg.substr(prefix_length, msg_length - prefix_length);
        std::ostringstream resp;
        resp << POKER_PREFIX;
        if (strcmp(message.c_str(), "!init") == 0)
        {
            uint32 seat = sPokerMgr->GetSeat(player);
            if (seat > 0)
                resp << "ping!";
            else
            {
                uint32 gold = player->GetMoney() / GOLD;
                if (gold >= 500)
                    resp << "init!_" << (gold <= 200000 ? gold : 200000);
                else
                    resp << "nogold!";
            }
            msg = resp.str();
        }
        else if (strcmp(message.c_str(), "!seat") == 0)
        {
            resp << "ping!";
            msg = resp.str();
        }
        else if (strcmp(message.c_str(), "quit") == 0)
        {
            sPokerMgr->PlayerLeave(player);
            NullMsg(msg);
        }
        else if (strcmp(message.c_str(), "showcards") == 0)
        {
            uint32 seat = sPokerMgr->GetSeat(player);
            if (seat > 0)
                sPokerMgr->ShowCards(seat);
            NullMsg(msg);
        }
        else
        {
            std::string tok = message;
            char *tab;
            tab = strtok(tok.data(), "_");
            if (strcmp(tab, "join") == 0)
            {
                tab = strtok(nullptr, "_");
                uint32 gold = (uint32) atoi(tab);
                JoinResult jR = sPokerMgr->PlayerJoin(player, gold);
                if (jR == POKER_JOIN_OK || jR == POKER_JOIN_SEATED)
                {
                    uint32 seat = sPokerMgr->GetSeat(player);
                    resp << "seat";
                    player->Whisper(resp.str(), LANG_ADDON, player);
                    sPokerMgr->InformPlayerJoined(seat, jR);
                    NullMsg(msg);
                }
                else
                {
                    switch (jR)
                    {
                        case POKER_JOIN_ERROR_NO_PLAYER:
                        {
                            resp << "noplayer!";
                            break;
                        }
                        case POKER_JOIN_ERROR_NO_ENOUGH_MONEY:
                        {
                            resp << "nogold!";
                            break;
                        }
                        case POKER_JOIN_ERROR_MONEY_OUT_OF_RANGE:
                        {
                            resp << "goldrange!";
                            break;
                        }
                        case POKER_JOIN_ERROR_MONEY_TABLE_FULL:
                        {
                            resp << "tablegold!";
                            break;
                        }
                        default:
                        {
                            resp << "noseats!";
                            break;
                        }
                    }
                    msg = resp.str();
                }
            }
            else if (strcmp(tab, "call") == 0)
            {
                uint32 seat = sPokerMgr->GetSeat(player);
                if (seat > 0)
                {
                    PokerPlayer *pp = sPokerMgr->GetSeatInfo(seat);
                    if (pp && pp->IsDealt())
                    {
                        tab = strtok(nullptr, "_"); // TODO: No enviar el numero de asiento ya que siempre llega 5.
                        tab = strtok(nullptr, "_");
                        uint32 delta = (uint32) atoi(tab);
                        sPokerMgr->PlayerAction(seat, delta);
                    }
                }
                NullMsg(msg);
            }
            else if (strcmp(tab, "fold") == 0)
            {
                uint32 seat = sPokerMgr->GetSeat(player);
                if (seat > 0)
                    sPokerMgr->FoldPlayer(seat);
                NullMsg(msg);
            }
        }
    }

    void OnLogout(Player* player) override
    {
        sPokerMgr->PlayerLeave(player, true);
    }

    void NullMsg(std::string& msg)
    {
        std::ostringstream nresp;
        nresp << POKER_PREFIX << "null";
        msg = nresp.str();
    }
};

class WPL_World : public WorldScript
{
public:
    WPL_World() : WorldScript("WPL_World") { }

    void OnUpdate(uint32 diff) override
    {
        sPokerMgr->OnWorldUpdate(diff);
    }
};


void AddSC_WoWPokerLerduzz()
{
    new WPL_Player();
    new WPL_World();
}

#include "Chat.h"
#include "PokerMgr.h"
#include "ScriptMgr.h"

class WPL_Player : public PlayerScript
{
public:
    WPL_Player() : PlayerScript("WPL_Player") { }

    void OnChat(Player* player, uint32 type, uint32 lang, std::string& msg, Player* receiver) override
    {
        if (!player || !receiver || player != receiver || lang != LANG_ADDON)
            return;
        size_t prefix_length = POKER_PREFIX.length();
        size_t msg_length = msg.length();
        if (msg_length <= prefix_length or msg.substr(0, prefix_length) != POKER_PREFIX)
            return;
        std::string message = msg.substr(prefix_length, msg_length - prefix_length);
        std::ostringstream resp;
        resp << POKER_PREFIX;
        if (strcmp(message.c_str(), "!seat") == 0)
        {
            resp << "ping!";
            msg = resp.str();
        }
        else if (strcmp(message.c_str(), "pong!") == 0)
        {
            uint32 seat = sPokerMgr->GetSeat(player);
            if (seat > 0 || sPokerMgr->PlayerJoin(player, 25000) == POKER_JOIN_OK)
            {
                if (seat == 0)
                    seat = sPokerMgr->GetSeat(player);
                resp << "seat_5";
                player->Whisper(resp.str(), LANG_ADDON, player);
                sPokerMgr->InformPlayerJoined(player);
                sPokerMgr->BroadcastToTableJoined(seat);
                NullMsg(msg);
            }
            else
            {
                // TODO: Informar el problema real de no union.
                resp << "noseats!";
                msg = resp.str();
            }
        }
        else if (strcmp(message.c_str(), "quit") == 0)
        {
            sPokerMgr->PlayerLeave(player);
            NullMsg(msg);
        }
        else
        {
            std::string tok = message;
            char *tab;
            tab = strtok(tok.data(), "_");
            if (strcmp(tab, "call") == 0)
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

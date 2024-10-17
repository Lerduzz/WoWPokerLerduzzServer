#include "Chat.h"
#include "Log.h"
#include "PokerMgr.h"
#include "ScriptMgr.h"

class WPL_Player : public PlayerScript
{
public:
    WPL_Player() : PlayerScript("WPL_Player") { }

    void OnLogin(Player* player) override
    {
        ChatHandler(player->GetSession()).SendSysMessage("El casino esta activado.");
    }

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
        if (message == "!seat")
        {
            resp << "ping!";
            msg = resp.str();
        }
        else if (message == "pong!")
        {
            uint32 seat = sPokerMgr->GetSeat(player);
            if (seat > 0 || sPokerMgr->PlayerJoin(player, 1000))
            {
                if (seat == 0)
                    seat = sPokerMgr->GetSeat(player);
                resp << "seat_5";
                player->Whisper(resp.str(), LANG_ADDON, player);
                sPokerMgr->InformPlayerJoined(player);
                sPokerMgr->BroadcastToTableJoined(seat);
                std::ostringstream nresp;
                nresp << POKER_PREFIX << "null";
                msg = nresp.str();
            }
            else
            {
                resp << "NoSeats";
                msg = resp.str();
            }
        }
        else
        {
            char *tab;
            tab = strtok(message.data(), "_");
            if (tab == "call")
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
                std::ostringstream nresp;
                nresp << POKER_PREFIX << "null";
                msg = nresp.str();
            }
            else
            {
                std::ostringstream ann;
                ann << "WoWPokerLerduzz:: [" << receiver->GetName() << "]: " << message;
                LOG_ERROR("poker", ann.str().c_str());
            }
        }
    }
};

void AddSC_WoWPokerLerduzz()
{
    new WPL_Player();
}

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
        std::string prefix = "PokerLerduzz\tFHS_v8.1.0_";
        size_t prefix_length = prefix.length();
        size_t msg_length = msg.length();
        if (msg_length <= prefix_length or msg.substr(0, prefix_length) != prefix)
            return;
        std::string message = msg.substr(prefix_length, msg_length - prefix_length);
        std::ostringstream resp;
        if (message == "!seat")
        {
            resp << prefix << "ping!";
            msg = resp.str();
        }
        else
        {
            std::ostringstream ann;
            ann << "WoWPokerLerduzz:: [" << receiver->GetName() << "]: " << message;
            LOG_ERROR("poker", ann.str().c_str());
        }
    }
};

void AddSC_WoWPokerLerduzz()
{
    new WPL_Player();
}

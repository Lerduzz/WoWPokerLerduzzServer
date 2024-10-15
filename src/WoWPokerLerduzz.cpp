#include "Chat.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "World.h"

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
        if (lang == LANG_ADDON)
        {
            std::ostringstream ann;
            ann << "Desde: " << ChatHandler(player->GetSession()).GetNameLink(player) << "; ";
            ann << "Hasta: " << ChatHandler(receiver->GetSession()).GetNameLink(receiver) << ": " << msg;
            sWorld->SendServerMessage(SERVER_MSG_STRING, ann.str().c_str());
        }
    }
};

void AddSC_WoWPokerLerduzz()
{
    new WPL_Player();
}

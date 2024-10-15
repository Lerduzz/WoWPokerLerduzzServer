#include "Chat.h"
#include "Player.h"
#include "ScriptMgr.h"

class WPL_Player : public PlayerScript
{
public:
    WPL_Player() : PlayerScript("WPL_Player") { }

    void OnLogin(Player* player) override
    {
        ChatHandler(player->GetSession()).SendSysMessage("El casino esta activado.");
    }
};

void AddSC_WoWPokerLerduzz()
{
    new WPL_Player();
}

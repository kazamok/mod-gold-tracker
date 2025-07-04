#include "ScriptMgr.h"
#include "Player.h"
#include "Chat.h"

class mod_gold_tracker : public PlayerScript
{
public:
    mod_gold_tracker() : PlayerScript("mod_gold_tracker") { }

    void OnPlayerMoneyChanged(Player* player, int32 amount) override
    {
        // Log gold changes to the console
        LOG_INFO("module", "Player {} (GUID: {}) money changed by {}. New total: {}",
            player->GetName(), player->GetGUID().GetRawValue(), amount, player->GetMoney());
    }
};

void Addmod_gold_trackerScripts()
{
    new mod_gold_tracker();
}

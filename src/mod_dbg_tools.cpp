/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "Chat.h"
#include "Config.h"
#include "Object.h"
#include "Player.h"
#include "ScriptMgr.h"

using namespace Acore::ChatCommands;

constexpr const char* MODULE_STRING = "dbg-tools";

enum DbgToolsStrings
{
    INFO_MSG = 1,
    ERROR_MSG = 2,
    SUCCESS_MSG = 3,
};

struct DbgToolsModule
{
    bool Enabled;
};

DbgToolsModule DbgTools;

class DbgToolsConfig : public WorldScript
{
public:
    DbgToolsConfig() : WorldScript("DbgToolsConfig") { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        DbgTools.Enabled = sConfigMgr->GetOption<bool>("DbgTools.Enabled", true);
    }
};

class DbgToolsPlayer : public PlayerScript
{
public:
    DbgToolsPlayer() : PlayerScript("PlayerDbgToolsPlayer") { }

    void OnPlayerLogin(Player* player) override
    {
        if (player->GetSession()->GetSecurity() >= SEC_ADMINISTRATOR)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("{} is enabled!", MODULE_STRING);
        }
    }

    void OnPlayerLogout(Player* /*player*/) override
    {
    }
};

class DbgToolsCommand : public CommandScript
{
public:
    DbgToolsCommand() : CommandScript("DbgToolsCommand") {}

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable DbgToolsResetCommandTable =
        {
            { "dailies",  HandleResetDailiesCommand, SEC_ADMINISTRATOR, Console::No },
        };

        static ChatCommandTable DbgToolsSwpCommandTable =
        {
            { "mad",  HandleResetMadrigosaCommand, SEC_ADMINISTRATOR, Console::No },
        };

        static ChatCommandTable DbgToolsCommandTable =
        {
            { "reset",  DbgToolsResetCommandTable },
            { "swp",  DbgToolsSwpCommandTable }
        };

        static ChatCommandTable DbgToolsBaseTable =
        {
            { "dbg",  DbgToolsCommandTable }
        };

        return DbgToolsBaseTable;
    }

    static bool HandleResetDailiesCommand(ChatHandler* handler)
    {
        Player* player = handler->GetSession()->GetPlayer();

        if (!player)
            return false;
        player->ResetDailyQuestStatus();
        handler->PSendSysMessage("[DBG]::Player::ResetDailyQuestStatus()::Dailies have been reset!");
        return true;
    }

    // Action to start Intro Event
    static bool HandleResetMadrigosaCommand(ChatHandler* handler)
    {
        Player* player = handler->GetSession()->GetPlayer();

        if (!player)
            return false;

        if (auto madrigosa = player->FindNearestCreature(24895, 500.0f))
        {
            madrigosa->AI()->DoAction(1); // Start Event
            handler->PSendSysMessage("[DBG]::Player::HandleResetMadrigosaCommand set action done!");
        }
        else
            handler->PSendSysMessage("[DBG]::Player::HandleResetMadrigosaCommand Madrigosa Not found!");
        return true;
    }
};

void AddSC_dbg_tools_script()
{
    new DbgToolsConfig();
    new DbgToolsPlayer();
    new DbgToolsCommand();
}
/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "Chat.h"
#include "Config.h"
#include "Object.h"
#include "Player.h"
#include "ScriptMgr.h"

using namespace Acore::ChatCommands;

constexpr char const* MODULE_STRING = "dbg-tools";

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
            ChatHandler(player->GetSession()).PSendSysMessage("{} is enabled!", MODULE_STRING);
    }

    void OnPlayerLogout(Player* /*player*/) override { }
};

class DbgToolsCommand : public CommandScript
{
public:
    DbgToolsCommand() : CommandScript("DbgToolsCommand") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable DbgToolsResetCommandTable = {
            {"dailies", HandleResetDailiesCommand, SEC_ADMINISTRATOR, Console::No},
        };

        static ChatCommandTable DbgToolsSwpCommandTable = {
            {"mad",     HandleResetMadrigosaCommand, SEC_ADMINISTRATOR, Console::No},
            {"felmyst", HandleSpawnFelmystCommand,   SEC_ADMINISTRATOR, Console::No},
            {"vapor",   HandleCastVaporCommand,      SEC_ADMINISTRATOR, Console::No},
        };

        static ChatCommandTable DbgToolsUldCommandTable = {
            {"yoggbraindmg", HandleStartYoggP3Command, SEC_ADMINISTRATOR, Console::No},
        };

        static ChatCommandTable DbgToolsCommandTable = {
            {"reset", DbgToolsResetCommandTable},
            {"swp",   DbgToolsSwpCommandTable  },
            {"uld",   DbgToolsUldCommandTable  }
        };

        static ChatCommandTable DbgToolsBaseTable = {
            {"dbg", DbgToolsCommandTable}
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

    // Vapor
    static bool HandleCastVaporCommand(ChatHandler* handler)
    {
        Player* player = handler->GetSession()->GetPlayer();

        if (!player)
            return false;

        player->CastCustomSpell(45391, SPELLVALUE_MAX_TARGETS, 1, player, true);

        handler->PSendSysMessage("[DBG]::Player::HandleCastVaporCommand set action done!");
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

    // Action to spawn Felmyst
    static bool HandleSpawnFelmystCommand(ChatHandler* handler)
    {
        Player* player = handler->GetSession()->GetPlayer();

        if (!player)
            return false;

        if (auto madrigosa = player->FindNearestCreature(24895, 500.0f))
        {
            madrigosa->AI()->DoAction(2); // Spawn Felmyst
            handler->PSendSysMessage("[DBG]::Player::HandleSpawnFelmystCommand set action done!");
        }
        else
            handler->PSendSysMessage("[DBG]::Player::HandleSpawnFelmystCommand Madrigosa Not found!");
        return true;
    }

    // Action to start Yogg-Saron P3 (skip entering portal to kill tentacles/brain)
    static bool HandleStartYoggP3Command(ChatHandler* handler)
    {
        Player* player = handler->GetSession()->GetPlayer();

        if (!player)
            return false;

        if (auto sara = player->FindNearestCreature(33134, 500.0f))
        {
            sara->AI()->DoAction(-8); /*ACTION_BRAIN_DAMAGED*/
            handler->PSendSysMessage("[DBG]::Player::HandleStartYoggP3Command brain damaged!");
        }
        else
            handler->PSendSysMessage("[DBG]::Player::HandleStartYoggP3Command Not found!");
        return true;
    }
};

void AddSC_dbg_tools_script()
{
    new DbgToolsConfig();
    new DbgToolsPlayer();
    new DbgToolsCommand();
}
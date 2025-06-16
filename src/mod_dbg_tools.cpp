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

        static ChatCommandTable DbgToolsSiCommandTable = {
            {"despawnnecropolis", HandleStartDespawnNecropolisCommand, SEC_ADMINISTRATOR, Console::No},
        };

        static ChatCommandTable DbgToolsCommandTable = {
            {"reset", DbgToolsResetCommandTable},
            {"swp",   DbgToolsSwpCommandTable  },
            {"uld",   DbgToolsUldCommandTable  },
            {"si",    DbgToolsSiCommandTable   }
        };

        static ChatCommandTable DbgToolsBaseTable = {
            {"dbg", DbgToolsCommandTable}
        };

        return DbgToolsBaseTable;
    }

    // .dbg reset dailies
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
    // .dbg swp vapor
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
    // .dbg swp mad
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
    // .dbg swp felmyst
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
    // .dbg uld yoggbraindmg
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

    enum SINecropolisObjects
    {
        GO_NECROPOLIS_TINY = 181154,   // Necropolis (scale 1.0).
        GO_NECROPOLIS_SMALL = 181373,  // Necropolis (scale 1.5).
        GO_NECROPOLIS_MEDIUM = 181374, // Necropolis (scale 2.0).
        GO_NECROPOLIS_BIG = 181215,    // Necropolis (scale 2.5).
        GO_NECROPOLIS_HUGE = 181223,   // Necropolis (scale 3.5).
    };

    // .dbg si despawnnecropolis <option>
    static bool HandleStartDespawnNecropolisCommand(ChatHandler* handler, uint32 option)
    {
        Player* player = handler->GetSession()->GetPlayer();

        if (!player)
            return false;

        std::list<GameObject*> necropolisList;
        player->GetGameObjectListWithEntryInGrid(necropolisList,
            {GO_NECROPOLIS_TINY, GO_NECROPOLIS_SMALL, GO_NECROPOLIS_MEDIUM, GO_NECROPOLIS_BIG, GO_NECROPOLIS_HUGE},
            30.0f // ATTACK_DISTANCE
        );
        if (necropolisList.empty())
        {
            handler->PSendSysMessage("[DBG]::Player::HandleStartDespawnNecropolisCommand Not found!");
            return false;
        }

        for (GameObject* const& necropolis : necropolisList)
        {
            if (option == 1)
            {
                necropolis->DespawnOrUnsummon(5s);
                handler->PSendSysMessage("[DBG]::Player::HandleStartDespawnNecropolisCommand DespawnOrUnsummon 5s delay");
            }
            else if (option == 2)
            {
                necropolis->UseDoorOrButton();
                handler->PSendSysMessage("[DBG]::Player::HandleStartDespawnNecropolisCommand UseDoorOrButton");
            }
            else if (option == 3)
            {
                necropolis->ResetDoorOrButton();
                handler->PSendSysMessage("[DBG]::Player::HandleStartDespawnNecropolisCommand ResetDoorOrButton");
            }
            else if (option == 4)
            {
                necropolis->DespawnOrUnsummon();
                handler->PSendSysMessage("[DBG]::Player::HandleStartDespawnNecropolisCommand DespawnOrUnsummon no delay");
            }
            else if (option == 5)
            {
                necropolis->DespawnOrUnsummon();
                necropolis->SetRespawnDelay(-1); // Disable respawn.
                handler->PSendSysMessage("[DBG]::Player::HandleStartDespawnNecropolisCommand DespawnOrUnsummon option 5");
            }
            else if (option == 6)
            {
                necropolis->DespawnOrUnsummon(10s);
                necropolis->SetRespawnDelay(-1); // Disable respawn.
                handler->PSendSysMessage("[DBG]::Player::HandleStartDespawnNecropolisCommand DespawnOrUnsummon option 5");
            }
            else
            {
                handler->PSendSysMessage("[DBG]::Player::HandleStartDespawnNecropolisCommand Invalid option!");
                return false;
            }
        }

        return true;
    }
};

void AddSC_dbg_tools_script()
{
    new DbgToolsConfig();
    new DbgToolsPlayer();
    new DbgToolsCommand();
}

void DespawnNecropolis() { }
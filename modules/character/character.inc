#if defined _module_character_included
    #endinput
#endif

#define _module_character_included

#include <PawnPlus>
#include <async-dialogs>
#include <uuid>

#include <config-default-spawn>
#include <config-character-spawn-types>
#include <config-cmd-flags>

#include <character-auth>
#include <inventory>

const MAX_CHARACTER_PERSONAL_VEHICLES = 6;

static enum E_CHARACTER_DATA
{
    bool:char_Spawned,
    char_Spawn_Type,
    
    char_UUID[UUID_LEN],
    char_Account_UUID[UUID_LEN],
    char_Name[MAX_PLAYER_NAME + 1],
    char_Skin,
    char_Cmd_Flags
}

static characterData[MAX_PLAYERS][E_CHARACTER_DATA];

stock bool:Character_IsSpawned(const playerid)
{
    return characterData[playerid][char_Spawned];
}

stock Character_GetUUID(const playerid, uuid[], const size = UUID_LEN)
{
    uuid[0] = EOS;
    strcat(uuid, characterData[playerid][char_UUID], size);
    return 1;
}

stock Character_GetAccountUUID(const playerid, uuid[], const size = UUID_LEN)
{
    uuid[0] = EOS;
    strcat(uuid, characterData[playerid][char_Account_UUID], size);
    return 1;
}

stock Character_GetName(const playerid, name[], size = MAX_PLAYER_NAME + 1)
{
    name[0] = EOS;
    strcat(name, characterData[playerid][char_Name], size);
    return 1;
}

stock Character_GetCmdFlags(const playerid)
{
    return characterData[playerid][char_Cmd_Flags];
}

stock Character_SetAdminLevel(const playerid, const level)
{
    switch(level)
    {
        case 0:
        {
            characterData[playerid][char_Cmd_Flags] = 0;
        }
        case 1:
        {
            characterData[playerid][char_Cmd_Flags] = CMD_FLAG_ADMIN_1;
        }
        case 2:
        {
            characterData[playerid][char_Cmd_Flags] = CMD_FLAG_ADMIN_1 | CMD_FLAG_ADMIN_2;
        }
        case 3:
        {
            characterData[playerid][char_Cmd_Flags] = CMD_FLAG_ADMIN_1 | CMD_FLAG_ADMIN_2 | CMD_FLAG_ADMIN_3;
        }
        case 4:
        {
            characterData[playerid][char_Cmd_Flags] = CMD_FLAG_ADMIN_1 | CMD_FLAG_ADMIN_2 | CMD_FLAG_ADMIN_3 | CMD_FLAG_ADMIN_4;
        }
        case 5:
        {
            characterData[playerid][char_Cmd_Flags] = CMD_FLAG_ADMIN_1 | CMD_FLAG_ADMIN_2 | CMD_FLAG_ADMIN_3 | CMD_FLAG_ADMIN_4 | CMD_FLAG_ADMIN_5;
        }
    }
    return 1;
}

stock Character_Reset(const playerid)
{
    characterData[playerid][char_UUID][0] = EOS;
    characterData[playerid][char_Account_UUID][0] = EOS;
    characterData[playerid][char_Spawned] = false;
    characterData[playerid][char_Skin] = 0;
    characterData[playerid][char_Cmd_Flags] = 0;
    return 1;
}

stock Character_Spawn(const playerid, const uuid[])
{
    task_yield(1);
    await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT * FROM characters WHERE uuid = '%e' LIMIT 1", uuid));

    if(cache_num_rows() > 0)
    {
        new Float:pos[3],
            Float:angle,
            Float:health,
            Float:armour,
            admin_level;
        
        cache_get_value_name(0, "uuid", characterData[playerid][char_UUID], UUID_LEN);
        cache_get_value_name(0, "account_uuid", characterData[playerid][char_Account_UUID], UUID_LEN);
        cache_get_value_name(0, "name", characterData[playerid][char_Name], MAX_PLAYER_NAME + 1);
        
        cache_get_value_name_int(0, "skin", characterData[playerid][char_Skin]);
        cache_get_value_name_int(0, "admin_level", admin_level);
        
        cache_get_value_name_float(0, "pos_x", pos[0]);
        cache_get_value_name_float(0, "pos_y", pos[1]);
        cache_get_value_name_float(0, "pos_z", pos[2]);
        cache_get_value_name_float(0, "angle", angle);
        cache_get_value_name_float(0, "health", health);
        cache_get_value_name_float(0, "armour", armour);

        TogglePlayerSpectating(playerid, false);

        characterData[playerid][char_Spawned] = true;
        characterData[playerid][char_Spawn_Type] = CHARACTER_SPAWN;

        if(pos[0] == 0.0000 && pos[1] == 0.0000 && pos[2] == 0.0000)
        {
            pos[0] = defaultSpawnCoords[0];
            pos[1] = defaultSpawnCoords[1];
            pos[2] = defaultSpawnCoords[2];
            angle = defaultSpawnCoords[3];
        }

        Player_SetName(playerid, characterData[playerid][char_Name]);
        Character_SetAdminLevel(playerid, admin_level);

        SetSpawnInfo(playerid, 1, characterData[playerid][char_Skin], pos[0], pos[1], pos[2], angle, 0, 0, 0, 0, 0, 0);
        SpawnPlayer(playerid);

        SetPlayerHealth(playerid, health);
        SetPlayerArmour(playerid, armour);
    }
    else
    {
        Player_Kick(playerid, -1, MSG_PREFIX_ALT"He thong khong tim thay nhan vat ton tai trong csdl, vui long bao loi hoac ket noi lai.");
    }
    return 1;
}

forward Character_OnLogin(const playerid, const uuid[]);
public Character_OnLogin(const playerid, const uuid[])
{
    Character_Reset(playerid);
    Character_Spawn(playerid, uuid);
    return 1;
}

forward Character_OnLogout(const playerid);
public Character_OnLogout(const playerid)
{
    new Float:pos[3],
        Float:health,
        Float:armour,
        Float:angle;
    
    GetPlayerPos(playerid, pos[0], pos[1], pos[2]);
    GetPlayerFacingAngle(playerid, angle);
    GetPlayerHealth(playerid, health);
    GetPlayerArmour(playerid, armour);

    await mysql_aquery_s(MySQL_GetHandle(), str_format("UPDATE characters SET pos_x = %.4f, pos_y = %.4f, pos_z = %.4f, angle = %.4f, health = %.4f, armour = %.4f WHERE uuid = '%e' LIMIT 1", pos[0], pos[1], pos[2], angle, health, armour, characterData[playerid][char_UUID]), true);

    if(cache_affected_rows() != 0)
    {
        printf("INFO: Character_OnLogout - character's stats saved (%s)", characterData[playerid][char_UUID]);
    }
    else
    {
        printf("INFO: Character_OnLogout - character's stats failed to save (%s)", characterData[playerid][char_UUID]);
    }

    Character_Reset(playerid);
    return 1;
}

forward Character_OnSpawn(const playerid, const type);
public Character_OnSpawn(const playerid, const type)
{
    return 1;
}

#include <YSI_Coding\y_hooks>

hook OnGameModeInit()
{
    mysql_query(MySQL_GetHandle(), "CREATE TABLE IF NOT EXISTS characters (\
        uuid VARCHAR(37) NOT NULL PRIMARY KEY DEFAULT '',\
        name VARCHAR(33) NOT NULL DEFAULT '',\
        password VARCHAR(61) NOT NULL DEFAULT '',\
        gender TINYINT UNSIGNED NOT NULL DEFAULT 0,\
        skin INT NOT NULL DEFAULT 0,\
        birthday VARCHAR(12) NOT NULL DEFAULT '',\
        admin_level TINYINT UNSIGNED NOT NULL DEFAULT 0,\
        health FLOAT(11, 4) NOT NULL DEFAULT 100.0000,\
        armour FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        pos_x FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        pos_y FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        pos_z FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        angle FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        interior INT NOT NULL DEFAULT 0,\
        world INT NOT NULL DEFAULT 0,\
        created_date TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP,\
        account_uuid VARCHAR(37),\
        FOREIGN KEY (account_uuid) REFERENCES accounts(uuid) ON UPDATE CASCADE ON DELETE CASCADE \
    );", false);
    
    for(new i = 0; i != MAX_PLAYERS; i++)
    {
        Character_Reset(i);
    }

    print("INFO: ./modules/character loaded");
    return 1;
}

hook OnPlayerDisconnect(playerid, reason)
{
    if(characterData[playerid][char_Spawned])
    {
        CallLocalFunction("Character_OnLogout", "i", playerid);
    }
    return 1;
}

hook OnPlayerSpawn(playerid)
{
    CallLocalFunction("Character_OnSpawn", "ii", playerid, characterData[playerid][char_Spawn_Type]);
    characterData[playerid][char_Spawn_Type] = CHARACTER_INVALID_SPAWN_TYPE;
    return 1;
}
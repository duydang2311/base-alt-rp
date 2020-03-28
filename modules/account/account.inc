#if defined _module_account_included
    #endinput
#endif

#define _module_account_included

#include <PawnPlus>
#include <uuid>

#include <account-auth>
#include <character>
#include <mysql>

enum E_ACCOUNT_DATA
{
    account_UUID[UUID_LEN],
    account_Name[MAX_PLAYER_NAME + 1]
}

static accountData[MAX_PLAYERS][E_ACCOUNT_DATA];

stock Account_GetUUID(const playerid, uuid[], const size = UUID_LEN)
{
    uuid[0] = EOS;
    strcat(uuid, accountData[playerid][account_UUID], size);
    return 1;
}

stock Task:Account_RetrieveName(const uuid[])
{
    new Task:my_task = task_new();
    task_detach();
    task_yield(my_task);

    await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT name FROM accounts WHERE uuid = '%e' LIMIT 1", uuid));

    new name[MAX_PLAYER_NAME + 1];
    if(cache_num_rows() > 0)
    {
        cache_get_value_name(0, "name", name, sizeof(name));
        task_set_result_str(my_task, name);
    }
    else
    {
        task_set_result_str(my_task, "");
    }
    return Task:0;
}

stock Account_SetUUID(const playerid, const uuid[])
{
    accountData[playerid][account_UUID][0] = EOS;
    strcat(accountData[playerid][account_UUID], uuid, UUID_LEN);
    return 1;
}

stock Account_Reset(const playerid)
{
    accountData[playerid][account_UUID][0] = EOS;
    return 1;
}

forward Account_OnLogin(const playerid, const uuid[]);
public Account_OnLogin(const playerid, const uuid[])
{
    task_yield(1);
    
    Account_SetUUID(playerid, uuid);
    await_arr(accountData[playerid][account_Name]) Account_RetrieveName(uuid);
    
    Character_Show(playerid);
    return 1;
}

#include <YSI_Coding\y_hooks>

hook OnGameModeInit()
{
    mysql_query(MySQL_GetHandle(), "CREATE TABLE IF NOT EXISTS accounts (\
        uuid VARCHAR(37) NOT NULL PRIMARY KEY DEFAULT '',\
        name VARCHAR(33) NOT NULL DEFAULT '',\
        password VARCHAR(61) NOT NULL DEFAULT '',\
        created_date TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP\
    );", false);
    
    for(new i = 0; i != MAX_PLAYERS; i++)
    {
        Account_Reset(i);
    }
    print("INFO: ./modules/account loaded");
    return 1;
}

hook Character_OnLogout(const playerid)
{
    Player_SetName(playerid, accountData[playerid][account_Name]);
    return 1;
}
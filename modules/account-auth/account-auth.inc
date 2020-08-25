#if defined _module_account_auth_included
    #endinput
#endif

#define _module_account_auth_included

#include <PawnPlus>
#include <pp-mysql>
#include <strlib>
#include <bcrypt>
#include <samp-precise-timers>

#include <config-msg-prefixes>
#include <config-dialog-ids>

#include <mysql>
#include <character>

static const BCRYPT_COST = 12;
static const ACCOUNT_REGISTER = 0;
static const ACCOUNT_LOGIN = 1; 

forward bool:Character_IsSpawned(const playerid);

stock Task:Account_IsRegistered(const name[])
{
    new Task:my_task = task_new();
    task_detach();
    task_yield(my_task);
    
    await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT COUNT(*) FROM accounts WHERE name = '%e' LIMIT 1", name));

    new counts = 0;
    cache_get_value_name_int(0, "COUNT(*)", counts);
    task_set_result(my_task, (counts > 0) ? ACCOUNT_LOGIN : ACCOUNT_REGISTER);
    return Task:0;
}

forward OnAccountPasswordHashed(const playerid);
public OnAccountPasswordHashed(const playerid)
{
    new hash[BCRYPT_HASH_LENGTH],
        uuid[UUID_LEN],
        name[MAX_PLAYER_NAME + 1];
    bcrypt_get_hash(hash);
    UUID(uuid, sizeof(uuid));
    GetPlayerName(playerid, name, sizeof(name));

    task_yield(1);
    await mysql_aquery_s(MySQL_GetHandle(), str_format("INSERT INTO accounts (uuid, name, password) VALUES ('%e', '%e', '%e')", uuid, name, hash));

    new auth = await Account_IsRegistered(name);
    if(auth == ACCOUNT_LOGIN)
    {
        if(GetPVarType(playerid, "accountAuth_Timer"))
        {
            DeletePreciseTimer(GetPVarInt(playerid, "accountAuth_Timer"));
        }

        SetPVarInt(playerid, "accountAuth_Timer", SetPreciseTimer("OnAuthenticationExpired", 60000, false, "i", playerid));

        SendClientMessage(playerid, -1, MSG_PREFIX_ALT"Ban co 60 giay de hoan tat qua trinh nhan dien tai khoan va nhan vat.");
        ShowPlayerDialog(playerid, DIALOG_ACCOUNT_LOGIN, DIALOG_STYLE_PASSWORD, "Account | Login", sprintf("{FFFFFF}Tai khoan '%s' da duoc dang ky, nhap mat khau de dang nhap.", name), "Dang nhap", "Thoat");
    }
    else
    {
        if(GetPVarType(playerid, "accountAuth_Timer"))
        {
            DeletePreciseTimer(GetPVarInt(playerid, "accountAuth_Timer"));
        }

        SetPVarInt(playerid, "accountAuth_Timer", SetPreciseTimer("OnAuthenticationExpired", 180000, false, "i", playerid));

        SendClientMessage(playerid, -1, MSG_PREFIX_ALT"Ban co 180 giay de hoan tat qua trinh dang ky tai khoan va nhan vat.");
        ShowPlayerDialog(playerid, DIALOG_ACCOUNT_REGISTER, DIALOG_STYLE_PASSWORD, "Account | Register", sprintf("{FFFFFF}Tai khoan '%s' chua duoc su dung, nhap mat khau de dang ky.", name), "Dang ky", "Thoat");
    }
	return 1;
}

forward OnAccountPasswordChecked(const playerid, const uuid[]);
public OnAccountPasswordChecked(const playerid, const uuid[])
{
    if(bcrypt_is_equal())
    {   
        if(GetPVarType(playerid, "accountAuth_Timer"))
        {
            DeletePreciseTimer(GetPVarInt(playerid, "accountAuth_Timer"));
        }

        CallLocalFunction("Account_OnLogin", "is", playerid, uuid);
    }
    else
    {
        new attempts = GetPVarInt(playerid, "accountAuth_Attempts");
        SetPVarInt(playerid, "accountAuth_Attempts", --attempts);
        if(attempts <= 0)
        {
            Player_Kick(playerid, -1, MSG_PREFIX_ALT"Ban da vuot qua so lan nhap sai mat khau.");
        }
        else
        {
            Player_Msg(playerid, -1, sprintf(MSG_PREFIX_ALT"Ban con lai %d lan nhap sai mat khau.", attempts));
        }

        new name[MAX_PLAYER_NAME+1];
        GetPlayerName(playerid, name, sizeof(name));
        ShowPlayerDialog(playerid, DIALOG_ACCOUNT_LOGIN, DIALOG_STYLE_PASSWORD, "Account | Login", sprintf("{FFFFFF}Tai khoan '%s' da duoc dang ky, nhap mat khau de dang nhap.", name), "Dang nhap", "Thoat");
    }
    return 1;
}

forward OnAuthenticationExpired(const playerid);
public OnAuthenticationExpired(const playerid)
{
    if(!Character_IsSpawned(playerid))
    {
        Player_Kick(playerid, -1, MSG_PREFIX_ALT"Thoi gian dang nhap/dang ky da het.");
    }
    return 1;
}

#include <YSI_Coding\y_hooks>

hook OnGameModeInit()
{
    print("INFO: ./modules/account-auth loaded");
    return 1;
}

hook OnPlayerConnect(playerid)
{
    TogglePlayerSpectating(playerid, true);
    
    new name[MAX_PLAYER_NAME+1];
    GetPlayerName(playerid, name, sizeof(name));

    new auth = await Account_IsRegistered(name);
    SetPVarInt(playerid, "accountAuth_Timestamp", gettime());
    SetPVarInt(playerid, "accountAuth_Attempts", 3);
    if(auth == 1)
    {
        if(GetPVarType(playerid, "accountAuth_Timer"))
        {
            DeletePreciseTimer(GetPVarInt(playerid, "accountAuth_Timer"));
        }

        SetPVarInt(playerid, "accountAuth_Timer", SetPreciseTimer("OnAuthenticationExpired", 60000, false, "i", playerid));
        SendClientMessage(playerid, -1, MSG_PREFIX_ALT"Ban co 60 giay de hoan tat qua trinh nhan dien tai khoan va nhan vat.");
        ShowPlayerDialog(playerid, DIALOG_ACCOUNT_LOGIN, DIALOG_STYLE_PASSWORD, "Account | Login", sprintf("{FFFFFF}Tai khoan '%s' da duoc dang ky, nhap mat khau de dang nhap.", name), "Dang nhap", "Thoat");
    }
    else
    {
        if(GetPVarType(playerid, "accountAuth_Timer"))
        {
            DeletePreciseTimer(GetPVarInt(playerid, "accountAuth_Timer"));
        }

        SetPVarInt(playerid, "accountAuth_Timer", SetPreciseTimer("OnAuthenticationExpired", 180000, false, "i", playerid));
        SendClientMessage(playerid, -1, MSG_PREFIX_ALT"Ban co 180 giay de hoan tat qua trinh dang ky tai khoan va nhan vat.");
        ShowPlayerDialog(playerid, DIALOG_ACCOUNT_REGISTER, DIALOG_STYLE_PASSWORD, "Account | Register", sprintf("{FFFFFF}Tai khoan '%s' chua duoc su dung, nhap mat khau de dang ky.", name), "Dang ky", "Thoat");
    }
    return 1;
}

hook OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    switch(dialogid)
    {
        case DIALOG_ACCOUNT_REGISTER:
        {
            if(response)
            {
                if(isnull(inputtext) || strlen(inputtext) > 127)
                {
                    new name[MAX_PLAYER_NAME+1];
                    GetPlayerName(playerid, name, sizeof(name));
                    ShowPlayerDialog(playerid, DIALOG_ACCOUNT_REGISTER, DIALOG_STYLE_PASSWORD, "Account | Register", sprintf("{FFFFFF}Tai khoan '%s' chua duoc su dung, nhap mat khau de dang ky.", name), "Dang ky", "Thoat");
                    return 1;
                }

                bcrypt_hash(inputtext, BCRYPT_COST, "OnAccountPasswordHashed", "i", playerid);
            }
            else
            {
                Player_Kick(playerid, -1, MSG_PREFIX_ALT"Ban da ngat ket noi voi ALT:Roleplay.");
            }
        }
        case DIALOG_ACCOUNT_LOGIN:
        {
            if(response)
            {
                new name[MAX_PLAYER_NAME+1];
                GetPlayerName(playerid, name, sizeof(name));
                if(isnull(inputtext) || strlen(inputtext) > 127)
                {
                    
                    ShowPlayerDialog(playerid, DIALOG_ACCOUNT_LOGIN, DIALOG_STYLE_PASSWORD, "Account | Login", sprintf("{FFFFFF}Tai khoan '%s' da duoc dang ky, nhap mat khau de dang nhap.", name), "Dang nhap", "Thoat");
                    return 1;
                }
                
                task_yield(1);
                await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT uuid, password FROM accounts WHERE name = '%e' LIMIT 1", name));

                if(cache_num_rows() > 0)
                {
                    new hash[BCRYPT_HASH_LENGTH],
                        uuid[UUID_LEN];
                    
                    cache_get_value_name(0, "uuid", uuid, sizeof(uuid));
                    cache_get_value_name(0, "password", hash, sizeof(hash));

                    bcrypt_check(inputtext, hash, "OnAccountPasswordChecked", "is", playerid, uuid);
                }
                else
                {
                    Player_Kick(playerid, -1, MSG_PREFIX_ALT"He thong khong tim thay ten tai khoan tuong ung voi ten cua ban.");
                }
            }
            else
            {
                Player_Kick(playerid, -1, MSG_PREFIX_ALT"Ban da ngat ket noi voi "EMBED_COLOR_ALT"ALT:Roleplay.");
            }
        }
    }
    return 1;
}
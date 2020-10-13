#if defined _module_character_auth_included
    #endinput
#endif

#define _module_character_auth_included

#include <a_samp>
#include <sscanf2>
#include <PawnPlus>
#include <pp-mysql>
#include <uuid>
#include <bcrypt>

#include <utils>
#include <config-dialog-ids>

#include <mysql>
#include <account>

static enum E_TEMP_CHARACTER_DATA
{
    char_UUID[UUID_LEN],
    char_Name[MAX_PLAYER_NAME + 1]
}

static temp_characterData[MAX_PLAYERS][MAX_CHARACTERS_PER_ACCOUNT][E_TEMP_CHARACTER_DATA];

forward Account_GetUUID(const playerid, uuid[], const size = UUID_LEN);

stock Character_Show(const playerid)
{
    new uuid[UUID_LEN];
    Account_GetUUID(playerid, uuid, sizeof(uuid));
    await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT uuid, name, password FROM characters WHERE account_uuid = '%e'", uuid));

    new rows = cache_num_rows(),
        string[256];

    strcat(string, "Ten nhan vat\tUUID");

    for(new i = 0; i != MAX_CHARACTERS_PER_ACCOUNT; i++)
    {
        if(i < rows)
        {
            cache_get_value_name(i, "uuid", temp_characterData[playerid][i][char_UUID], UUID_LEN);
            cache_get_value_name(i, "name", temp_characterData[playerid][i][char_Name], MAX_PLAYER_NAME + 1);
        }
        else
        {
            strcopy(temp_characterData[playerid][i][char_Name], "Tao nhan vat", MAX_PLAYER_NAME + 1);
        }
    }

    for(new i = 0; i != MAX_CHARACTERS_PER_ACCOUNT; i++)
    {
        format(string, sizeof(string), "%s\n%s\t%s", string, temp_characterData[playerid][i][char_Name], temp_characterData[playerid][i][char_UUID]);
    }

    ShowPlayerDialog(playerid, DIALOG_CHARACTER_LIST, DIALOG_STYLE_TABLIST_HEADERS, "Character | List", string, "Chon", "Thoat");
    return 1;
}

stock bool:Character_IsValidName(const name[], len)
{
    new first_name[MAX_PLAYER_NAME + 1],
        last_name[MAX_PLAYER_NAME + 1];
    if(sscanf(name, "p<_>s[32]s[32]", first_name, last_name))
    {
        return false;
    }

    if(strlen(first_name) == 0 || strlen(last_name) == 0)
    {
        return false;
    }

    while(len--)
    {
        switch(name[len])
        {
            case 'a' .. 'z', 'A' .. 'Z', '_': continue;
            default: return false;
        }
    }
    return true;
}

Character_IsAvailable(const username[]) {
    new 
        count = 0;
    
    await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT COUNT(*) FROM characters WHERE name = '%e' LIMIT 1", username));
    cache_get_value_name_int(0, "COUNT(*)", count);
    return count;    
}

forward OnCharacterPasswordChecked(const playerid, const uuid[]);
public OnCharacterPasswordChecked(const playerid, const uuid[])
{
    if(bcrypt_is_equal())
    {
        CallLocalFunction("Character_OnLogin", "is", playerid, uuid);

        for(new i = 0; i != MAX_CHARACTERS_PER_ACCOUNT; i++)
        {
            temp_characterData[playerid][i][char_UUID][0] = EOS;
            temp_characterData[playerid][i][char_Name][0] = EOS;
        } 
        DeletePVar(playerid, "charAuth_index");
        DeletePVar(playerid, "charAuth_attempts");
    }
    else
    {
        new attempts = GetPVarInt(playerid, "charAuth_attempts");
        if((--attempts) <= 0)
        {
            Player_Kick(playerid, -1, MSG_PREFIX_ALT"Ban da vuot qua so lan nhap sai mat khau.");
        }
        else
        {
            Player_Msg(playerid, -1, sprintf(MSG_PREFIX_ALT"Ban con %d lan nhap sai mat khau.", attempts));
            SetPVarInt(playerid, "charAuth_attempts", attempts);
        }
        ShowPlayerDialog(playerid, DIALOG_CHAR_LOGIN_PASSWORD, DIALOG_STYLE_PASSWORD, "Character | Password", sprintf("{FFFFFF}Nhan vat '%s' co dat mat khau, nhap mat khau de dang nhap.", temp_characterData[playerid][GetPVarInt(playerid, "charAuth_index")][char_Name]), "Dang nhap", "<<");
    }
    return 1;
}

#include <YSI_Coding\y_hooks>

hook OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    switch(dialogid)
    {
        case DIALOG_CHARACTER_LIST:
        {
            if(response)
            {
                if(isnull(temp_characterData[playerid][listitem][char_UUID]))
                {
                    ShowPlayerDialog(playerid, DIALOG_CHAR_SIGNUP_NAME, DIALOG_STYLE_INPUT, "Character | Sign Up | Name", "{FFFFFF}Nhap ho va ten (hoac nhap ca ten dem neu co) cho nhan vat [cu phap: Ho_Ten, do dai: 31 ki tu]:", ">>", "<<");
                }
                else
                {
                    new password[BCRYPT_HASH_LENGTH];
                    await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT password FROM characters WHERE uuid = '%e' LIMIT 1", temp_characterData[playerid][listitem][char_UUID]));

                    if(cache_num_rows() > 0)
                    {
                        cache_get_value_name(0, "password", password, sizeof(password));
                    }
                    else
                    {
                        Player_Kick(playerid, -1, MSG_PREFIX_ALT"Nhan vat khong ton tai trong csdl, vui long bao loi hoac ket noi lai.");
                        return 1;
                    }

                    if(!isnull(password))
                    {
                        SetPVarInt(playerid, "charAuth_index", listitem);
                        SetPVarInt(playerid, "charAuth_attempts", 3);
                        ShowPlayerDialog(playerid, DIALOG_CHAR_LOGIN_PASSWORD, DIALOG_STYLE_PASSWORD, "Character | Password", sprintf("{FFFFFF}Nhan vat '%s' co dat mat khau, nhap mat khau de dang nhap.", temp_characterData[playerid][listitem][char_Name]), "Dang nhap", "<<");
                    }
                    else
                    {
                        CallLocalFunction("Character_OnLogin", "is", playerid, temp_characterData[playerid][listitem][char_UUID]);
                        for(new i = 0; i != MAX_CHARACTERS_PER_ACCOUNT; i++)
                        {
                            temp_characterData[playerid][i][char_UUID][0] = EOS;
                            temp_characterData[playerid][i][char_Name][0] = EOS;
                        }
                    }
                }
            }
            else
            {
                Player_Kick(playerid, -1, MSG_PREFIX_ALT"Ban da ngat ket noi khoi "EMBED_COLOR_ALT"ALT:Roleplay.");
            }
        }
        case DIALOG_CHAR_LOGIN_PASSWORD:
        {
            if(response)
            {
                new char_idx = GetPVarInt(playerid, "charAuth_index");
                if(isnull(inputtext) || strlen(inputtext) > 127)
                {
                    ShowPlayerDialog(playerid, DIALOG_CHAR_LOGIN_PASSWORD, DIALOG_STYLE_PASSWORD, "Character | Password", sprintf("{FFFFFF}Nhan vat '%s' co dat mat khau, nhap mat khau de dang nhap.", temp_characterData[playerid][char_idx][char_Name]), "Dang nhap", "<<");
                    return 1;
                }

                new password[BCRYPT_HASH_LENGTH];
                await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT password FROM characters WHERE uuid = '%e' LIMIT 1", temp_characterData[playerid][char_idx][char_UUID]));

                if(cache_num_rows() > 0)
                {
                    cache_get_value_name(0, "password", password, sizeof(password));
                }
                else
                {
                    Player_Kick(playerid, -1, MSG_PREFIX_ALT"Nhan vat khong ton tai trong csdl, vui long bao loi hoac ket noi lai.");
                    return 1;
                }

                bcrypt_check(inputtext, password, "OnCharacterPasswordChecked", "is", playerid, temp_characterData[playerid][char_idx][char_UUID]);
            }
            else
            {
                Character_Show(playerid);
                DeletePVar(playerid, "charAuth_index");
                DeletePVar(playerid, "charAuth_attempts");
            }
        }
        case DIALOG_CHAR_SIGNUP_NAME:
        {
            if(response)
            {
                if(isnull(inputtext) || strlen(inputtext) >= MAX_PLAYER_NAME)
                {
                    ShowPlayerDialog(playerid, DIALOG_CHAR_SIGNUP_NAME, DIALOG_STYLE_INPUT, "Character | Sign Up | Name", "{FFFFFF}Nhap ho va ten (hoac nhap ca ten dem neu co) cho nhan vat [cu phap: Ho_Ten, do dai: 31 ki tu]:", ">>", "<<");
                    return 1;
                }

                if(Character_IsAvailable(inputtext)) {
                    Player_Msg(playerid, -1, MSG_PREFIX_ALT"Ten nhan vat da co trong he thong.");
                    ShowPlayerDialog(playerid, DIALOG_CHAR_SIGNUP_NAME, DIALOG_STYLE_INPUT, "Character | Sign Up | Name", "{FFFFFF}Nhap ho va ten (hoac nhap ca ten dem neu co) cho nhan vat [cu phap: Ho_Ten, do dai: 31 ki tu]:", ">>", "<<");
                    return 1;                    
                }

                if(!Character_IsValidName(inputtext, strlen(inputtext)))
                {
                    Player_Msg(playerid, -1, MSG_PREFIX_ALT"Ho va ten khong hop le.");
                    ShowPlayerDialog(playerid, DIALOG_CHAR_SIGNUP_NAME, DIALOG_STYLE_INPUT, "Character | Sign Up | Name", "{FFFFFF}Nhap ho va ten (hoac nhap ca ten dem neu co) cho nhan vat [cu phap: Ho_Ten, do dai: 31 ki tu]:", ">>", "<<");
                    return 1;
                }

                SetPVarString(playerid, "charSignUp_name", inputtext);
                ShowPlayerDialog(playerid, DIALOG_CHAR_SIGNUP_GENDER, DIALOG_STYLE_LIST, "Character | Sign Up | Gender", "{FFFFFF}Nam\nNu\n{333333}Chon mot gioi tinh", ">>", "<<");
            }
            else
            {
                Character_Show(playerid);
            }
        }
        case DIALOG_CHAR_SIGNUP_GENDER:
        {
            if(response)
            {
                if(listitem == 2)
                {
                    ShowPlayerDialog(playerid, DIALOG_CHAR_SIGNUP_GENDER, DIALOG_STYLE_LIST, "Character | Sign Up | Gender", "{FFFFFF}Nam\nNu\n{333333}Chon mot gioi tinh", ">>", "<<");
                    return 1;
                }

                SetPVarInt(playerid, "charSignUp_gender", listitem);
                ShowPlayerDialog(playerid, DIALOG_CHAR_SIGNUP_SKIN, DIALOG_STYLE_INPUT, "Character | Sign Up | Skin", "{FFFFFF}Nhap ID skin cho nhan vat:", ">>", "<<");
            }
            else
            {
                DeletePVar(playerid, "charSignUp_name");
                ShowPlayerDialog(playerid, DIALOG_CHAR_SIGNUP_NAME, DIALOG_STYLE_INPUT, "Character | Sign Up | Name", "{FFFFFF}Nhap ho va ten (hoac nhap ca ten dem neu co) cho nhan vat [cu phap: Ho_Ten, do dai: 31 ki tu]:", ">>", "<<");
            }
        }
        case DIALOG_CHAR_SIGNUP_SKIN:
        {
            if(response)
            {
                new skin;
                if(sscanf(inputtext, "i", skin))
                {
                    ShowPlayerDialog(playerid, DIALOG_CHAR_SIGNUP_SKIN, DIALOG_STYLE_INPUT, "Character | Sign Up | Skin", "{FFFFFF}Nhap ID skin cho nhan vat:", ">>", "<<");
                    return 1;
                }

                if(skin < 1 || skin > 299)
                {
                    ShowPlayerDialog(playerid, DIALOG_CHAR_SIGNUP_SKIN, DIALOG_STYLE_INPUT, "Character | Sign Up | Skin", "{FFFFFF}Nhap ID skin cho nhan vat:", ">>", "<<");
                    return 1;
                }

                SetPVarInt(playerid, "charSignUp_skin", skin);
                ShowPlayerDialog(playerid, DIALOG_CHAR_SIGNUP_BIRTHDAY, DIALOG_STYLE_INPUT, "Character | Sign Up | Birthday", "{FFFFFF}Nhap ngay-thang-nam sinh cua nhan vat:", ">>", "<<");
            }
            else
            {
                DeletePVar(playerid, "charSignUp_gender");
                ShowPlayerDialog(playerid, DIALOG_CHAR_SIGNUP_GENDER, DIALOG_STYLE_LIST, "Character | Sign Up | Gender", "{FFFFFF}Nam\nNu\n{333333}Chon mot gioi tinh", ">>", "<<");
            }
        }
        case DIALOG_CHAR_SIGNUP_BIRTHDAY: {
            if(response) {
                new day, month, year, day2, month2, year2;
                getdate(year, month, day);

                if(sscanf(inputtext, "p<->iii", day2, month2, year2)) {
                    Player_Msg(playerid, -1, MSG_PREFIX_ALT"Ngay thang nam sinh duoc nhap theo dinh dang: ngay-thang-nam (vi du: 03-01-2002).");
                    return ShowPlayerDialog(playerid, DIALOG_CHAR_SIGNUP_BIRTHDAY, DIALOG_STYLE_INPUT, "Character | Sign Up | Birthday", "{FFFFFF}Nhap ngay-thang-nam sinh cua nhan vat:", ">>", "<<");
                }

                new check = year - year2;
                if(check > 50 || check < 15 || day2 > Utils_GetMonthDays(year2, month2) || day2 < 1 || month2 > 12 || month2 < 1) {
                    Player_Msg(playerid, -1, MSG_PREFIX_ALT"Thong tin ma ban nhap vao khong hop le.");
                    if(check > 50 || check < 15) Player_Msg(playerid, -1, sprintf(MSG_PREFIX_ALT"Nam sinh gioi han tu %d - %d.", year - 15, year - 50));
                    if(month2 > 12 || month2 < 1) Player_Msg(playerid, -1, MSG_PREFIX_ALT"Thang sinh gioi han tu 1 - 12.");
                    if(day2 > Utils_GetMonthDays(year2, month2) || day2 < 1) Player_Msg(playerid, -1, sprintf(MSG_PREFIX_ALT"Thang %02d co ngay gioi han tu 1 - %d.", Utils_GetMonthDays(year2, month2)));
                    return ShowPlayerDialog(playerid, DIALOG_CHAR_SIGNUP_BIRTHDAY, DIALOG_STYLE_INPUT, "Character | Sign Up | Birthday", "{FFFFFF}Nhap ngay-thang-nam sinh cua nhan vat:", ">>", "<<");
                }

                new name[MAX_PLAYER_NAME + 1],
                    birthday[12],
                    string[180];
                GetPVarString(playerid, "charSignUp_name", name, sizeof(name));
                format(birthday, sizeof(birthday), "%02d-%02d-%d", day2, month2, year2);
                SetPVarString(playerid, "charSignUp_birthday", birthday);

                format(string, sizeof(string), "\n\
                {FFFFFF}Ho va ten: %s\n\
                Gioi tinh: %s\n\
                Trang phuc: %d\n\
                Ngay thang nam sinh: %s\n\n\
                Xin hay xac nhan cac thong tin ban da nhap, sau do an 'Xac nhan' de tiep tuc.", name, (GetPVarInt(playerid, "charSignUp_gender") == 0) ? "Nam" : "Nu", GetPVarInt(playerid, "charSignUp_skin"), birthday);

                ShowPlayerDialog(playerid, DIALOG_CHAR_SIGNUP_CONFIRM, DIALOG_STYLE_MSGBOX, "Character | Sign Up | Confirm", string, EMBED_COLOR_ALT"Xac nhan", EMBED_COLOR_ALT"<<");
            }
            else {
                DeletePVar(playerid, "charSignUp_Skin");
                ShowPlayerDialog(playerid, DIALOG_CHAR_SIGNUP_SKIN, DIALOG_STYLE_INPUT, "Character | Sign Up | Skin", "{FFFFFF}Nhap ID skin cho nhan vat:", ">>", "<<");
            }
        }
        case DIALOG_CHAR_SIGNUP_CONFIRM:
        {
            if(response)
            {
                new uuid[UUID_LEN],
                    account_uuid[UUID_LEN],
                    name[MAX_PLAYER_NAME + 1],
                    birthday[12];

                UUID(uuid, sizeof(uuid));
                Account_GetUUID(playerid, account_uuid, sizeof(account_uuid));
                GetPVarString(playerid, "charSignUp_name", name, sizeof(name));
                GetPVarString(playerid, "charSignUp_birthday", birthday, sizeof(birthday));
                
                await mysql_aquery_s(MySQL_GetHandle(), str_format("INSERT INTO characters (uuid, account_uuid, name, gender, skin, birthday) VALUES ('%e', '%e', '%e', %d, %d, '%e')", uuid, account_uuid, name, GetPVarInt(playerid, "charSignUp_gender"), GetPVarInt(playerid, "charSignUp_skin"), birthday));

                if(cache_affected_rows() == 0)
                {
                    Player_Kick(playerid, -1, MSG_PREFIX_ALT"Loi khong xac dinh xay ra trong luc tao nhan vat, vui long bao loi hoac dang nhap lai.");
                    return 1;
                }
                
                Character_Show(playerid);
            }
            else
            {
                DeletePVar(playerid, "charSignUp_birthday");
                ShowPlayerDialog(playerid, DIALOG_CHAR_SIGNUP_BIRTHDAY, DIALOG_STYLE_INPUT, "Character | Sign Up | Birthday", "{FFFFFF}Nhap ngay-thang-nam sinh cua nhan vat:", ">>", "<<");
            }
        }
    }
    return 1;
}


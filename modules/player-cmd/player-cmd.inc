#if defined _module_player_cmd_included
    #endinput
#endif

#define _module_player_cmd_included

#include <Pawn.CMD>

#include <config-colors>

#include <utils>

#include <character>
#include <inventory>
#include <vehicle>
#include <portal>

static lastCmdUseTimestamp[MAX_PLAYERS] = {0, ...};

static CmdArray:PC_array;
static PC_cmdNames[1000][32];
static PC_cmdFlags[1000];

CMD:inv(playerid, params[])
{
    Inventory_Show(playerid, 0);
    return 1;
}
alias:inv("inventory");

CMD:enter(playerid)
{
    new areas[20],
        count = GetPlayerNumberDynamicAreas(playerid),
        entrance_extra_id = Portal_GetEntranceExtraId(); 
        
    GetPlayerDynamicAreas(playerid, areas, sizeof(areas));

    for(new i = 0, j = sizeof(areas); i != j; i++)
    {
        if(i >= count)
        {
            break;
        }

        if(Streamer_GetIntData(STREAMER_TYPE_AREA, areas[i], E_STREAMER_EXTRA_ID) == entrance_extra_id)
        {
            new portal_index = Portal_GetByEntranceAreaId(areas[i]);
            if(portal_index != -1)
            {
                Portal_StreamPlayerToExit(playerid, portal_index);
                break;
            }
        }
    }
    return 1;
}

CMD:exit(playerid)
{
    new areas[20],
        count = GetPlayerNumberDynamicAreas(playerid),
        exit_extra_id = Portal_GetExitExtraId(); 
        
    GetPlayerDynamicAreas(playerid, areas, sizeof(areas));

    for(new i = 0, j = sizeof(areas); i != j; i++)
    {
        if(i >= count)
        {
            break;
        }

        if(Streamer_GetIntData(STREAMER_TYPE_AREA, areas[i], E_STREAMER_EXTRA_ID) == exit_extra_id)
        {
            new portal_index = Portal_GetByExitAreaId(areas[i]);
            if(portal_index != -1)
            {
                Portal_StreamPlayerToEntrance(playerid, portal_index);
                break;
            }
        }
    }
    return 1;
}

CMD:vehicle(playerid, params[])
{
    if(isnull(params))
    {
        Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /v(ehicle) [choice]");
        Player_Msg(playerid, COLOR_FADE2, "* choice: get, insertkey, takekey, checkkey, engine, lights, lock, trunk, hood, park, buypark.");
        return 1;
    }

    if(!strcmp(params, "get", true))
    {
        task_yield(1);

        new uuid[UUID_LEN];
        Character_GetUUID(playerid, uuid, sizeof(uuid));
        await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT * FROM personal_vehicles WHERE owner_uuid = '%e'", uuid));
        
        new rows = cache_num_rows();
        if(rows == 0)
        {
            Player_Msg(playerid, COLOR_RED, "> ERROR: Ban khong co so huu bat ki phuong tien nao.");
        }
        else
        {
            new string[21 + (10 + 17 + 37) * MAX_CHARACTER_PERSONAL_VEHICLES],
                model,
                name[17];
            strcat(string, "Ten phuong tien\tUUID");
            for(new i = 0; i < rows; i++)
            {
                cache_get_value_name_int(i, "model", model);
                cache_get_value_name(i, "uuid", uuid, sizeof(uuid));

                Vehicle_GetNameByModel(model, name, sizeof(name));

                format(string, sizeof(string), "%s\n%s\t%s", string, name, uuid);

                format(name, sizeof(name), "veh_UUID_%d", i);
                SetPVarString(playerid, name, uuid);
            }
            
            ShowPlayerDialog(playerid, DIALOG_CHAR_PERSONAL_VEHICLE, DIALOG_STYLE_TABLIST_HEADERS, "Character | Vehicles", string, "Chon", "Tat");
        }
    }
    else if(!strcmp(params, "insertkey", true))
    {
        if(!IsPlayerInAnyVehicle(playerid) || GetPlayerVehicleSeat(playerid) != 0) return Player_Msg(playerid, COLOR_RED, "> ERROR: Ban can phai ngoi o ghe tai xe cua mot phuong tien.");

        new vehicleid = GetPlayerVehicleID(playerid);
        if(Vehicle_IsKeyInserted(vehicleid)) return Player_Msg(playerid, COLOR_RED, "> ERROR: Phuong tien da co san mot chia khoa.");

        new key[24];
        Vehicle_GetKey(vehicleid, key, sizeof(key));
        
        if(isnull(key))
        {
            Vehicle_SetKeyInserted(vehicleid, true);
            Player_Msg(playerid, COLOR_GREEN, "> Info: Ban da dut chia khoa vao phuong tien, co the bat dau su dung.");
        }
        else
        {
            new index = Inventory_GetByIdAndKey(playerid, 44, key);
            if(index == -1)
            {
                Player_Msg(playerid, COLOR_RED, "> ERROR: Tren nguoi ban khong co bat ki chia khoa nao phu hop voi phuong tien.");
            }
            else
            {
                Vehicle_SetKeyInserted(vehicleid, true);
                Player_Msg(playerid, COLOR_GREEN, "> Info: Ban da dut chia khoa vao phuong tien, co the bat dau su dung.");
                Inventory_RemoveByIndex(playerid, index);
            }
        }
    }
    else if(!strcmp(params, "takekey", true))
    {
        if(!IsPlayerInAnyVehicle(playerid) || GetPlayerVehicleSeat(playerid) != 0) return Player_Msg(playerid, COLOR_RED, "> ERROR: Ban can phai ngoi o ghe tai xe cua mot phuong tien.");

        new vehicleid = GetPlayerVehicleID(playerid);
        if(!Vehicle_IsKeyInserted(vehicleid)) return Player_Msg(playerid, COLOR_RED, "> ERROR: Phuong tien chua co bat ki chia khoa nao.");
        
        new key[24];
        Vehicle_GetKey(vehicleid, key, sizeof(key));
        
        if(!isnull(key))
        {
            Inventory_Add(playerid, 44, 1, 1, key);
        }
        
        Vehicle_SetParams(vehicleid, false, false, Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_ALARM), Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_DOORS), Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_BONNET), Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_BOOT), Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_OBJECTIVE));
        
        Player_Msg(playerid, COLOR_GREEN, "> Info: Ban da lay chia khoa tu phuong tien.");
        Vehicle_SetKeyInserted(vehicleid, false);
    }
    else if(!strcmp(params, "checkkey", true))
    {
        if(!IsPlayerInAnyVehicle(playerid) || GetPlayerVehicleSeat(playerid) != 0) return Player_Msg(playerid, COLOR_RED, "> ERROR: Ban can phai ngoi o ghe tai xe cua mot phuong tien.");

        new key[24],
            vehicleid = GetPlayerVehicleID(playerid);
        Vehicle_GetKey(vehicleid, key, sizeof(key));
        
        if(!isnull(key))
        {
            new string[24 + 45];
            format(string, sizeof(string), "> Info: Chia khoa cua phuong tien nay la: %s.", key);
            Player_Msg(playerid, COLOR_GREEN, string);
        }
        else
        {
            Player_Msg(playerid, COLOR_RED, "> ERROR: Phuong tien khong can chia khoa.");
        }
        
    }
    else if(!strcmp(params, "engine", true))
    {
        if(!IsPlayerInAnyVehicle(playerid) || GetPlayerVehicleSeat(playerid) != 0) return Player_Msg(playerid, COLOR_RED, "> ERROR: Ban can phai ngoi o ghe tai xe cua mot phuong tien.");
        
        new vehicleid = GetPlayerVehicleID(playerid),
            model = GetVehicleModel(vehicleid);
        if(model == 510 || model == 509 || model == 481) return Player_Msg(playerid, COLOR_RED, "> ERROR: Xe dap khong phu hop cho lua chon nay.");

        if(!Vehicle_IsKeyInserted(vehicleid)) return Player_Msg(playerid, COLOR_RED, "> ERROR: Phuong tien chua duoc dut chia khoa vao.");

        if(Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_ENGINE))
        {
            Player_Msg(playerid, COLOR_GREEN, "> Info: Phuong tien da tat dong co.");
        }
        else
        {
            Player_Msg(playerid, COLOR_GREEN, "> Info: Phuong tien da khoi dong.");
        }

        Vehicle_ToggleParam(vehicleid, VEHICLE_PARAMS_ENGINE);
    }
    else if(!strcmp(params, "lights", true))
    {
        if(!IsPlayerInAnyVehicle(playerid) || GetPlayerVehicleSeat(playerid) != 0) return Player_Msg(playerid, COLOR_RED, "> ERROR: Ban can phai ngoi o ghe tai xe cua mot phuong tien.");
        
        new vehicleid = GetPlayerVehicleID(playerid),
            model = GetVehicleModel(vehicleid);
        if(model == 510 || model == 509 || model == 481) return Player_Msg(playerid, COLOR_RED, "> ERROR: Xe dap khong phu hop cho lua chon nay.");

        if(!Vehicle_IsKeyInserted(vehicleid)) return Player_Msg(playerid, COLOR_RED, "> ERROR: Phuong tien chua duoc dut chia khoa vao.");

        if(Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_LIGHTS))
        {
            Player_Msg(playerid, COLOR_GREEN, "> Info: Phuong tien da tat den.");
        }
        else
        {
            Player_Msg(playerid, COLOR_GREEN, "> Info: Phuong tien da mo den.");
        }

        Vehicle_ToggleParam(vehicleid, VEHICLE_PARAMS_LIGHTS);
    }
    else if(!strcmp(params, "lock", true))
    {
        new vehicleid = (IsPlayerInAnyVehicle(playerid)) ? GetPlayerVehicleID(playerid) : Player_GetClosestVehicleId(playerid, 3.0);
        if(vehicleid == 0) return Player_Msg(playerid, COLOR_RED, "> ERROR: Ban phai ngoi ben trong hoac o gan mot phuong tien.");

        if(Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_DOORS))
        {
            new key[24];
            Vehicle_GetKey(vehicleid, key, sizeof(key));
            
            if(!isnull(key))
            {
                new index = Inventory_GetByIdAndKey(playerid, 44, key);
                if(index == -1)
                {
                    return Player_Msg(playerid, COLOR_RED, "> ERROR: Ban khong co chia khoa cua phuong tien nay.");
                }
            }
        }

        if(Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_DOORS))
        {
            Player_Msg(playerid, COLOR_GREEN, "> Info: Phuong tien da mo khoa.");
        }
        else
        {
            Player_Msg(playerid, COLOR_GREEN, "> Info: Phuong tien da dong khoa.");
        }

        Vehicle_ToggleParam(vehicleid, VEHICLE_PARAMS_DOORS);
    }
    else if(!strcmp(params, "trunk", true))
    {
        new vehicleid = Player_GetClosestVehicleId(playerid, 5.0);

        if(vehicleid == 0)
        {
            return Player_Msg(playerid, COLOR_RED, "> ERROR: Ban can phai o gan mot phuong tien.");
        }

        new Float:model_pos[3],
            Float:pos[3];
        GetVehicleModelInfo(GetVehicleModel(vehicleid), VEHICLE_MODEL_INFO_WHEELSREAR, model_pos[0], model_pos[1], model_pos[2]);
        Utils_GetVehicleRelativePos(vehicleid, pos[0], pos[1], pos[2], model_pos[0] - 1.0, model_pos[1] - 0.5, model_pos[2]);

        if(IsPlayerInRangeOfPoint(playerid, 2.0, pos[0], pos[1], pos[2]))
        {
            if(Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_BOOT))
            {
                Vehicle_ToggleParam(vehicleid, VEHICLE_PARAMS_BOOT);

                Player_Msg(playerid, COLOR_GREEN, "> Info: Cop cua phuong tien da duoc dong.");
            }
            else
            {
                if(!Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_DOORS))
                {
                    Vehicle_ToggleParam(vehicleid, VEHICLE_PARAMS_BOOT);

                    Player_Msg(playerid, COLOR_GREEN, "> Info: Cop cua phuong tien da duoc mo.");
                }
                else Player_Msg(playerid, COLOR_RED, "> ERROR: Phuong tien da bi khoa, khong the mo cop.");
            }
        }
        else Player_Msg(playerid, COLOR_RED, "> ERROR: Ban khong o gan cop cua phuong tien.");
    }
    else if(!strcmp(params, "hood", true))
    {
        new vehicleid = Player_GetClosestVehicleId(playerid, 5.0);

        if(vehicleid == 0)
        {
            return Player_Msg(playerid, COLOR_RED, "> ERROR: Ban can phai o gan mot phuong tien.");
        }
        
        new Float:model_pos[3],
            Float:pos[3];
        GetVehicleModelInfo(GetVehicleModel(vehicleid), VEHICLE_MODEL_INFO_WHEELSFRONT, model_pos[0], model_pos[1], model_pos[2]);
        Utils_GetVehicleRelativePos(vehicleid, pos[0], pos[1], pos[2], model_pos[0] - 1.0, model_pos[1] + 0.5, model_pos[2]);

        if(IsPlayerInRangeOfPoint(playerid, 2.0, pos[0], pos[1], pos[2]))
        {
            if(Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_BONNET))
            {
                Vehicle_ToggleParam(vehicleid, VEHICLE_PARAMS_BONNET);

                Player_Msg(playerid, COLOR_GREEN, "> Info: Nap cua phuong tien da duoc dong.");
            }
            else
            {
                if(!Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_DOORS))
                {
                    Vehicle_ToggleParam(vehicleid, VEHICLE_PARAMS_BONNET);

                    Player_Msg(playerid, COLOR_GREEN, "> Info: Nap cua phuong tien da duoc mo.");
                }
                else Player_Msg(playerid, COLOR_RED, "> ERROR: Phuong tien da bi khoa, khong the mo nap.");
            }
        }
        else Player_Msg(playerid, COLOR_RED, "> ERROR: Ban khong o gan nap cua phuong tien.");
    }
    else if(!strcmp(params, "park", true))
    {
        if(!IsPlayerInAnyVehicle(playerid) || GetPlayerVehicleSeat(playerid) != 0) return Player_Msg(playerid, COLOR_RED, "> ERROR: Ban can phai ngoi tren ghe tai xe cua mot phuong tien.");

        new vehicleid = GetPlayerVehicleID(playerid);

        if(Vehicle_GetType(vehicleid) != VEHICLE_TYPE_PERSONAL) return Player_Msg(playerid, COLOR_RED, "> ERROR: Phuong tien nay khong phu hop de dau xe.");

        new key[24];
        Vehicle_GetKey(vehicleid, key, sizeof(key));

        task_yield(1);
        new result = await PersonalVehicle_Park(vehicleid);

        if(result < 0)
        {
            Player_Msg(playerid, COLOR_RED, sprintf("> ERROR: Da co loi xay ra trong qua trinh dau phuong tien (error code: %d).", result));
        }
        else
        {
            //Inventory_Add(playerid, 44, 1, 1, key); // give the key back to player after parking the vehicle.

            Player_Msg(playerid, COLOR_GREEN, "> Info: Phuong tien cua ban da duoc dau thanh cong.");
        }
    }
    else if(!strcmp(params, "buypark", true))
    {
        if(!IsPlayerInAnyVehicle(playerid) || GetPlayerVehicleSeat(playerid) != 0) return Player_Msg(playerid, COLOR_RED, "> ERROR: Ban can phai ngoi tren ghe tai xe cua mot phuong tien.");

        new vehicleid = GetPlayerVehicleID(playerid);

        if(!Vehicle_IsKeyInserted(vehicleid)) return Player_Msg(playerid, COLOR_RED, "> ERROR: Phuong tien chua duoc cam chia khoa.");

        if(Vehicle_GetType(vehicleid) != VEHICLE_TYPE_PERSONAL) return Player_Msg(playerid, COLOR_RED, "> ERROR: Phuong tien nay khong phu hop de mua vi tri dau xe.");

        new Float:pos[4],
            uuid[UUID_LEN];
        GetVehiclePos(vehicleid, pos[0], pos[1], pos[2]);
        GetVehicleZAngle(vehicleid, pos[3]);
        PersonalVehicle_GetUUID(vehicleid, uuid, sizeof(uuid));

        task_yield(1);
        if(task_await(PersonalVehicle_SetParkingPos(uuid, pos[0], pos[1], pos[2], pos[3], Vehicle_GetInterior(vehicleid), Vehicle_GetWorld(vehicleid))) > 0)
        {
            Player_Msg(playerid, COLOR_GREEN, "> Info: Ban da mua vi tri dau xe cho phuong tien thanh cong, su dung /v park de dau phuong tien.");
        }
        else
        {
            Player_Msg(playerid, COLOR_RED, "> ERROR: Da co loi xay ra trong qua trinh mua vi tri dau xe.");
        }
    }
    else
    {
        Player_Msg(playerid, COLOR_FADE2, "* choice: get, insertkey, takekey, checkkey, engine, lights, lock, trunk, hood, park, buypark.");
    }
    return 1;
}
alias:vehicle("v", "veh");

flags:avehicle(CMD_FLAG_ADMIN_1 | CMD_FLAG_ADMIN_2 | CMD_FLAG_ADMIN_3 | CMD_FLAG_ADMIN_4 | CMD_FLAG_ADMIN_5);
CMD:avehicle(playerid, params[])
{
    new model,
        colors[2],
        siren;
    if(sscanf(params, "iI(0)I(1)I(0)", model, colors[0], colors[1], siren)) return Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /avehicle [model] (primary color) (secondary color) (siren)");

    new Float:pos[3],
        Float:angle;
    GetPlayerPos(playerid, pos[0], pos[1], pos[2]);
    GetPlayerFacingAngle(playerid, angle);
    Utils_GetPlayerForwardPos(playerid, pos[0], pos[1], 3.0);

    CreateVehicle(model, pos[0], pos[1], pos[2], angle, colors[0], colors[1], -1, siren);
    return 1;
}
alias:avehicle("aveh");

flags:aweapon(CMD_FLAG_ADMIN_1 | CMD_FLAG_ADMIN_2 | CMD_FLAG_ADMIN_3 | CMD_FLAG_ADMIN_4 | CMD_FLAG_ADMIN_5);
CMD:aweapon(playerid, params[])
{
    new weapon_id,
        ammo;
    if(sscanf(params, "iI(100)", weapon_id, ammo)) return Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /aweapon [weapon id] (ammo)");

    GivePlayerWeapon(playerid, weapon_id, ammo);
    return 1;
}
alias:aweapon("aweap");

flags:agiveitem(CMD_FLAG_ADMIN_3 | CMD_FLAG_ADMIN_4 | CMD_FLAG_ADMIN_5);
CMD:agiveitem(playerid, params[])
{
    new targetid,
        item_id,
        amount,
        number_of_uses,
        key[24];
    if(sscanf(params, "iiI(1)I(1)S()[24]", targetid, item_id, amount, number_of_uses, key)) return Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /agiveitem [playerid] [item id] (amount) (number of uses) (key)");

    if(!Character_IsSpawned(targetid)) return Player_Msg(playerid, -1, "> ERROR: Nguoi choi chua dang nhap vao nhan vat.");

    task_yield(1);
    new result = await Inventory_Add(targetid, item_id, amount, number_of_uses, key);

    if(result == 0)
    {
        Player_Msg(playerid, COLOR_ERROR, "> ERROR: Item ID khong ton tai trong co so du lieu.");
    }
    else
    {
        new string[82 + 32 + 64 + 24],
            name[MAX_PLAYER_NAME + 1],
            item_name[64];
        Character_GetName(targetid, name, sizeof(name));

        task_yield(1);
        await_arr(item_name) Item_RetrieveName(item_id);

        format(string, sizeof(string), "> Info: Ban da cho '%s' vat pham '%s' (so luong: %d | so lan su dung: %d | key: %s)", name, item_name, amount, number_of_uses, (isnull(key)) ? "empty" : key);
        Player_Msg(playerid, COLOR_GREEN, string);
    }
    return 1;
}

flags:acreatepvehicle(CMD_FLAG_ADMIN_4 | CMD_FLAG_ADMIN_5);
CMD:acreatepvehicle(playerid, params[])
{
    new targetid,
        model,
        primary_color,
        secondary_color,
        siren;
    if(sscanf(params, "iiI(0)I(1)I(0)", targetid, model, primary_color, secondary_color, siren)) return Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /acreatepveh(icle) [playerid] [model] (primary color) (secondary color) (siren)");

    if(!Character_IsSpawned(targetid)) return Player_Msg(playerid, COLOR_RED, "> ERROR: Nguoi choi chua dang nhap vao nhan vat.");

    if(model < 400 || model > 611) return Player_Msg(playerid, COLOR_RED, "> ERROR: Model phuong tien khong hop le.");

    new target_uuid[UUID_LEN];
    Character_GetUUID(targetid, target_uuid, sizeof(target_uuid));

    task_yield(1);
    new count = await PersonalVehicle_CountByOwner(target_uuid);

    if(count >= MAX_CHARACTER_PERSONAL_VEHICLES) return Player_Msg(playerid, COLOR_RED, "> ERROR: Nguoi choi da so huu toi da phuong tien.");
    
    new Float:pos[3],
        Float:angle;
    GetPlayerPos(targetid, pos[0], pos[1], pos[2]);
    Utils_GetPlayerForwardPos(targetid, pos[0], pos[1], 3.5);
    GetPlayerFacingAngle(targetid, angle);
    
    task_yield(1);
    new index = await PersonalVehicle_Create(target_uuid, model, pos[0], pos[1], pos[2], angle, primary_color, secondary_color, GetPlayerInterior(targetid), GetPlayerVirtualWorld(targetid), -1, siren);
    if(index != -1)
    {
        new vehicle_name[17],
            char_name[MAX_PLAYER_NAME + 1];
        
        Character_GetName(playerid, char_name, sizeof(char_name));
        Vehicle_GetNameByModel(model, vehicle_name, sizeof(vehicle_name));
        Player_Msg(playerid, COLOR_GREEN, sprintf("> Info: Ban da tao phuong tien '%s' cho '%s'.", vehicle_name, char_name));
    }
    else
    {
        Player_Msg(playerid, COLOR_RED, "> ERROR: Da co loi xay ra trong luc tao phuong tien, vui long bao loi.");
    }
    return 1;
}
alias:acreatepvehicle("acreatepveh");

flags:apotl(CMD_FLAG_ADMIN_3 | CMD_FLAG_ADMIN_4 | CMD_FLAG_ADMIN_5);
CMD:apotl(playerid, params[])
{
    new choice[16],
        portal_index;
    if(sscanf(params, "s[16]I(-1)", choice, portal_index))
    {
        Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /apotl [choice] (portal index)");
        Player_Msg(playerid, COLOR_FADE2, "* choice: create, entrance, exit, togglelock, destroy");
        return 1;
    }

    if(!strcmp(choice, "create", true))
    {
        #pragma unused portal_index
        
        Player_Msg(playerid, COLOR_GREEN, sprintf("> Info: Ban da tao ra portal index %d, hay su dung portal index cho lua chon khac nhu 'entrance', 'exit',...", Portal_Create()));
    }
    else if(!strcmp(choice, "entrance", true))
    {
        if(portal_index < 0 || portal_index >= Portal_GetSize())
        {
            Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /apotl entrance (portal index)");
            Player_Msg(playerid, COLOR_FADE2, "* portal index khong hop le.");
            return 1;
        }

        new Float:pos[3],
            Float:angle;
        
        GetPlayerPos(playerid, pos[0], pos[1], pos[2]);
        GetPlayerFacingAngle(playerid, angle);
        Portal_SetEntrance(portal_index, pos[0], pos[1], pos[2], angle, GetPlayerInterior(playerid), GetPlayerVirtualWorld(playerid));

        Player_Msg(playerid, COLOR_GREEN, sprintf("> Info: Ban da dieu chinh loi vao cua portal %d ve vi tri hien tai cua ban.", portal_index));
    }
    else if(!strcmp(choice, "exit", true))
    {
        if(portal_index < 0 || portal_index >= Portal_GetSize())
        {
            Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /apotl exit (portal index)");
            Player_Msg(playerid, COLOR_FADE2, "* portal index khong hop le.");
            return 1;
        }

        new Float:pos[3],
            Float:angle;
        
        GetPlayerPos(playerid, pos[0], pos[1], pos[2]);
        GetPlayerFacingAngle(playerid, angle);
        Portal_SetExit(portal_index, pos[0], pos[1], pos[2], angle, GetPlayerInterior(playerid), GetPlayerVirtualWorld(playerid));

        Player_Msg(playerid, COLOR_GREEN, sprintf("> Info: Ban da dieu chinh loi ra cua portal %d ve vi tri hien tai cua ban.", portal_index));
    }
    else if(!strcmp(choice, "togglelock", true))
    {
        if(portal_index < 0 || portal_index >= Portal_GetSize())
        {
            Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /apotl exit (portal index)");
            Player_Msg(playerid, COLOR_FADE2, "* portal index khong hop le.");
            return 1;
        }

        if(Portal_GetLock(portal_index))
        {
            Player_Msg(playerid, COLOR_GREEN, sprintf("> Info: Ban da khoa portal index %d.", portal_index));
        }
        else
        {
            Player_Msg(playerid, COLOR_GREEN, sprintf("> Info: Ban da mo khoa portal index %d.", portal_index));
        }
        Portal_ToggleLock(portal_index);
    }
    else if(!strcmp(choice, "destroy", true))
    {
        if(portal_index < 0 || portal_index >= Portal_GetSize())
        {
            Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /apotl exit (portal index)");
            Player_Msg(playerid, COLOR_FADE2, "* portal index khong hop le.");
            return 1;
        }

        if(Portal_DestroyByIndex(portal_index) == 1)
        {
            Player_Msg(playerid, COLOR_GREEN, sprintf("> Info: Ban da xoa portal index %d thanh cong.", portal_index));
        }
        else
        {
            Player_Msg(playerid, COLOR_RED, sprintf("> ERROR: Xoa portal index %d that bai.", portal_index));
        }
    }
    else 
    {
        Player_Msg(playerid, COLOR_FADE2, "* choice: create, entrance, exit, togglelock, destroy");
    }
    return 1;
}

public OnPlayerCommandReceived(playerid, cmd[], params[], flags)
{
    if(!Character_IsSpawned(playerid))
    {
        Player_Msg(playerid, COLOR_RED, "> ERROR: Ban can dang nhap vao nhan vat truoc khi su dung lenh.");
        return 0;
    } 
    else if(!(flags & Character_GetCmdFlags(playerid)) && flags != 0)
    {
        Player_Msg(playerid, COLOR_RED, "> ERROR: Ban khong du quyen han de su dung lenh nay.");
        return 0;
    }
    else if((gettime() - lastCmdUseTimestamp[playerid]) < 1)
    {
        Player_Msg(playerid, COLOR_FADE3, "> ERROR: Moi lan su dung lenh phai cach nhau it nhat 1 giay.");
        return 0;
    }
    return 1;
}

public OnPlayerCommandPerformed(playerid, cmd[], params[], result, flags)
{
    if(result == -1)
    {
        Player_Msg(playerid, COLOR_RED, "> ERROR: Lenh ban vua nhap khong ton tai.");
        
        if(strlen(cmd) >= 2)
        {
            new string[200],
                bool:similarCmdsFound = false;
            for (new i, j = PC_GetArraySize(PC_array); i != j; i++)
            {
                if(Utils_CountCommonPrefix(PC_cmdNames[i], cmd) > 2 || strfind(PC_cmdNames[i], cmd, true) != -1)
                {
                    if((PC_cmdFlags[i] != 0 && (PC_cmdFlags[i] & Character_GetCmdFlags(playerid))) || PC_cmdFlags[i] == 0)
                    {
                        if(strlen(string) > 170)
                        {
                            break;
                        }
                        format(string, sizeof(string), "%s/%s, ", string, PC_cmdNames[i]);
                        similarCmdsFound = true;
                    }
                }
            }

            if(similarCmdsFound)
            {
                string[strlen(string)-2] = EOS;
                strcat(string, " "EMBED_COLOR_YELLOW"<");
                Player_Msg(playerid, COLOR_FADE3, string);
            }
        }
        return 0;
    }
    else
    {
        lastCmdUseTimestamp[playerid] = gettime();
    }
    return 1;
}

public PC_OnInit()
{
    PC_array = PC_GetCommandArray();
    for (new i = 0, j = PC_GetArraySize(PC_array); i != j; i++)
    {
        PC_GetCommandName(PC_array, i, PC_cmdNames[i]);
        PC_cmdFlags[i] = PC_GetFlags(PC_cmdNames[i]);
    }
    return 1;
}

#include <YSI_Coding\y_hooks>

hook OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    switch(dialogid)
    {
        case DIALOG_CHAR_PERSONAL_VEHICLE:
        {
            if(response)
            {
                new var_name[17];
                format(var_name, sizeof(var_name), "veh_UUID_%d", listitem);
                
                if(GetPVarType(playerid, var_name) == PLAYER_VARTYPE_STRING)
                {
                    new uuid[UUID_LEN];
                    GetPVarString(playerid, var_name, uuid, sizeof(uuid));

                    if(PersonalVehicle_GetByUUID(uuid) != 0)
                    {
                        Player_Msg(playerid, COLOR_RED, "> ERROR: Phuong tien da duoc lay ra truoc do.");
                    }
                    else
                    {
                        task_yield(1);

                        new result = await PersonalVehicle_Spawn(uuid);
                        if(result == -1)
                        {
                            Player_Msg(playerid, COLOR_RED, "> ERROR: Da co loi xay ra trong luc lay phuong tien, vui long lay lai hoac bao loi.");
                        }
                        else
                        {
                            Player_Msg(playerid, COLOR_GREEN, "> Info: Phuong tien da xuat hien tai vi tri dau xe.");
                        }
                    }  
                }
                else
                {
                    Player_Msg(playerid, COLOR_RED, "> ERROR: Da co loi xay ra, vui long [/v get] lai.");
                }
            }
            DeletePVar(playerid, "veh_UUID_0");
            DeletePVar(playerid, "veh_UUID_1");
            DeletePVar(playerid, "veh_UUID_2");
            DeletePVar(playerid, "veh_UUID_3");
            DeletePVar(playerid, "veh_UUID_4");
            DeletePVar(playerid, "veh_UUID_5");
        }
    }
    return 1;
}
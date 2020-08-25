#if defined _module_player_chat_included
#endinput
#endif

#define _module_player_chat_included

#include <Pawn.CMD>
#include <YSF>

#include <config-colors>

#include <utils>

#include <character>
#include <inventory>
#include <vehicle>
#include <portal>
#include <house>
#include <house-furniture>

static lastCmdUseTimestamp[MAX_PLAYERS] = { 0, ... };

static CmdArray:PC_array;
static PC_cmdNames[1000][32];
static PC_cmdFlags[1000];

/*static PlayerText:TD_Circles[MAX_PLAYERS][10] = {{PlayerText:INVALID_TEXT_DRAW, ...}, ...};
static PlayerText:TD_Center_Point[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...};
static currentCircleAngle[MAX_PLAYERS] = {0, ...};

stock static HEX(&color, rr, gg, bb, aa) {
	color = (aa & 0xFF) | ((bb & 0xFF) << 8) | ((gg & 0xFF) << 16) | (rr << 24);
}

CMD:circle(playerid, params[])
{
    new const Float:CENTER_X = 240.0000;
    new const Float:CENTER_Y = 240.0000;
    new const Float:radius = 15.0;
    new const Float:size = 8.0;
    new color;

    if(TD_Center_Point[playerid] == PlayerText:INVALID_TEXT_DRAW)
    {
        TD_Center_Point[playerid] = CreatePlayerTextDraw(playerid, CENTER_X, CENTER_Y, "ld_beat:chit");
        PlayerTextDrawFont(playerid, TD_Center_Point[playerid], 4);
        PlayerTextDrawLetterSize(playerid, TD_Center_Point[playerid], 0.600000, 2.000000);
        PlayerTextDrawTextSize(playerid, TD_Center_Point[playerid], 5.000000, 5.000000);
        PlayerTextDrawSetOutline(playerid, TD_Center_Point[playerid], 1);
        PlayerTextDrawSetShadow(playerid, TD_Center_Point[playerid], 0);
        PlayerTextDrawAlignment(playerid, TD_Center_Point[playerid], 1);
        PlayerTextDrawColor(playerid, TD_Center_Point[playerid], -1);
        PlayerTextDrawBackgroundColor(playerid, TD_Center_Point[playerid], 255);
        PlayerTextDrawBoxColor(playerid, TD_Center_Point[playerid], 50);
        PlayerTextDrawUseBox(playerid, TD_Center_Point[playerid], 1);
        PlayerTextDrawSetProportional(playerid, TD_Center_Point[playerid], 1);
        PlayerTextDrawSetSelectable(playerid, TD_Center_Point[playerid], 0);

        PlayerTextDrawShow(playerid, TD_Center_Point[playerid]);
    }

    for(new i = 0; i < 10; i++)
    {
        if(TD_Circles[playerid][i] == PlayerText:INVALID_TEXT_DRAW)
        {
            HEX(color, random(255), random(255), random(255), 255 - i * 5);
            TD_Circles[playerid][i] = CreatePlayerTextDraw(playerid, CENTER_X + radius * floatcos((0 - (size - 2.0) * i) * 3.14 / 180, radian), CENTER_Y + radius * floatsin((0 - (size - 2.0) * i) * 3.14 / 180, radian), "ld_beat:chit");
            PlayerTextDrawFont(playerid, TD_Circles[playerid][i], 4);
            PlayerTextDrawLetterSize(playerid, TD_Circles[playerid][i], 0.600000, 2.000000);
            PlayerTextDrawTextSize(playerid, TD_Circles[playerid][i], size, size);
            PlayerTextDrawSetOutline(playerid, TD_Circles[playerid][i], 1);
            PlayerTextDrawSetShadow(playerid, TD_Circles[playerid][i], 0);
            PlayerTextDrawAlignment(playerid, TD_Circles[playerid][i], 1);
            PlayerTextDrawColor(playerid, TD_Circles[playerid][i], color);
            PlayerTextDrawBackgroundColor(playerid, TD_Circles[playerid][i], 255);
            PlayerTextDrawBoxColor(playerid, TD_Circles[playerid][i], 50);
            PlayerTextDrawUseBox(playerid, TD_Circles[playerid][i], 1);
            PlayerTextDrawSetProportional(playerid, TD_Circles[playerid][i], 1);
            PlayerTextDrawSetSelectable(playerid, TD_Circles[playerid][i], 0);
        }
        PlayerTextDrawShow(playerid, TD_Circles[playerid][i]);
    }

    currentCircleAngle[playerid] = 0;
    SetPVarInt(playerid, "circleLoadingTimer", SetPreciseTimer("circleLoading", 33, true, "iffff", playerid, radius, size, CENTER_X, CENTER_Y));
    return 1;
}

forward circleLoading(const playerid, Float:radius, Float:size, Float:center_x, Float:center_y);
public circleLoading(const playerid, Float:radius, Float:size, Float:center_x, Float:center_y)
{
    currentCircleAngle[playerid] += 5;

    for(new i = 0; i < 10; i++)
    {
        PlayerTextDrawSetPos(playerid, TD_Circles[playerid][i], center_x + radius * floatcos(currentCircleAngle[playerid] - (size - 2.0) * i, degrees), center_y + radius * floatsin(currentCircleAngle[playerid] - (size - 2.0) * i, degrees));

        PlayerTextDrawShow(playerid, TD_Circles[playerid][i]);
    }

    if(currentCircleAngle[playerid] >= 360)
    {
        currentCircleAngle[playerid] = 0;
        DeletePreciseTimer(GetPVarInt(playerid, "circleLoadingTimer"));
        DeletePVar(playerid, "circleLoadingTimer");
    }
    return 1;
}*/

public OnPlayerCommandReceived(playerid, cmd[], params[], flags) {
    if(!Character_IsSpawned(playerid)) {
        Player_Msg(playerid, COLOR_FADE3, "Ban can dang nhap vao nhan vat truoc khi su dung lenh.");
        return 0;
    } else if(!(flags & Character_GetCmdFlags(playerid)) && flags != 0) {
        Player_Msg(playerid, COLOR_FADE3, "Ban khong du quyen han de su dung lenh nay.");
        return 0;
    } else if((gettime() - lastCmdUseTimestamp[playerid]) < 1) {
        Player_Msg(playerid, COLOR_FADE3, "> ERROR: Moi lan su dung lenh phai cach nhau it nhat 1 giay.");
        return 0;
    }
    return 1;
}

public OnPlayerCommandPerformed(playerid, cmd[], params[], result, flags) {
    if(result == -1) {
        if(strlen(cmd) >= 2) {
            new string[128] = "Lenh khong ton tai, goi y: ",
                bool:similarCmdsFound = false;
            for (new i, j = PC_GetArraySize(PC_array); i != j; i++) {
                if(Utils_CountCommonPrefix(PC_cmdNames[i], cmd) > 2 || strfind(PC_cmdNames[i], cmd, true) != -1) {
                    if((PC_cmdFlags[i] != 0 && (PC_cmdFlags[i] & Character_GetCmdFlags(playerid))) || PC_cmdFlags[i] == 0) {
                        if(strlen(string) > 126) {
                            break;
                        }
                        format(string, sizeof(string), "%s/%s, ", string, PC_cmdNames[i]);
                        similarCmdsFound = true;
                    }
                }
            }

            if(similarCmdsFound) {
                string[strlen(string) - 2] = EOS;
                strcat(string, ".");
                Player_Msg(playerid, COLOR_FADE3, string);

                return 0;
            }
        }

        Player_Msg(playerid, COLOR_FADE3, "Lenh khong ton tai.");
        return 0;
    } else {
        lastCmdUseTimestamp[playerid] = gettime();
    }
    return 1;
}

public PC_OnInit() {
    PC_array = PC_GetCommandArray();
    for (new i = 0, j = PC_GetArraySize(PC_array); i != j; i++) {
        PC_GetCommandName(PC_array, i, PC_cmdNames[i]);
        PC_cmdFlags[i] = PC_GetFlags(PC_cmdNames[i]);
    }
    return 1;
}

CMD:inventory(playerid, params[]) {
    Inventory_Show(playerid, 0);
    return 1;
}
alias:inventory("inv", "tuido")

CMD:enter(playerid) {
    new areas[20],
        count = GetPlayerNumberDynamicAreas(playerid),
        entrance_extra_id = Portal_GetEntranceExtraId();

    GetPlayerDynamicAreas(playerid, areas, sizeof(areas));

    for (new i = 0, j = sizeof(areas); i != j; i++) {
        if(i >= count) {
            break;
        }

        if(Streamer_GetIntData(STREAMER_TYPE_AREA, areas[i], E_STREAMER_EXTRA_ID) == entrance_extra_id) {
            new portal_index = Portal_GetByEntranceAreaId(areas[i]);
            if(portal_index != -1) {
                new result = Portal_StreamPlayerToExit(playerid, portal_index);
                if(result == -1) {
                    Player_Msg(playerid, COLOR_FADE3, "Loi vao chua duoc dieu chinh loi ra.");
                } else if(result == -2) {
                    Player_Msg(playerid, COLOR_FADE3, "Loi vao da bi khoa.");
                } else {
                    return 2;
                }
                break;
            }
        }
    }
    return 1;
}
alias:enter("vao")

CMD:exit(playerid) {
    new areas[20],
        count = GetPlayerNumberDynamicAreas(playerid),
        exit_extra_id = Portal_GetExitExtraId();

    GetPlayerDynamicAreas(playerid, areas, sizeof(areas));

    for (new i = 0, j = sizeof(areas); i != j; i++) {
        if(i >= count) {
            break;
        }

        if(Streamer_GetIntData(STREAMER_TYPE_AREA, areas[i], E_STREAMER_EXTRA_ID) == exit_extra_id) {
            new portal_index = Portal_GetByExitAreaId(areas[i]);
            if(portal_index != -1) {
                new result = Portal_StreamPlayerToEntrance(playerid, portal_index);
                if(result == -1) {
                    Player_Msg(playerid, COLOR_FADE3, "Loi ra chua duoc dieu chinh loi vao.");
                } else if(result == -2) {
                    Player_Msg(playerid, COLOR_FADE3, "Loi ra da bi khoa.");
                } else {
                    return 2;
                }
                break;
            }
        }
    }
    return 1;
}
alias:exit("ra")

CMD:vehicle(playerid, params[]) {
    if(isnull(params)) {
        Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /v(ehicle) [choice]");
        Player_Msg(playerid, COLOR_FADE2, "* choice: get, insertkey, takekey, checkkey, engine, lights, lock, trunk, hood, park, buypark.");
        return 1;
    }

    if(!strcmp(params, "get", true)) {
        task_yield(1);

        new uuid[UUID_LEN];
        Character_GetUUID(playerid, uuid, sizeof(uuid));
        await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT * FROM personal_vehicles WHERE owner_uuid = '%e'", uuid));

        new rows = cache_num_rows();
        if(rows == 0) {
            Player_Msg(playerid, COLOR_FADE3, "Ban khong co so huu bat ki phuong tien nao.");
        } else {
            new string[21 + (10 + 17 + 37) * MAX_CHARACTER_PERSONAL_VEHICLES],
                model,
                name[17];
            strcat(string, "Ten phuong tien\tUUID");
            for (new i = 0; i < rows; i++) {
                cache_get_value_name_int(i, "model", model);
                cache_get_value_name(i, "uuid", uuid, sizeof(uuid));

                Vehicle_GetNameByModel(model, name, sizeof(name));

                format(string, sizeof(string), "%s\n%s\t%s", string, name, uuid);

                format(name, sizeof(name), "veh_UUID_%d", i);
                SetPVarString(playerid, name, uuid);
            }

            ShowPlayerDialog(playerid, DIALOG_CHAR_PERSONAL_VEHICLE, DIALOG_STYLE_TABLIST_HEADERS, "Character | Vehicles", string, "Chon", "Tat");
        }
    } else if(!strcmp(params, "insertkey", true)) {
        if(!IsPlayerInAnyVehicle(playerid) || GetPlayerVehicleSeat(playerid) != 0) return Player_Msg(playerid, COLOR_FADE3, "Ban can phai ngoi o ghe tai xe cua mot phuong tien.");

        new vehicleid = GetPlayerVehicleID(playerid);
        if(Vehicle_IsKeyInserted(vehicleid)) return Player_Msg(playerid, COLOR_FADE3, "Phuong tien da co san mot chia khoa.");

        new key[24];
        Vehicle_GetKey(vehicleid, key, sizeof(key));

        if(isnull(key)) {
            Vehicle_SetKeyInserted(vehicleid, true);
            Player_Msg(playerid, COLOR_GREEN, "Ban da dut chia khoa vao phuong tien, co the bat dau su dung.");
        } else {
            new index = Inventory_GetByIdAndKey(playerid, 44, key);
            if(index == -1) {
                Player_Msg(playerid, COLOR_FADE3, "Tren nguoi ban khong co bat ki chia khoa nao phu hop voi phuong tien.");
            } else {
                Vehicle_SetKeyInserted(vehicleid, true);
                Player_Msg(playerid, COLOR_GREEN, "Ban da dut chia khoa vao phuong tien, co the bat dau su dung.");
                Inventory_RemoveByIndex(playerid, index);
            }
        }
    } else if(!strcmp(params, "takekey", true)) {
        if(!IsPlayerInAnyVehicle(playerid) || GetPlayerVehicleSeat(playerid) != 0) return Player_Msg(playerid, COLOR_FADE3, "Ban can phai ngoi o ghe tai xe cua mot phuong tien.");

        new vehicleid = GetPlayerVehicleID(playerid);
        if(!Vehicle_IsKeyInserted(vehicleid)) return Player_Msg(playerid, COLOR_FADE3, "Phuong tien chua co bat ki chia khoa nao.");

        new key[24];
        Vehicle_GetKey(vehicleid, key, sizeof(key));

        if(!isnull(key)) {
            Inventory_Add(playerid, 44, 1, 1, key);
        }

        Vehicle_SetParams(vehicleid, false, false, Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_ALARM), Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_DOORS), Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_BONNET), Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_BOOT), Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_OBJECTIVE));

        Player_Msg(playerid, COLOR_GREEN, "Ban da lay chia khoa tu phuong tien.");
        Vehicle_SetKeyInserted(vehicleid, false);
    } else if(!strcmp(params, "checkkey", true)) {
        if(!IsPlayerInAnyVehicle(playerid) || GetPlayerVehicleSeat(playerid) != 0) return Player_Msg(playerid, COLOR_FADE3, "Ban can phai ngoi o ghe tai xe cua mot phuong tien.");

        new key[24],
            vehicleid = GetPlayerVehicleID(playerid);
        Vehicle_GetKey(vehicleid, key, sizeof(key));

        if(!isnull(key)) {
            new string[24 + 45];
            format(string, sizeof(string), "Chia khoa cua phuong tien nay la: %s.", key);
            Player_Msg(playerid, COLOR_GREEN, string);
        } else {
            Player_Msg(playerid, COLOR_FADE3, "Phuong tien khong can chia khoa.");
        }

    } else if(!strcmp(params, "engine", true)) {
        if(!IsPlayerInAnyVehicle(playerid) || GetPlayerVehicleSeat(playerid) != 0) return Player_Msg(playerid, COLOR_FADE3, "Ban can phai ngoi o ghe tai xe cua mot phuong tien.");

        new vehicleid = GetPlayerVehicleID(playerid),
            model = GetVehicleModel(vehicleid);
        if(model == 510 || model == 509 || model == 481) return Player_Msg(playerid, COLOR_FADE3, "Xe dap khong phu hop cho lua chon nay.");

        if(!Vehicle_IsKeyInserted(vehicleid)) return Player_Msg(playerid, COLOR_FADE3, "Phuong tien chua duoc dut chia khoa vao.");

        if(Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_ENGINE)) {
            Player_Msg(playerid, COLOR_GREEN, "Phuong tien da tat dong co.");
        } else {
            Player_Msg(playerid, COLOR_GREEN, "Phuong tien da khoi dong.");
        }

        Vehicle_ToggleParam(vehicleid, VEHICLE_PARAMS_ENGINE);
    } else if(!strcmp(params, "lights", true)) {
        if(!IsPlayerInAnyVehicle(playerid) || GetPlayerVehicleSeat(playerid) != 0) return Player_Msg(playerid, COLOR_FADE3, "Ban can phai ngoi o ghe tai xe cua mot phuong tien.");

        new vehicleid = GetPlayerVehicleID(playerid),
            model = GetVehicleModel(vehicleid);
        if(model == 510 || model == 509 || model == 481) return Player_Msg(playerid, COLOR_FADE3, "Xe dap khong phu hop cho lua chon nay.");

        if(!Vehicle_IsKeyInserted(vehicleid)) return Player_Msg(playerid, COLOR_FADE3, "Phuong tien chua duoc dut chia khoa vao.");

        if(Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_LIGHTS)) {
            Player_Msg(playerid, COLOR_GREEN, "Phuong tien da tat den.");
        } else {
            Player_Msg(playerid, COLOR_GREEN, "Phuong tien da mo den.");
        }

        Vehicle_ToggleParam(vehicleid, VEHICLE_PARAMS_LIGHTS);
    } else if(!strcmp(params, "lock", true)) {
        new vehicleid = (IsPlayerInAnyVehicle(playerid)) ? GetPlayerVehicleID(playerid) :Player_GetClosestVehicleId(playerid, 3.0);
        if(vehicleid == 0) return Player_Msg(playerid, COLOR_FADE3, "Ban phai ngoi ben trong hoac o gan mot phuong tien.");

        if(Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_DOORS)) {
            new key[24];
            Vehicle_GetKey(vehicleid, key, sizeof(key));

            if(!isnull(key)) {
                new index = Inventory_GetByIdAndKey(playerid, 44, key);
                if(index == -1) {
                    return Player_Msg(playerid, COLOR_FADE3, "Ban khong co chia khoa cua phuong tien nay.");
                }
            }
        }

        if(Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_DOORS)) {
            Player_Msg(playerid, COLOR_GREEN, "Phuong tien da mo khoa.");
        } else {
            Player_Msg(playerid, COLOR_GREEN, "Phuong tien da dong khoa.");
        }

        Vehicle_ToggleParam(vehicleid, VEHICLE_PARAMS_DOORS);
    } else if(!strcmp(params, "trunk", true)) {
        new vehicleid = Player_GetClosestVehicleId(playerid, 5.0);

        if(vehicleid == 0) {
            return Player_Msg(playerid, COLOR_FADE3, "Ban can phai o gan mot phuong tien.");
        }

        new Float:model_pos[3],
            Float:pos[3];
        GetVehicleModelInfo(GetVehicleModel(vehicleid), VEHICLE_MODEL_INFO_WHEELSREAR, model_pos[0], model_pos[1], model_pos[2]);
        Utils_GetVehicleRelativePos(vehicleid, pos[0], pos[1], pos[2], model_pos[0] - 1.0, model_pos[1] - 0.5, model_pos[2]);

        if(IsPlayerInRangeOfPoint(playerid, 2.0, pos[0], pos[1], pos[2])) {
            if(Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_BOOT)) {
                Vehicle_ToggleParam(vehicleid, VEHICLE_PARAMS_BOOT);

                Player_Msg(playerid, COLOR_GREEN, "Cop cua phuong tien da duoc dong.");
            } else {
                if(!Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_DOORS)) {
                    Vehicle_ToggleParam(vehicleid, VEHICLE_PARAMS_BOOT);

                    Player_Msg(playerid, COLOR_GREEN, "Cop cua phuong tien da duoc mo.");
                } else Player_Msg(playerid, COLOR_FADE3, "Phuong tien da bi khoa, khong the mo cop.");
            }
        } else Player_Msg(playerid, COLOR_FADE3, "Ban khong o gan cop cua phuong tien.");
    } else if(!strcmp(params, "hood", true)) {
        new vehicleid = Player_GetClosestVehicleId(playerid, 5.0);

        if(vehicleid == 0) {
            return Player_Msg(playerid, COLOR_FADE3, "Ban can phai o gan mot phuong tien.");
        }

        new Float:model_pos[3],
            Float:pos[3];
        GetVehicleModelInfo(GetVehicleModel(vehicleid), VEHICLE_MODEL_INFO_WHEELSFRONT, model_pos[0], model_pos[1], model_pos[2]);
        Utils_GetVehicleRelativePos(vehicleid, pos[0], pos[1], pos[2], model_pos[0] - 1.0, model_pos[1] + 0.5, model_pos[2]);

        if(IsPlayerInRangeOfPoint(playerid, 2.0, pos[0], pos[1], pos[2])) {
            if(Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_BONNET)) {
                Vehicle_ToggleParam(vehicleid, VEHICLE_PARAMS_BONNET);

                Player_Msg(playerid, COLOR_GREEN, "Nap cua phuong tien da duoc dong.");
            } else {
                if(!Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_DOORS)) {
                    Vehicle_ToggleParam(vehicleid, VEHICLE_PARAMS_BONNET);

                    Player_Msg(playerid, COLOR_GREEN, "Nap cua phuong tien da duoc mo.");
                } else Player_Msg(playerid, COLOR_FADE3, "Phuong tien da bi khoa, khong the mo nap.");
            }
        } else Player_Msg(playerid, COLOR_FADE3, "Ban khong o gan nap cua phuong tien.");
    } else if(!strcmp(params, "park", true)) {
        if(!IsPlayerInAnyVehicle(playerid) || GetPlayerVehicleSeat(playerid) != 0) return Player_Msg(playerid, COLOR_FADE3, "Ban can phai ngoi tren ghe tai xe cua mot phuong tien.");

        new vehicleid = GetPlayerVehicleID(playerid);

        if(Vehicle_GetType(vehicleid) != VEHICLE_TYPE_PERSONAL) return Player_Msg(playerid, COLOR_FADE3, "Phuong tien nay khong phu hop de dau xe.");

        new key[24];
        Vehicle_GetKey(vehicleid, key, sizeof(key));

        task_yield(1);
        new result = await PersonalVehicle_Park(vehicleid);

        if(result < 0) {
            Player_Msg(playerid, COLOR_RED, sprintf("> ERROR: Da co loi xay ra trong qua trinh dau phuong tien (error code: %d).", result));
        } else {
            //Inventory_Add(playerid, 44, 1, 1, key); // give the key back to player after parking the vehicle.

            Player_Msg(playerid, COLOR_GREEN, "Phuong tien cua ban da duoc dau thanh cong.");
        }
    } else if(!strcmp(params, "buypark", true)) {
        if(!IsPlayerInAnyVehicle(playerid) || GetPlayerVehicleSeat(playerid) != 0) return Player_Msg(playerid, COLOR_FADE3, "Ban can phai ngoi tren ghe tai xe cua mot phuong tien.");

        new vehicleid = GetPlayerVehicleID(playerid);

        if(!Vehicle_IsKeyInserted(vehicleid)) return Player_Msg(playerid, COLOR_FADE3, "Phuong tien chua duoc cam chia khoa.");

        if(Vehicle_GetType(vehicleid) != VEHICLE_TYPE_PERSONAL) return Player_Msg(playerid, COLOR_FADE3, "Phuong tien nay khong phu hop de mua vi tri dau xe.");

        new Float:pos[4],
            uuid[UUID_LEN];
        GetVehiclePos(vehicleid, pos[0], pos[1], pos[2]);
        GetVehicleZAngle(vehicleid, pos[3]);
        PersonalVehicle_GetUUID(vehicleid, uuid, sizeof(uuid));

        task_yield(1);
        if(task_await(PersonalVehicle_SetParkingPos(uuid, pos[0], pos[1], pos[2], pos[3], Vehicle_GetInterior(vehicleid), Vehicle_GetWorld(vehicleid))) > 0) {
            Player_Msg(playerid, COLOR_GREEN, "Ban da mua vi tri dau xe cho phuong tien thanh cong, su dung /v park de dau phuong tien.");
        } else {
            Player_Msg(playerid, COLOR_FADE3, "Da co loi xay ra trong qua trinh mua vi tri dau xe.");
        }
    } else {
        Player_Msg(playerid, COLOR_FADE2, "* choice: get, insertkey, takekey, checkkey, engine, lights, lock, trunk, hood, park, buypark.");
    }
    return 1;
}
alias:vehicle("v", "veh", "phuongtien")

flags:avehicle(CMD_FLAG_ADMIN_1 | CMD_FLAG_ADMIN_2 | CMD_FLAG_ADMIN_3 | CMD_FLAG_ADMIN_4 | CMD_FLAG_ADMIN_5)
CMD:avehicle(playerid, params[]) {
    new model,
    colors[2],
        siren;
    if(sscanf(params, "iI(0)I(1)I(0)", model, colors[0], colors[1], siren)) return Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /avehicle [model] (primary color) (secondary color) (siren)");

    new Float:pos[3],
        Float:angle;
    GetPlayerPos(playerid, pos[0], pos[1], pos[2]);
    GetPlayerFacingAngle(playerid, angle);
    Utils_GetPlayerForwardPos(playerid, pos[0], pos[1], 3.0);

    Vehicle_Create(model, pos[0], pos[1], pos[2], angle, colors[0], colors[1], GetPlayerInterior(playerid), GetPlayerVirtualWorld(playerid), -1, siren);
    return 1;
}
alias:avehicle("aveh")

flags:aweapon(CMD_FLAG_ADMIN_1 | CMD_FLAG_ADMIN_2 | CMD_FLAG_ADMIN_3 | CMD_FLAG_ADMIN_4 | CMD_FLAG_ADMIN_5)
CMD:aweapon(playerid, params[]) {
    new weapon_id,
    ammo;
    if(sscanf(params, "iI(100)", weapon_id, ammo)) return Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /aweapon [weapon id] (ammo)");

    GivePlayerWeapon(playerid, weapon_id, ammo);
    return 1;
}
alias:aweapon("aweap")

flags:agiveitem(CMD_FLAG_ADMIN_3 | CMD_FLAG_ADMIN_4 | CMD_FLAG_ADMIN_5)
CMD:agiveitem(playerid, params[]) {
    new targetid,
    item_id,
    amount,
    number_of_uses,
    key[24];
    if(sscanf(params, "iiI(1)I(1)S()[24]", targetid, item_id, amount, number_of_uses, key)) return Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /agiveitem [playerid] [item id] (amount) (number of uses) (key)");

    if(!Character_IsSpawned(targetid)) return Player_Msg(playerid, -1, "Nguoi choi chua dang nhap vao nhan vat.");

    task_yield(1);
    new result = await Inventory_Add(targetid, item_id, amount, number_of_uses, key);

    if(result == 0) {
        Player_Msg(playerid, COLOR_ERROR, "Cho vat pham: that bai - Item ID khong ton tai trong co so du lieu.");
    } else {
        new string[87 + 33 + 64 + 20 + 24],
            name[MAX_PLAYER_NAME + 1],
            item_name[64];
        Character_GetName(targetid, name, sizeof(name));

        task_yield(1);
        await_arr(item_name) Item_RetrieveName(item_id);

        format(string, sizeof(string), "Cho vat pham: thanh cong ['%s' | Name:'%s' | Amount: %d | Number Of Uses: %d | Key: %s]", name, item_name, amount, number_of_uses, (isnull(key)) ? "empty" :key);
        Player_Msg(playerid, COLOR_GREEN, string);
    }
    return 1;
}

flags:acreatepvehicle(CMD_FLAG_ADMIN_4 | CMD_FLAG_ADMIN_5)
CMD:acreatepvehicle(playerid, params[]) {
    new targetid,
    model,
    primary_color,
    secondary_color,
    siren;
    if(sscanf(params, "iiI(0)I(1)I(0)", targetid, model, primary_color, secondary_color, siren)) return Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /acreatepveh(icle) [playerid] [model] (primary color) (secondary color) (siren)");

    if(!Character_IsSpawned(targetid)) return Player_Msg(playerid, COLOR_FADE3, "Nguoi choi chua dang nhap vao nhan vat.");

    if(model < 400 || model > 611) return Player_Msg(playerid, COLOR_FADE3, "Model phuong tien khong hop le.");

    new target_uuid[UUID_LEN];
    Character_GetUUID(targetid, target_uuid, sizeof(target_uuid));

    task_yield(1);
    new count = await PersonalVehicle_CountByOwner(target_uuid);

    if(count >= MAX_CHARACTER_PERSONAL_VEHICLES) return Player_Msg(playerid, COLOR_FADE3, "Nguoi choi da so huu toi da phuong tien.");

    new Float:pos[3],
        Float:angle;
    GetPlayerPos(targetid, pos[0], pos[1], pos[2]);
    Utils_GetPlayerForwardPos(targetid, pos[0], pos[1], 3.5);
    GetPlayerFacingAngle(targetid, angle);

    task_yield(1);
    new index = await PersonalVehicle_Create(target_uuid, model, pos[0], pos[1], pos[2], angle, primary_color, secondary_color, GetPlayerInterior(targetid), GetPlayerVirtualWorld(targetid), -1, siren);
    if(index != -1) {
        new vehicle_name[17],
            char_name[MAX_PLAYER_NAME + 1];

        Character_GetName(playerid, char_name, sizeof(char_name));
        Vehicle_GetNameByModel(model, vehicle_name, sizeof(vehicle_name));
        Player_Msg(playerid, COLOR_GREEN, sprintf("Ban da tao phuong tien '%s' cho '%s'.", vehicle_name, char_name));
    } else {
        Player_Msg(playerid, COLOR_FADE3, "Da co loi xay ra trong luc tao phuong tien, vui long bao loi.");
    }
    return 1;
}
alias:acreatepvehicle("acreatepveh")

flags:aportal(CMD_FLAG_ADMIN_3 | CMD_FLAG_ADMIN_4 | CMD_FLAG_ADMIN_5)
CMD:aportal(playerid, params[]) {
    new choice[16],
        portal_index;
    if(sscanf(params, "s[16]I(-1)", choice, portal_index)) {
        Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /aportal [choice] (portal index)");
        Player_Msg(playerid, COLOR_FADE2, "* choice: create, entrance, exit, togglelock, destroy");
        return 1;
    }

    if(!strcmp(choice, "create", true)) {
        #pragma unused portal_index

        Player_Msg(playerid, COLOR_GREEN, sprintf("Ban da tao ra portal index %d, hay su dung portal index cho lua chon khac nhu 'entrance', 'exit',...", Portal_Create()));
    } else if(!strcmp(choice, "entrance", true)) {
        if(portal_index < 0 || portal_index >= Portal_GetSize()) {
            Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /aportal entrance (portal index)");
            Player_Msg(playerid, COLOR_FADE2, "* portal index khong hop le.");
            return 1;
        }

        new Float:pos[3],
            Float:angle;

        GetPlayerPos(playerid, pos[0], pos[1], pos[2]);
        GetPlayerFacingAngle(playerid, angle);
        Portal_SetEntrance(portal_index, pos[0], pos[1], pos[2], angle, GetPlayerInterior(playerid), GetPlayerVirtualWorld(playerid));

        Player_Msg(playerid, COLOR_GREEN, sprintf("Ban da dieu chinh loi vao cua portal %d ve vi tri hien tai cua ban.", portal_index));
    } else if(!strcmp(choice, "exit", true)) {
        if(portal_index < 0 || portal_index >= Portal_GetSize()) {
            Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /aportal exit (portal index)");
            Player_Msg(playerid, COLOR_FADE2, "* portal index khong hop le.");
            return 1;
        }

        new Float:pos[3],
            Float:angle;

        GetPlayerPos(playerid, pos[0], pos[1], pos[2]);
        GetPlayerFacingAngle(playerid, angle);
        Portal_SetExit(portal_index, pos[0], pos[1], pos[2], angle, GetPlayerInterior(playerid), GetPlayerVirtualWorld(playerid));

        Player_Msg(playerid, COLOR_GREEN, sprintf("Ban da dieu chinh loi ra cua portal %d ve vi tri hien tai cua ban.", portal_index));
    } else if(!strcmp(choice, "togglelock", true)) {
        if(portal_index < 0 || portal_index >= Portal_GetSize()) {
            Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /aportal exit (portal index)");
            Player_Msg(playerid, COLOR_FADE2, "* portal index khong hop le.");
            return 1;
        }

        if(Portal_GetLock(portal_index)) {
            Player_Msg(playerid, COLOR_GREEN, sprintf("Ban da mo khoa portal index %d.", portal_index));
        } else {
            Player_Msg(playerid, COLOR_GREEN, sprintf("Ban da khoa portal index %d.", portal_index));
        }
        Portal_ToggleLock(portal_index);
    } else if(!strcmp(choice, "destroy", true)) {
        if(portal_index < 0 || portal_index >= Portal_GetSize()) {
            Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /aportal exit (portal index)");
            Player_Msg(playerid, COLOR_FADE2, "* portal index khong hop le.");
            return 1;
        }

        if(Portal_DestroyByIndex(portal_index) == 1) {
            Player_Msg(playerid, COLOR_GREEN, sprintf("Ban da xoa portal index %d thanh cong.", portal_index));
        } else {
            Player_Msg(playerid, COLOR_RED, sprintf("> ERROR: Xoa portal index %d that bai.", portal_index));
        }
    } else {
        Player_Msg(playerid, COLOR_FADE2, "* choice: create, entrance, exit, togglelock, destroy");
    }
    return 1;
}
alias:aportal("apotl")

flags:ahouse(CMD_FLAG_ADMIN_3 | CMD_FLAG_ADMIN_4 | CMD_FLAG_ADMIN_5)
CMD:ahouse(playerid, params[]) {
    new choice[16],
        index,
        amount;
    if(sscanf(params, "s[16]I(-1)I(-1)", choice, index, amount)) {
        Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /ahouse [choice] (index) (amount)");
        Player_Msg(playerid, COLOR_FADE2, "* choice: create, destroy, price, roomprice, maxroom, checkkey");
        return 1;
    }

    if(!strcmp(choice, "create", true)) {
        Player_Msg(playerid, COLOR_GREEN, sprintf("Tao can nha: thanh cong [ID: %d]", House_Create()));
    } else if(!strcmp(choice, "destroy", true)) {
        if(House_DestroyByIndex(index)) {
            Player_Msg(playerid, COLOR_GREEN, sprintf("Ban da xoa thanh cong house %d.", index));
        } else {
            Player_Msg(playerid, COLOR_FADE3, "Xoa that bai (co the id cua can nha khong ton tai).");
        }
    } else if(!strcmp(choice, "price", true)) {
        if(!House_SetPrice(index, amount)) {
            Player_Msg(playerid, COLOR_FADE3, "House Index ban vua nhap khong ton tai.");
        } else {
            if(amount < 0) {
                return Player_Msg(playerid, COLOR_FADE3, "Gia tien khong hop le.");
            }
            Player_Msg(playerid, COLOR_GREEN, sprintf("Ban da dieu chinh gia tien cua can nha %d ve %d$.", index, amount));
        }
    } else if(!strcmp(choice, "roomprice", true)) {
        if(!House_SetRoomPrice(index, amount)) {
            new string[28 + 10];
            format(string, sizeof(string), "Can nha ID %d khong ton tai.", index);
            Player_Msg(playerid, COLOR_FADE3, string);
        } else {
            if(amount < 0) {
                Player_Msg(playerid, COLOR_FADE3, "Gia tien khong hop le.");
            } else {
                Player_Msg(playerid, COLOR_GREEN, sprintf("Ban da dieu chinh gia tien thue phong cua can nha %d ve %d$.", index, amount));
            }
        }
    } else if(!strcmp(choice, "maxroom", true)) {
        if(!House_SetMaxRoom(index, amount)) {
            new string[28 + 10];
            format(string, sizeof(string), "Can nha ID %d khong ton tai.", index);
            Player_Msg(playerid, COLOR_FADE3, string);
        } else {
            if(amount < 0) {
                Player_Msg(playerid, COLOR_FADE3, "So phong khong hop le.");
            } else {
                Player_Msg(playerid, COLOR_GREEN, sprintf("Ban da dieu chinh so luong phong cua can nha %d ve %d.", index, amount));
            }
        }
    } else if(!strcmp(choice, "checkkey", true)) {
        new key[24];
        if(!House_GetKey(index, key, sizeof(key))) {
            new string[28 + 10];
            format(string, sizeof(string), "Can nha ID %d khong ton tai.", index);
            Player_Msg(playerid, COLOR_FADE3, string);
        } else {
            new string[35 + 10 + 24];
            format(string, sizeof(string), "Chia khoa cua can nha ID %d la: %s.", index, key);

            Player_Msg(playerid, COLOR_GREEN, string);
        }
    } else {
        Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /ahouse [choice] (index) (amount)");
        Player_Msg(playerid, COLOR_FADE2, "* choice: create, destroy, price, roomprice, maxroom, checkkey");
        return 1;
    }
    return 1;
}

flags:ahportal(CMD_FLAG_ADMIN_3 | CMD_FLAG_ADMIN_4 | CMD_FLAG_ADMIN_5)
CMD:ahportal(playerid, params[]) {
    new choice[17],
        amount;
    if(sscanf(params, "s[17]i", choice, amount)) {
        Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /ahportal [choice] [amount]");
        Player_Msg(playerid, COLOR_FADE2, "* choice: create, destroy, destroybypotlidx");
        return 1;
    }

    if(!strcmp(choice, "create", true)) {
        new uuid[UUID_LEN];
        if(House_GetUUID(amount, uuid, sizeof(uuid))) {
            new result = await HousePortal_Create(uuid);

            if(result == -1) {
                Player_Msg(playerid, COLOR_FADE3, "Tao House Portal: that bai [2].");
            } else {
                new string[41 + 10];
                format(string, sizeof(string), "Tao House Portal: thanh cong [ID: %d].", result);
                Player_Msg(playerid, COLOR_GREEN, string);
            }
        } else {
            Player_Msg(playerid, COLOR_FADE3, "Tao House Portal: that bai [1].");
        }
    } else if(!strcmp(choice, "destroy", true)) {
        if(HousePortal_Destroy(amount)) {
            new string[32 + 10];
            format(string, sizeof(string), "Xoa House Portal %d: thanh cong.", amount);
            Player_Msg(playerid, COLOR_GREEN, string);
        } else {
            new string[30 + 10];
            format(string, sizeof(string), "Xoa House Portal %d: that bai.", amount);
            Player_Msg(playerid, COLOR_FADE3, string);
        }
    } else if(!strcmp(choice, "destroybypotlidx", true)) {
        if(Portal_DestroyByIndex(amount)) {
            new string[47 + 10];
            format(string, sizeof(string), "Xoa House Portal co Portal ID %d: thanh cong.", amount);
            Player_Msg(playerid, COLOR_GREEN, string);
        } else {
            new string[46 + 10];
            format(string, sizeof(string), "Xoa House Portal co Portal ID %d: that bai.", amount);
            Player_Msg(playerid, COLOR_FADE3, string);
        }
    } else {
        Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /ahportal [choice] [amount]");
        Player_Msg(playerid, COLOR_FADE2, "* choice: create, destroy, destroybypotlidx");
    }
    return 1;
}
alias:ahportal("ahpotl")

flags:afurniture(CMD_FLAG_ADMIN_3 | CMD_FLAG_ADMIN_4 | CMD_FLAG_ADMIN_5)
CMD:afurniture(playerid, params[]) {
    new choice[7],
        house_index,
        amount;
    if(sscanf(params, "s[7]iI(0)", choice, house_index, amount)) {
        Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /afurniture [choice] [house index] (amount)");
        Player_Msg(playerid, COLOR_FADE3, "* choice: create, remove");
        return 1;
    }

    if(!strcmp(choice, "create", true)) {
        new Float:pos[3];
        GetPlayerPos(playerid, pos[0], pos[1], pos[2]);

        new index = -1;
        if((index = Furniture_Add(house_index, amount, pos[0], pos[1], pos[2], 0.0, 0.0, 0.0, GetPlayerVirtualWorld(playerid), GetPlayerInterior(playerid))) == -1) {
            new string[40 + 10];
            format(string, sizeof(string), "Tao noi that cho can nha ID %d: that bai.", house_index);
            Player_Msg(playerid, COLOR_FADE3, string);
        } else {
            new string[64 + 20];
            format(string, sizeof(string), "Tao noi that cho can nha ID %d: thanh cong [Furniture ID: %d].", house_index, index);
            Player_Msg(playerid, COLOR_GREEN, string);
        }
    } else if(!strcmp(choice, "remove", true)) {
        if(Furniture_Remove(house_index, amount)) {
            new string[49 + 20];
            format(string, sizeof(string), "Xoa noi that ID %d cua can nha ID %d: thanh cong.", amount, house_index);
            Player_Msg(playerid, COLOR_GREEN, string);
        } else {
            new string[47 + 20];
            format(string, sizeof(string), "Xoa noi that ID %d cua can nha ID %d: that bai.", amount, house_index);
            Player_Msg(playerid, COLOR_FADE3, string);
        }
    } else {
        Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /afurniture [choice] (amount)");
        Player_Msg(playerid, COLOR_FADE3, "* choice: create, remove");
        return 1;
    }
    return 1;
}
alias:afurniture("afurn")

CMD:noithat(playerid, params[]) {
    new house_index;
    if(sscanf(params, "I(-1)", house_index)) {
        return Player_Msg(playerid, COLOR_YELLOW, "> Syntax: /noithat (house index)");
    }

    if(house_index == -1) {
        new uuid[UUID_LEN];
        Character_GetUUID(playerid, uuid, sizeof(uuid));

        if((house_index = House_GetByOwnerUUID(uuid)) == -1) {
            Player_Msg(playerid, COLOR_FADE3, "Ban khong so huu bat ki can nha nao.");
        } else {
            if(!Furniture_ShowPanel(house_index, playerid)) {
                Player_Msg(playerid, COLOR_FADE3, "Can nha cua ban chua co noi that.");
            }
        }
    } else {
        new owner_uuid[UUID_LEN];
        House_GetOwnerUUID(house_index, owner_uuid, sizeof(owner_uuid));
        if(!isnull(owner_uuid)) {
            new uuid[UUID_LEN];
            Character_GetUUID(playerid, uuid, sizeof(uuid));
            if(!strcmp(owner_uuid, uuid)) {
                if(!Furniture_ShowPanel(house_index, playerid)) {
                    Player_Msg(playerid, COLOR_FADE3, "Can nha cua ban chua co noi that.");
                }
            } else {
                Player_Msg(playerid, COLOR_FADE3, "Can nha khong thuoc so huu cua ban.");
            }
        } else {
            Player_Msg(playerid, COLOR_FADE3, "Can nha khong ton tai.");
        }
    }
    return 1;
}
alias:noithat("furniture")

#include <YSI_Coding\y_hooks>
hook OnGameModeInit() {
    print("INFO: ./modules/player-chat loaded");
    return 1;
}

hook OnDialogResponse(playerid, dialogid, response, listitem, inputtext[]) {
    switch (dialogid) {
        case DIALOG_CHAR_PERSONAL_VEHICLE:{
            if(response) {
                new var_name[17];
                format(var_name, sizeof(var_name), "veh_UUID_%d", listitem);

                if(GetPVarType(playerid, var_name) == PLAYER_VARTYPE_STRING) {
                    new uuid[UUID_LEN];
                    GetPVarString(playerid, var_name, uuid, sizeof(uuid));

                    if(PersonalVehicle_GetByUUID(uuid) != 0) {
                        Player_Msg(playerid, COLOR_FADE3, "Phuong tien da duoc lay ra truoc do.");
                    } else {
                        task_yield(1);
                        new result = await PersonalVehicle_Spawn(uuid);
                        if(result == -1) {
                            Player_Msg(playerid, COLOR_FADE3, "Da co loi xay ra trong luc lay phuong tien, vui long lay lai hoac bao loi.");
                        } else {
                            Player_Msg(playerid, COLOR_GREEN, "Phuong tien da xuat hien tai vi tri dau xe.");
                        }
                    }
                } else {
                    Player_Msg(playerid, COLOR_FADE3, "Da co loi xay ra, vui long [/v get] lai.");
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

hook OnPlayerText(playerid, text[]) {
    if(strlen(text) > 0) {
        new string[128 + 33 + 8],
            name[MAX_PLAYER_NAME + 1];
        Character_GetName(playerid, name, sizeof(name));
        format(string, sizeof(string), "%s: %s", name, text);
        Player_Broadcast(playerid, 20.0, COLOR_FADE1, COLOR_FADE9, string);
    }
    return 0;
}
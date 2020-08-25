#if defined _module_player_included
    #endinput
#endif

#define _module_player_included

#include <YSI_Data\y_iterate>
#include <PawnPlus>
#include <zmessage>
#include <uuid>
#include <streamer>

#include <character>
#include <vehicle>

static Iterator:Players<MAX_PLAYERS>;

stock Player_Msg(const playerid, const color = -1, const message[])
{
    ZMsg_SendClientMessage(playerid, color, message);
    return 1;
}

stock Player_Kick(const playerid, const color = -1, const message[] = "")
{
    if(isnull(message))
    {
        Kick(playerid);
    }
    else
    {
        Player_Msg(playerid, color, message);
        await task_ticks(10);
        Kick(playerid);
    }
    return 1;
}

stock Player_Count()
{
    return Iter_Count(Players);
}

stock Player_GetFreeAttachmentIndex(const playerid)
{
    for(new i = 0; i < MAX_PLAYER_ATTACHED_OBJECTS; i++)
    {
        if(!IsPlayerAttachedObjectSlotUsed(playerid, i))
        {
            return i;
        }
    }
    return -1;
}

stock Player_SetName(const playerid, const name[])
{
    return SetPlayerName(playerid, name);
}

stock Player_GetByCharacterUUID(const char_uuid[])
{
    new uuid[UUID_LEN];
    foreach(new i : Players)
    {
        if(Character_IsSpawned(i))
        {
            Character_GetUUID(i, uuid, UUID_LEN);
            if(!strcmp(char_uuid, uuid))
            {
                return i;
            }
        }
    }
    return -1;
}

stock Player_GetClosestVehicleId(const playerid, const Float:range)
{
    new Float:max_dist = range + 0.01,
        Float:dist,
        vehicle_id = 0,
        Float:pos[3];

    GetPlayerPos(playerid, pos[0], pos[1], pos[2]);

    foreach(new i : Vehicle)
    {
        if((dist = GetVehicleDistanceFromPoint(i, pos[0], pos[1], pos[2])) < max_dist)
        {
            max_dist = dist;
            vehicle_id = i;
        }
    }
    return vehicle_id;
}

stock Player_StreamToPosition(const playerid, const Float:x, const Float:y, const Float:z, const Float:angle, const interior, const world, const freezeTime = 0)
{
    if(freezeTime > 0)
    {
        TogglePlayerControllable(playerid, false);
        SetPreciseTimer("Player_OnPositionStreamed", 1000, false, "i", playerid);
    }

    Streamer_UpdateEx(playerid, x, y, z, world, interior, STREAMER_TYPE_OBJECT);

    SetPlayerPos(playerid, x, y, z);
    SetPlayerFacingAngle(playerid, angle);
    SetPlayerInterior(playerid, interior);
    SetPlayerVirtualWorld(playerid, world);

    SetCameraBehindPlayer(playerid);
    return 1;
}

stock Player_Broadcast(const playerid, const Float:range, const from_color, const to_color, const string[])
{
    new Float:player_pos[3],
        Float:dist,
        new_color,
        Float:ratio,
        from_r,
        from_g,
        from_b,
        to_r,
        to_g,
        to_b,
        unused;

    Utils_RGBA(from_color, from_r, from_g, from_b, unused);
    Utils_RGBA(to_color, to_r, to_g, to_b, unused);

    GetPlayerPos(playerid, player_pos[0], player_pos[1], player_pos[2]);
    foreach(new i : Players)
    {
        if(Character_IsSpawned(i))
        {
            dist = GetPlayerDistanceFromPoint(i, player_pos[0], player_pos[1], player_pos[2]);
            if(dist <= range)
            {
                ratio = dist / range;
                Utils_Hex(new_color, from_r + floatround((to_r - from_r) * ratio), from_g + floatround((to_g - from_g) * ratio), from_b + floatround((to_b - from_b) * ratio), 255);
                Player_Msg(i, new_color, string);
            }
        }
    }
    return 1;
}

forward Player_OnPositionStreamed(const playerid);
public Player_OnPositionStreamed(const playerid)
{
    TogglePlayerControllable(playerid, true);
    return 1;
}

#include <YSI_Coding\y_hooks>

hook OnGameModeInit()
{
    Iter_Clear(Players);
    print("INFO: ./modules/player loaded");
    return 1;
}

hook OnPlayerConnect(playerid)
{
    Iter_Add(Players, playerid);
    return 1;
}

hook OnPlayerDisconnect(playerid, reason)
{
    Iter_Remove(Players, playerid);
    return 1;
}

hook OnPlayerWeaponShot(playerid, weaponid, hittype, hitid, Float:fX, Float:fY, Float:fZ)
{
    if((weaponid < 22 || weaponid > 34) && weaponid != 38)
    {
        // invalid bullet weapon
        return 0; // desync the shot
    }
    return 1;
}

hook OnPlayerExitVehicle(playerid, vehicleid)
{
    if(Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_DOORS))
    {
        new Float:pos[3];
        GetPlayerPos(playerid, pos[0], pos[1], pos[2]);
        SetPlayerPos(playerid, pos[0], pos[1], pos[2] + 2.0);
        PutPlayerInVehicle(playerid, vehicleid, GetPlayerVehicleSeat(playerid));
        return 0;
    }
    return 1;
}
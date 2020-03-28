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

stock Player_StreamToPosition(const playerid, const Float:x, const Float:y, const Float:z, const Float:angle, const interior, const world)
{
    TogglePlayerControllable(playerid, false);

    Streamer_UpdateEx(playerid, x, y, z, world, interior, STREAMER_TYPE_OBJECT);

    SetPlayerPos(playerid, x, y, z);
    SetPlayerFacingAngle(playerid, angle);
    SetPlayerInterior(playerid, interior);
    SetPlayerVirtualWorld(playerid, world);

    SetPreciseTimer("Player_OnPositionStreamed", 1000, false, "i", playerid);
    return 1;
}

forward Player_OnPositionStreamed(const playerid);
public Player_OnPositionStreamed(const playerid)
{
    TogglePlayerControllable(playerid, true);
    SetCameraBehindPlayer(playerid);
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
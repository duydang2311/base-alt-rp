#if defined _included_portal
    #endinput
#endif

#define _included_portal

#include <streamer>

static const PORTAL_ENTRANCE_EXTRA_ID = 'p' + 'o' + 'r' + 't' + 'a' + 'l' + 'e' + 'n' + 't' + 'r' + 'a' + 'n' + 'c' + 'e';
static const PORTAL_EXIT_EXTRA_ID = 'p' + 'o' + 'r' + 't' + 'a' + 'l' + 'e' + 'x' + 'i' + 't';

enum _:e_PORTAL_TYPE
{
    PORTAL_TYPE_NONE,
    PORTAL_TYPE_HOUSE,
    PORTAL_TYPE_BUSINESS,
}

static enum _:E_PORTAL_DATA
{
    potl_UUID[UUID_LEN],
    potl_Type,
    Float:potl_Entrance_Pos[3],
    Float:potl_Entrance_Angle,
    potl_Entrance_Interior,
    potl_Entrance_World,
    potl_Entrance_Pickup,
    potl_Entrance_Area,
    Float:potl_Exit_Pos[3],
    Float:potl_Exit_Angle,
    potl_Exit_Interior,
    potl_Exit_World,
    potl_Exit_Pickup,
    potl_Exit_Area,
    bool:potl_Locked
}

static List:portals;

forward bool:HousePortal_DestroyByPortalIdx(const index);

stock Portal_Create()
{
    new portal[E_PORTAL_DATA];

    UUID(portal[potl_UUID], UUID_LEN);
    portal[potl_Type] = PORTAL_TYPE_NONE;
    portal[potl_Entrance_Pos][0] = 0.0000;
    portal[potl_Entrance_Pos][1] = 0.0000;
    portal[potl_Entrance_Pos][2] = 0.0000;
    portal[potl_Entrance_Angle] = 0.0000;
    portal[potl_Entrance_Interior] = 0;
    portal[potl_Entrance_World] = 0;
    portal[potl_Entrance_Pickup] = 0;
    portal[potl_Entrance_Area] = 0;
    portal[potl_Exit_Pos][0] = 0.0000;
    portal[potl_Exit_Pos][1] = 0.0000;
    portal[potl_Exit_Pos][2] = 0.0000;
    portal[potl_Exit_Angle] = 0.0000;
    portal[potl_Exit_Interior] = 0;
    portal[potl_Exit_World] = 0;
    portal[potl_Exit_Pickup] = 0;
    portal[potl_Exit_Area] = 0;
    portal[potl_Locked] = false;

    new query[40 + 37 + 10];
    mysql_format(MySQL_GetHandle(), query, sizeof(query), "INSERT INTO portals (uuid) VALUES ('%e')", portal[potl_UUID]);
    mysql_tquery(MySQL_GetHandle(), query);

    return list_add_arr(portals, portal);
}

stock Portal_SetEntrance(const index, const Float:x, const Float:y, const Float:z, const Float:angle, const interior, const world)
{
    if(index < list_size(portals))
    {
        new portal[E_PORTAL_DATA];
        if(list_get_arr_safe(portals, index, portal))
        {
            if(portal[potl_Entrance_Pickup] != 0)
            {
                DestroyDynamicPickup(portal[potl_Entrance_Pickup]);
            }
            
            if(portal[potl_Entrance_Area] != 0)
            {
                DestroyDynamicArea(portal[potl_Entrance_Area]);
            }

            new area = CreateDynamicSphere(x, y, z, 2.0, world, interior);

            Streamer_SetIntData(STREAMER_TYPE_AREA, area, E_STREAMER_EXTRA_ID, PORTAL_ENTRANCE_EXTRA_ID);
            
            list_set_cell(portals, index, potl_Entrance_Pos, x);
            list_set_cell(portals, index, potl_Entrance_Pos + 1, y);
            list_set_cell(portals, index, potl_Entrance_Pos + 2, z);
            list_set_cell(portals, index, potl_Entrance_Angle, angle);
            list_set_cell(portals, index, potl_Entrance_Interior, interior);
            list_set_cell(portals, index, potl_Entrance_World, world);
            list_set_cell(portals, index, potl_Entrance_Pickup, CreateDynamicPickup(1318, 1, x, y, z, world, interior, _, 10.0));
            list_set_cell(portals, index, potl_Entrance_Area, area);

            new query[180 + 37 + 100];
            mysql_format(MySQL_GetHandle(), query, sizeof(query), "UPDATE portals SET entrance_pos_x = %.4f, entrance_pos_y = %.4f, entrance_pos_z = %.4f, entrance_angle = %.4f, entrance_interior = %d, entrance_world = %d WHERE uuid = '%e' LIMIT 1", x, y, z, angle, interior, world, portal[potl_UUID]);
            mysql_tquery(MySQL_GetHandle(), query);
        }  
    }
    return 1;
}

stock Portal_SetExit(const index, const Float:x, const Float:y, const Float:z, const Float:angle, const interior, const world)
{
    if(index < list_size(portals))
    {
        new portal[E_PORTAL_DATA];
        if(list_get_arr_safe(portals, index, portal))
        {
            if(portal[potl_Exit_Pickup] != 0)
            {
                DestroyDynamicPickup(portal[potl_Exit_Pickup]);
            }
            
            if(portal[potl_Exit_Area] != 0)
            {
                DestroyDynamicArea(portal[potl_Exit_Area]);
            }
            
            new area = CreateDynamicSphere(x, y, z, 2.0, world, interior);

            Streamer_SetIntData(STREAMER_TYPE_AREA, area, E_STREAMER_EXTRA_ID, PORTAL_EXIT_EXTRA_ID);

            list_set_cell(portals, index, potl_Exit_Pos, x);
            list_set_cell(portals, index, potl_Exit_Pos + 1, y);
            list_set_cell(portals, index, potl_Exit_Pos + 2, z);
            list_set_cell(portals, index, potl_Exit_Angle, angle);
            list_set_cell(portals, index, potl_Exit_Interior, interior);
            list_set_cell(portals, index, potl_Exit_World, world);
            list_set_cell(portals, index, potl_Exit_Pickup, CreateDynamicPickup(1318, 1, x, y, z, world, interior, _, 10.0));
            list_set_cell(portals, index, potl_Exit_Area, area);

            new query[180 + 37 + 100];
            mysql_format(MySQL_GetHandle(), query, sizeof(query), "UPDATE portals SET exit_pos_x = %.4f, exit_pos_y = %.4f, exit_pos_z = %.4f, exit_angle = %.4f, exit_interior = %d, exit_world = %d WHERE uuid = '%e' LIMIT 1", x, y, z, angle, interior, world, portal[potl_UUID]);
            mysql_tquery(MySQL_GetHandle(), query);
        }
    }
    return 1;
}

stock Portal_DestroyByIndex(const index)
{
    if(index < 0 || index >= list_size(portals))
    {
        return 0;
    }

    new portal[E_PORTAL_DATA];
    if(list_get_arr_safe(portals, index, portal))
    {
        if(portal[potl_Type] == PORTAL_TYPE_HOUSE)
        {
            HousePortal_DestroyByPortalIdx(index);
        }
        else if(portal[potl_Type] == PORTAL_TYPE_BUSINESS)
        {
            // todo: BusinessPortal_Destroy(index);
        }
        
        if(portal[potl_Entrance_Pickup] != 0)
        {
            DestroyDynamicPickup(portal[potl_Entrance_Pickup]);
        }
        
        if(portal[potl_Entrance_Area] != 0)
        {
            DestroyDynamicArea(portal[potl_Entrance_Area]);
        }

        if(portal[potl_Exit_Pickup] != 0)
        {
            DestroyDynamicPickup(portal[potl_Exit_Pickup]);
        }
        
        if(portal[potl_Exit_Area] != 0)
        {
            DestroyDynamicArea(portal[potl_Exit_Area]);
        }

        new query[45 + 37 + 10];
        mysql_format(MySQL_GetHandle(), query, sizeof(query), "DELETE FROM portals WHERE uuid = '%e' LIMIT 1", portal[potl_UUID]);
        mysql_tquery(MySQL_GetHandle(), query);

        list_remove_deep(portals, index);
        return 1;
    }
    return 0;
}

stock Portal_GetUUID(const index, uuid[], const size = UUID_LEN)
{
    if(index < list_size(portals))
    {
        new portal[E_PORTAL_DATA];
        if(list_get_arr_safe(portals, index, portal))
        {
            uuid[0] = EOS;
            strcat(uuid, portal[potl_UUID], size);
        }
    }
    return 1;
}

stock bool:Portal_GetLock(const index)
{
    new portal[E_PORTAL_DATA];
    if(list_get_arr_safe(portals, index, portal))
    {
        return portal[potl_Locked];
    }
    return false;
}

stock Portal_ToggleLock(const index)
{
    new portal[E_PORTAL_DATA];
    if(list_get_arr_safe(portals, index, portal))
    {
        list_set_cell(portals, index, potl_Locked, (portal[potl_Locked]) ? 0 : 1);

        new query[56 + 37 + 10];
        mysql_format(MySQL_GetHandle(), query, sizeof(query), "UPDATE portals SET locked = %d WHERE uuid = '%e' LIMIT 1", (portal[potl_Locked]) ? 0 : 1, portal[potl_UUID]);
        mysql_tquery(MySQL_GetHandle(), query);
        return 1;
    }
    return 0;
}

stock Portal_GetSize()
{
    return list_size(portals);
}

stock Portal_GetEntranceExtraId()
{
    return PORTAL_ENTRANCE_EXTRA_ID;
}

stock Portal_GetExitExtraId()
{
    return PORTAL_EXIT_EXTRA_ID;
}

stock Portal_GetByEntranceAreaId(const areaid)
{
    new portal[E_PORTAL_DATA];
    for(new Iter:i = list_iter(portals); iter_inside(i); iter_move_next(i))
    {
        if(iter_get_arr_safe(i, portal))
        {
            if(portal[potl_Entrance_Area] == areaid)
            {
                return iter_get_key(i);
            }
        }
    }
    return -1;
}

stock Portal_GetByExitAreaId(const areaid)
{
    new portal[E_PORTAL_DATA];
    for(new Iter:i = list_iter(portals); iter_inside(i); iter_move_next(i))
    {
        if(iter_get_arr_safe(i, portal))
        {
            if(portal[potl_Exit_Area] == areaid)
            {
                return iter_get_key(i);
            }
        }
    }
    return -1;
}

stock Portal_StreamPlayerToEntrance(const playerid, const portal_index)
{
    if(portal_index < 0 || portal_index >= list_size(portals))
    {
        return 0;
    }

    new portal[E_PORTAL_DATA];
    if(list_get_arr_safe(portals, portal_index, portal))
    {
        if(portal[potl_Entrance_Area] == 0)
        {
            return -1;
        }
        
        if(portal[potl_Locked])
        {
            return -2;
        }

        Player_StreamToPosition(playerid, portal[potl_Entrance_Pos][0], portal[potl_Entrance_Pos][1], portal[potl_Entrance_Pos][2], portal[potl_Entrance_Angle], portal[potl_Entrance_Interior], portal[potl_Entrance_World]);
        return 1;
    }
    return 0;
}

stock Portal_StreamPlayerToExit(const playerid, const portal_index)
{
    if(portal_index < 0 || portal_index >= list_size(portals))
    {
        return 0;
    }

    new portal[E_PORTAL_DATA];
    if(list_get_arr_safe(portals, portal_index, portal))
    {
        if(portal[potl_Exit_Area] == 0)
        {
            return -1;
        }
        
        if(portal[potl_Locked])
        {
            return -2;
        }
        Player_StreamToPosition(playerid, portal[potl_Exit_Pos][0], portal[potl_Exit_Pos][1], portal[potl_Exit_Pos][2], portal[potl_Exit_Angle], portal[potl_Exit_Interior], portal[potl_Exit_World]);
        return 1;
    }
    return 0;
}

stock bool:Portal_SetType(const portal_index, const type)
{
    if(portal_index < 0 || portal_index >= list_size(portals))
    {
        return false;
    }

    list_set_cell(portals, portal_index, potl_Type, type);
    return true;
}

#include "types\house_portal.inc"
#include "types\business_portal.inc"

#include <YSI_Coding\y_hooks>

hook OnGameModeInit()
{
    mysql_query(MySQL_GetHandle(), "CREATE TABLE IF NOT EXISTS portals (\
        uuid VARCHAR(37) NOT NULL PRIMARY KEY DEFAULT '',\
        type TINYINT UNSIGNED NOT NULL DEFAULT 0,\
        entrance_pos_x FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        entrance_pos_y FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        entrance_pos_z FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        entrance_angle FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        entrance_interior INT NOT NULL DEFAULT 0,\
        entrance_world INT NOT NULL DEFAULT 0,\
        exit_pos_x FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        exit_pos_y FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        exit_pos_z FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        exit_angle FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        exit_interior INT NOT NULL DEFAULT 0,\
        exit_world INT NOT NULL DEFAULT 0,\
        locked BOOLEAN NOT NULL DEFAULT 0\
    );", false);

    portals = list_new();

    print("> Retrieving and initiating portals...");
    
    mysql_query(MySQL_GetHandle(), "SELECT * FROM portals", true);

    new rows = cache_num_rows();
    if(rows > 0)
    {
        new portal[E_PORTAL_DATA],
            index;
        for(new i = 0; i != rows; i++)
        {
            cache_get_value_name(i, "uuid", portal[potl_UUID], UUID_LEN);

            cache_get_value_name_int(i, "type", portal[potl_Type]);
            cache_get_value_name_int(i, "locked", portal[potl_Locked]);
            
            cache_get_value_name_float(i, "entrance_pos_x", portal[potl_Entrance_Pos][0]);
            cache_get_value_name_float(i, "entrance_pos_y", portal[potl_Entrance_Pos][1]);
            cache_get_value_name_float(i, "entrance_pos_z", portal[potl_Entrance_Pos][2]);
            cache_get_value_name_float(i, "entrance_angle", portal[potl_Entrance_Angle]);
            cache_get_value_name_int(i, "entrance_interior", portal[potl_Entrance_Interior]);
            cache_get_value_name_int(i, "entrance_world", portal[potl_Entrance_World]);

            cache_get_value_name_float(i, "exit_pos_x", portal[potl_Exit_Pos][0]);
            cache_get_value_name_float(i, "exit_pos_y", portal[potl_Exit_Pos][1]);
            cache_get_value_name_float(i, "exit_pos_z", portal[potl_Exit_Pos][2]);
            cache_get_value_name_float(i, "exit_angle", portal[potl_Exit_Angle]);
            cache_get_value_name_int(i, "exit_interior", portal[potl_Exit_Interior]);
            cache_get_value_name_int(i, "exit_world", portal[potl_Exit_World]);

            index = list_add_arr(portals, portal);

            Portal_SetEntrance(index, portal[potl_Entrance_Pos][0], portal[potl_Entrance_Pos][1], portal[potl_Entrance_Pos][2], portal[potl_Entrance_Angle], portal[potl_Entrance_Interior], portal[potl_Entrance_World]);

            Portal_SetExit(index, portal[potl_Exit_Pos][0], portal[potl_Exit_Pos][1], portal[potl_Exit_Pos][2], portal[potl_Exit_Angle], portal[potl_Exit_Interior], portal[potl_Exit_World]);
        }
    }

    printf("> %d portals have been initiated succesfully", rows);
    
    print("INFO: ./modules/portal loaded");
    return 1;
}
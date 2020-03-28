static enum _:E_PERSONAL_VEHICLE_DATA
{
    pveh_Id,
    pveh_UUID[UUID_LEN],
    pveh_Owner_UUID[UUID_LEN]
}

static List:personalVehicles;

forward bool:Vehicle_GetParam(const vehicleid, const param); // avoid using before definition warning
forward bool:Vehicle_IsKeyInserted(const vehicleid); // avoid using before definition warning

stock Task:PersonalVehicle_Create(const owner_uuid[], const model, const Float:x, const Float:y, const Float:z, const Float:rotation, const color1, const color2, const interior, const world, const respawn_delay, const addsiren = 0)
{
    new Task:t = task_new();
    task_detach();
    task_yield(t);

    {
        await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT COUNT(*) FROM characters WHERE uuid = '%e' LIMIT 1", owner_uuid));

        new count = 0;
        cache_get_value_name_int(0, "COUNT(*)", count);
        if(count == 0)
        {
            task_set_result(t, -1);
            return Task:0;
        }
    }
    
    new vehicleid = Vehicle_Create(model, x, y, z, rotation, color1, color2, interior, world, respawn_delay, addsiren),
        index = -1;
    
    if(vehicleid != -1)
    {
        new vehicle[E_PERSONAL_VEHICLE_DATA],
            key[24],
            uuid[UUID_LEN];

        Vehicle_GenerateKey(key, 24);

        Vehicle_SetType(vehicleid, VEHICLE_TYPE_PERSONAL);
        Vehicle_SetKey(vehicleid, key);

        vehicle[pveh_Id] = vehicleid; 
        UUID(vehicle[pveh_UUID], UUID_LEN);
        strcopy(vehicle[pveh_Owner_UUID], owner_uuid, UUID_LEN);
        
        index = list_add_arr(personalVehicles, vehicle);

        {
            new query[226 + 37 + 37 + 24 + 40];
            mysql_format(MySQL_GetHandle(), query, sizeof(query), "INSERT INTO personal_vehicles (uuid, owner_uuid, vehicle_key, model, primary_color, secondary_color, siren, pos_x, pos_y, pos_z, angle, interior, world) VALUES ('%e', '%e', '%e', %d, %d, %d, %d, %.4f, %.4f, %.4f, %.4f, %d, %d)", vehicle[pveh_UUID], owner_uuid, key, model, color1, color2, addsiren, x, y, z, rotation, interior, world);
            mysql_tquery(MySQL_GetHandle(), query);
        }

        new playerid = Player_GetByCharacterUUID(owner_uuid);

        if(playerid == -1)
        {
            new query[140 + 37 + 37 + 24 + 10];
            for(new i = 0; i < 2; i++)
            {
                UUID(uuid, sizeof(uuid));
                mysql_format(MySQL_GetHandle(), query, sizeof(query), "INSERT INTO inventory (item_uuid, item_id, item_amount, item_number_of_uses, item_key, character_uuid) VALUES ('%e', %d, %d, %d, '%e', '%e')", uuid, 44, 1, 1, key, owner_uuid);
                mysql_tquery(MySQL_GetHandle(), query);
            }
        }
        else
        {
            Inventory_Add(playerid, 44, 1, 1, key);
            Inventory_Add(playerid, 44, 1, 1, key);
            
            new vehicle_name[17];
            Vehicle_GetNameById(vehicleid, vehicle_name, sizeof(vehicle_name));
            Player_Msg(playerid, COLOR_GREEN, sprintf("> Info: Ban da nhan duoc hai chia khoa cho phuong tien '%s'.", vehicle_name));
        }
    }

    task_set_result(t, index);

    return Task:0;
}

stock Task:PersonalVehicle_Spawn(const uuid[])
{
    new Task:t = task_new();
    task_detach();
    task_yield(t);

    await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT * FROM personal_vehicles WHERE uuid = '%e' LIMIT 1", uuid));

    new rows = cache_num_rows();
    if(rows > 0)
    {
        new vehicle[E_PERSONAL_VEHICLE_DATA];

        new bool:key_inserted,
            primary_color,
            secondary_color,
            model,
            key[24],
            bool:params[7],
            damages[4],
            bool:siren,
            Float:pos[4],
            interior,
            world;
        
        strcopy(vehicle[pveh_UUID], uuid, UUID_LEN);
        cache_get_value_name(0, "owner_uuid", vehicle[pveh_Owner_UUID], UUID_LEN);
        cache_get_value_name(0, "vehicle_key", key, sizeof(key));
        
        cache_get_value_name_int(0, "key_inserted", key_inserted);
        cache_get_value_name_int(0, "model", model);
        cache_get_value_name_int(0, "primary_color", primary_color);
        cache_get_value_name_int(0, "secondary_color", secondary_color);
        cache_get_value_name_int(0, "siren", siren);
        cache_get_value_name_int(0, "param_engine", params[0]);
        cache_get_value_name_int(0, "param_lights", params[1]);
        cache_get_value_name_int(0, "param_alarm", params[2]);
        cache_get_value_name_int(0, "param_doors", params[3]);
        cache_get_value_name_int(0, "param_bonnet", params[4]);
        cache_get_value_name_int(0, "param_boot", params[5]);
        cache_get_value_name_int(0, "param_objective", params[6]);
        cache_get_value_name_int(0, "damage_panels", damages[0]);
        cache_get_value_name_int(0, "damage_doors", damages[1]);
        cache_get_value_name_int(0, "damage_lights", damages[2]);
        cache_get_value_name_int(0, "damage_tires", damages[3]);
        cache_get_value_name_int(0, "world", world);
        cache_get_value_name_int(0, "interior", interior);

        cache_get_value_name_float(0, "pos_x", pos[0]);
        cache_get_value_name_float(0, "pos_y", pos[1]);
        cache_get_value_name_float(0, "pos_z", pos[2]);
        cache_get_value_name_float(0, "angle", pos[3]);

        new vehicleid = Vehicle_Create(model, pos[0], pos[1], pos[2], pos[3], primary_color, secondary_color, interior, world, -1, siren);

        if(vehicleid != -1)
        {
            vehicle[pveh_Id] = vehicleid;

            Vehicle_SetType(vehicleid, VEHICLE_TYPE_PERSONAL);
            Vehicle_SetKey(vehicleid, key);
            Vehicle_SetParams(vehicleid, params[0], params[1], params[2], params[3], params[4], params[5], params[6]);
            Vehicle_SetDamages(vehicleid, damages[0], damages[1], damages[2], damages[3]);
            Vehicle_SetKeyInserted(vehicleid, key_inserted);

            list_add_arr(personalVehicles, vehicle);

            task_set_result(t, vehicleid);
            return Task:0;
        }
    }

    task_set_result(t, -1);
    return Task:0;
}

stock PersonalVehicle_Despawn(const vehicleid)
{
    new vehicle[E_PERSONAL_VEHICLE_DATA];
    for(new Iter:i = list_iter(personalVehicles); iter_inside(i); iter_move_next(i))
    {
        if(iter_get_arr_safe(i, vehicle))
        {
            if(vehicle[pveh_Id] == vehicleid)
            {
                iter_erase_deep(i);
                break;
            }
        }
    }
    
    return Vehicle_Destroy(vehicleid);
}

stock Task:PersonalVehicle_CountByOwner(const owner_uuid[])
{
    new Task:t = task_new();
    task_detach();
    task_yield(t);

    await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT COUNT(*) FROM personal_vehicles WHERE owner_uuid = '%e'", owner_uuid));

    new count = 0;
    cache_get_value_name_int(0, "COUNT(*)", count);
    task_set_result(t, count);
    
    return Task:0;
}

stock PersonalVehicle_GetByUUID(const uuid[])
{
    new vehicle[E_PERSONAL_VEHICLE_DATA];
    for(new Iter:i = list_iter(personalVehicles); iter_inside(i); iter_move_next(i))
    {
        iter_get_arr(i, vehicle);
        if(!isnull(vehicle[pveh_UUID]))
        {
            if(!strcmp(uuid, vehicle[pveh_UUID]))
            {
                return vehicle[pveh_Id];
            }
        }
    }
    return 0;
}

stock PersonalVehicle_GetUUID(const vehicleid, uuid[], const size = UUID_LEN)
{
    if(IsValidVehicle(vehicleid))
    {
        new vehicle[E_PERSONAL_VEHICLE_DATA];
        for(new Iter:i = list_iter(personalVehicles); iter_inside(i); iter_move_next(i))
        {
            if(iter_get_arr_safe(i, vehicle))
            {
                if(vehicle[pveh_Id] == vehicleid)
                {
                    uuid[0] = EOS;
                    strcat(uuid, vehicle[pveh_UUID], size);
                    break;
                }
            }
        }
    }
    return 1;
}

stock Task:PersonalVehicle_SetParkingPos(const uuid[], const Float:x, const Float:y, const Float:z, const Float:angle, const interior, const world)
{
    new Task:t = task_new();
    task_detach();
    task_yield(t);

    await mysql_aquery_s(MySQL_GetHandle(), str_format("UPDATE personal_vehicles SET pos_x = %.4f, pos_y = %.4f, pos_z = %.4f, angle = %.4f, interior = %d, world = %d WHERE uuid = '%e' LIMIT 1", x, y, z, angle, interior, world, uuid));

    task_set_result(t, cache_affected_rows());
    return Task:0;
}

stock Task:PersonalVehicle_Park(const vehicleid)
{
    /*
    * Code Errors
    * -1: failed to get vehicle's uuid (maybe vehicle's type is not personal vehicle)
    * -2: row does not exist in db
    * -3: vehicle is not in vehicle's bought parking place (not close to the pos, not in the same interior or not in the same world)
    */
    new Task:t = task_new();
    task_detach();
    task_yield(t);
    
    new uuid[UUID_LEN];
    PersonalVehicle_GetUUID(vehicleid, uuid, sizeof(uuid));

    if(isnull(uuid))
    {
        task_set_result(t, -1); 
        return Task:0;
    }

    await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT pos_x, pos_y, pos_z, interior, world FROM personal_vehicles WHERE uuid = '%e' LIMIT 1", uuid));

    if(cache_num_rows() == 0)
    {
        task_set_result(t, -2);
        return Task:0;
    }

    {
        new Float:pos[3],
            interior,
            world;
        cache_get_value_name_float(0, "pos_x", pos[0]);
        cache_get_value_name_float(0, "pos_y", pos[1]);
        cache_get_value_name_float(0, "pos_z", pos[2]);
        cache_get_value_name_int(0, "interior", interior);
        cache_get_value_name_int(0, "world", world);

        if((GetVehicleDistanceFromPoint(vehicleid, pos[0], pos[1], pos[2]) > 3.0) || Vehicle_GetInterior(vehicleid) != interior || Vehicle_GetWorld(vehicleid) != world)
        {
            task_set_result(t, -3);
            return Task:0;
        }
    }

    new Float:health,
        Float:fuel,
        damages[4],
        query[310 + 37 + 40];
    GetVehicleHealth(vehicleid, health);
    Vehicle_GetFuel(vehicleid, fuel);
    GetVehicleDamageStatus(vehicleid, damages[0], damages[1], damages[2], damages[3]);

    mysql_format(MySQL_GetHandle(), query, sizeof(query), "UPDATE personal_vehicles SET key_inserted = %d, health = %.4f, fuel = %.4f, param_engine = %d, param_lights = %d, param_alarm = %d, param_doors = %d, param_bonnet = %d, param_boot = %d, param_objective = %d, damage_panels = %d, damage_doors = %d, damage_lights = %d, damage_tires = %d WHERE uuid = '%e' LIMIT 1", Vehicle_IsKeyInserted(vehicleid), health, fuel, Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_ENGINE), Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_LIGHTS), Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_ALARM), Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_DOORS), Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_BONNET), Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_BOOT), Vehicle_GetParam(vehicleid, VEHICLE_PARAMS_OBJECTIVE), damages[0], damages[1], damages[2], damages[3], uuid);
    mysql_tquery(MySQL_GetHandle(), query);

    task_set_result(t, PersonalVehicle_Despawn(vehicleid));
    return Task:0;
}

#include <YSI_Coding\y_hooks>

hook OnGameModeInit()
{
    mysql_query(MySQL_GetHandle(), "CREATE TABLE IF NOT EXISTS personal_vehicles (\
        owner_uuid VARCHAR(37) NOT NULL DEFAULT '',\
        uuid VARCHAR(37) NOT NULL PRIMARY KEY DEFAULT '',\
        vehicle_key VARCHAR(24) NOT NULL DEFAULT '',\
        key_inserted BOOLEAN NOT NULL DEFAULT 0,\
        model INT UNSIGNED NOT NULL DEFAULT 0,\
        fuel FLOAT(11, 4) NOT NULL DEFAULT 100.0000,\
        health FLOAT(11, 4) NOT NULL DEFAULT 1000.0000,\
        primary_color INT UNSIGNED NOT NULL DEFAULT 0,\
        secondary_color INT UNSIGNED NOT NULL DEFAULT 1,\
        pos_x FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        pos_y FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        pos_z FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        angle FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        interior INT NOT NULL DEFAULT 0,\
        world INT NOT NULL DEFAULT 0,\
        siren BOOLEAN NOT NULL DEFAULT 0,\
        param_engine TINYINT NOT NULL DEFAULT 0,\
        param_lights TINYINT NOT NULL DEFAULT 0,\
        param_alarm TINYINT NOT NULL DEFAULT 0,\
        param_doors TINYINT NOT NULL DEFAULT 0,\
        param_bonnet TINYINT NOT NULL DEFAULT 0,\
        param_boot TINYINT NOT NULL DEFAULT 0,\
        param_objective TINYINT NOT NULL DEFAULT 0,\
        damage_panels INT UNSIGNED NOT NULL DEFAULT 0,\
        damage_doors INT UNSIGNED NOT NULL DEFAULT 0,\
        damage_lights INT UNSIGNED NOT NULL DEFAULT 0,\
        damage_tires INT UNSIGNED NOT NULL DEFAULT 0,\
        FOREIGN KEY (owner_uuid) REFERENCES characters(uuid) ON UPDATE CASCADE ON DELETE CASCADE\
    );", false);

    personalVehicles = list_new();
    return 1;
}
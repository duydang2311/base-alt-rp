static enum _:E_FACTION_VEHICLE_DATA
{
    fveh_Id,
    fveh_Faction_Id
}

static List:factionVehicles;

stock FactionVehicle_Create(const faction_id, const model, const Float:x, const Float:y, const Float:z, const Float:rotation, const color1, const color2, const respawn_delay, const addsiren = 0)
{
    new vehicleid = Vehicle_Create(model, x, y, z, rotation, color1, color2, respawn_delay, addsiren),
        index = -1;
    
    if(vehicleid != -1)
    {
        new vehicle[E_FACTION_VEHICLE_DATA];

        vehicleData[veh_Type] = VEHICLE_TYPE_FACTION;

        vehicle[fveh_Id] = vehicleid; 
        vehicle[fveh_Faction_Id] = faction_id;
        UUID(vehicle[fveh_UUID], UUID_LEN);
        
        index = list_add_arr(factionVehicles, vehicle);
    }

    return index;
}

#include <YSI_Coding\y_hooks>

hook OnGameModeInit()
{
    factionVehicles = list_new();
    return 1;
}
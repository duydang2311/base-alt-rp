static enum _:E_TEMPORARY_VEHICLE_DATA
{
    tveh_Id
}

static List:temporaryVehicles;

stock TemporaryVehicle_Create()
{
    new vehicleid = Vehicle_Create(model, x, y, z, rotation, color1, color2, respawn_delay, addsiren),
        index = -1;
    
    if(vehicleid != -1)
    {
        new vehicle[E_TEMPORARY_VEHICLE_DATA];

        vehicleData[veh_Type] = VEHICLE_TYPE_TEMPORARY;  

        vehicle[tveh_Id] = vehicleid; 
        
        index = list_add_arr(temporaryVehicles, vehicle);
    }

    return index;
}

#include <YSI_Coding\y_hooks>

hook OnGameModeInit()
{
    temporaryVehicles = list_new();
    return 1;
}
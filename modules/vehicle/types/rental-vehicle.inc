static enum _:E_RENTAL_VEHICLE_DATA
{
    rveh_Id,
    rveh_UUID[UUID_LEN],
    rveh_Renter_UUID[UUID_LEN],
    rveh_Rent_Timestamp
}


static List:rentalVehicles;

stock RentalVehicle_Create(const renter_uuid[], const model, const Float:x, const Float:y, const Float:z, const Float:rotation, const color1, const color2, const respawn_delay, const addsiren = 0)
{
    new vehicleid = Vehicle_Create(model, x, y, z, rotation, color1, color2, respawn_delay, addsiren),
        index = -1;
    
    if(vehicleid != -1)
    {
        new vehicle[E_RENTAL_VEHICLE_DATA];

        vehicleData[veh_Type] = VEHICLE_TYPE_RENTAL;

        vehicle[rveh_Id] = vehicleid; 
        vehicle[vreh_Rent_Timestamp] = gettime();
        UUID(vehicle[rveh_UUID], UUID_LEN);
        strcopy(vehicle[rveh_Renter_UUID], owner_uuid, UUID_LEN);
        
        index = list_add_arr(rentalVehicles, vehicle);
    }

    return index;
}

#include <YSI_Coding\y_hooks>

hook OnGameModeInit()
{
    rentalVehicles = list_new();
    return 1;
}
#if defined _included_vehicle
    #endinput
#endif

#define _included_vehicle

static const vehicleNames[][] =
{
    "Landstalker", "Bravura", "Buffalo", "Linerunner", "Perrenial", "Sentinel",
    "Dumper", "Firetruck", "Trashmaster", "Stretch", "Manana", "Infernus",
    "Voodoo", "Pony", "Mule", "Cheetah", "Ambulance", "Leviathan", "Moonbeam",
    "Esperanto", "Taxi", "Washington", "Bobcat", "Whoopee", "BF Injection",
    "Hunter", "Premier", "Enforcer", "Securicar", "Banshee", "Predator", "Bus",
    "Rhino", "Barracks", "Hotknife", "Trailer", "Previon", "Coach", "Cabbie",
    "Stallion", "Rumpo", "RC Bandit", "Romero", "Packer", "Monster", "Admiral",
    "Squalo", "Seasparrow", "Pizzaboy", "Tram", "Trailer", "Turismo", "Speeder",
    "Reefer", "Tropic", "Flatbed", "Yankee", "Caddy", "Solair", "Berkley's RC Van",
    "Skimmer", "PCJ-600", "Faggio", "Freeway", "RC Baron", "RC Raider", "Glendale",
    "Oceanic","Sanchez", "Sparrow", "Patriot", "Quad", "Coastguard", "Dinghy",
    "Hermes", "Sabre", "Rustler", "ZR-350", "Walton", "Regina", "Comet", "BMX",
    "Burrito", "Camper", "Marquis", "Baggage", "Dozer", "Maverick", "News Chopper",
    "Rancher", "FBI Rancher", "Virgo", "Greenwood", "Jetmax", "Hotring", "Sandking",
    "Blista Compact", "Police Maverick", "Boxville", "Benson", "Mesa", "RC Goblin",
    "Hotring Racer A", "Hotring Racer B", "Bloodring Banger", "Rancher", "Super GT",
    "Elegant", "Journey", "Bike", "Mountain Bike", "Beagle", "Cropduster", "Stunt",
    "Tanker", "Roadtrain", "Nebula", "Majestic", "Buccaneer", "Shamal", "Hydra",
    "FCR-900", "NRG-500", "HPV1000", "Cement Truck", "Tow Truck", "Fortune",
    "Cadrona", "FBI Truck", "Willard", "Forklift", "Tractor", "Combine", "Feltzer",
    "Remington", "Slamvan", "Blade", "Freight", "Streak", "Vortex", "Vincent",
    "Bullet", "Clover", "Sadler", "Firetruck", "Hustler", "Intruder", "Primo",
    "Cargobob", "Tampa", "Sunrise", "Merit", "Utility", "Nevada", "Yosemite",
    "Windsor", "Monster", "Monster", "Uranus", "Jester", "Sultan", "Stratium",
    "Elegy", "Raindance", "RC Tiger", "Flash", "Tahoma", "Savanna", "Bandito",
    "Freight Flat", "Streak Carriage", "Kart", "Mower", "Dune", "Sweeper",
    "Broadway", "Tornado", "AT-400", "DFT-30", "Huntley", "Stafford", "BF-400",
    "News Van", "Tug", "Trailer", "Emperor", "Wayfarer", "Euros", "Hotdog", "Club",
    "Freight Box", "Trailer", "Andromada", "Dodo", "RC Cam", "Launch", "Police Car",
    "Police Car", "Police Car", "Police Ranger", "Picador", "S.W.A.T", "Alpha",
    "Phoenix", "Glendale", "Sadler", "Luggage", "Luggage", "Stairs", "Boxville",
    "Tiller", "Utility Trailer"
};

enum _:e_VEHICLE_TYPE
{
    VEHICLE_TYPE_TEMPORARY,
    VEHICLE_TYPE_PERSONAL,
    VEHICLE_TYPE_RENTAL,
    VEHICLE_TYPE_FACTION
}

enum _:e_VEHICLE_PARAMS
{
    VEHICLE_PARAMS_ENGINE,
    VEHICLE_PARAMS_LIGHTS,
    VEHICLE_PARAMS_ALARM,
    VEHICLE_PARAMS_DOORS,
    VEHICLE_PARAMS_BONNET,
    VEHICLE_PARAMS_BOOT,
    VEHICLE_PARAMS_OBJECTIVE
}

static enum _:E_VEHICLE_DATA
{
    veh_Type,
    veh_Key[24],
    bool:veh_Key_Inserted,
    veh_Interior,
    veh_World,
    veh_Primary_Color,
    veh_Secondary_Color,
    Float:veh_Fuel,
    bool:veh_Params[7],
    veh_Damages[4]
}

static vehicleData[MAX_VEHICLES][E_VEHICLE_DATA];

#include "types\temporary-vehicle.inc"
#include "types\personal-vehicle.inc"
#include "types\rental-vehicle.inc"
#include "types\faction-vehicle.inc"

stock Vehicle_Reset(const vehicleid)
{
    vehicleData[vehicleid][veh_Type] = VEHICLE_TYPE_TEMPORARY;
    vehicleData[vehicleid][veh_Key][0] = EOS;
    vehicleData[vehicleid][veh_Key_Inserted] = false;
    vehicleData[vehicleid][veh_Interior] = 0;
    vehicleData[vehicleid][veh_World] = 0;
    vehicleData[vehicleid][veh_Fuel] = 100.0000;
    vehicleData[vehicleid][veh_Primary_Color] = 0;
    vehicleData[vehicleid][veh_Secondary_Color] = 0;
    vehicleData[vehicleid][veh_Params][0] = false;
    vehicleData[vehicleid][veh_Params][1] = false;
    vehicleData[vehicleid][veh_Params][2] = false;
    vehicleData[vehicleid][veh_Params][3] = true; // doors locked as default
    vehicleData[vehicleid][veh_Params][4] = false;
    vehicleData[vehicleid][veh_Params][5] = false;
    vehicleData[vehicleid][veh_Params][6] = false;
    vehicleData[vehicleid][veh_Damages][0] = 0;
    vehicleData[vehicleid][veh_Damages][1] = 0;
    vehicleData[vehicleid][veh_Damages][2] = 0;
    vehicleData[vehicleid][veh_Damages][3] = 0;
    return 1;
}

stock Vehicle_Create(const model, const Float:x, const Float:y, const Float:z, const Float:rotation, const color1, const color2, const interior, const world, const respawn_delay, const addsiren = 0)
{
    new vehicleid = CreateVehicle(model, x, y, z, rotation, color1, color2, respawn_delay, addsiren);
    
    if(vehicleid == 0 || vehicleid == INVALID_VEHICLE_ID)
    {
        return -1;
    }
    else
    {
        Vehicle_Reset(vehicleid);
        
        Vehicle_SetParams(vehicleid, vehicleData[vehicleid][veh_Params][0], vehicleData[vehicleid][veh_Params][1], vehicleData[vehicleid][veh_Params][2], vehicleData[vehicleid][veh_Params][3], vehicleData[vehicleid][veh_Params][4], vehicleData[vehicleid][veh_Params][5], vehicleData[vehicleid][veh_Params][6]);
        Vehicle_SetInterior(vehicleid, interior);
        Vehicle_SetWorld(vehicleid, world);
        vehicleData[vehicleid][veh_Primary_Color] = color1;
        vehicleData[vehicleid][veh_Secondary_Color] = color2;
        return vehicleid;
    }
}

stock Vehicle_Destroy(const vehicleid)
{
    DestroyVehicle(vehicleid);
    Vehicle_Reset(vehicleid);
    return 1;
}

stock Vehicle_GetClosestVehicleId(const vehicleid, const Float:range)
{
    new Float:max_dist = range + 0.01,
        Float:dist,
        vehicle_id = 0,
        Float:pos[3];

    GetVehiclePos(vehicleid, pos[0], pos[1], pos[2]);

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

stock Vehicle_GenerateKey(key[], const size = 24)
{
    format(key, size, "VEH-%c%c%c%c-%c%c%c%c", Utils_RandomChar(), Utils_RandomChar(), Utils_RandomChar(), Utils_RandomChar(), Utils_RandomChar(), Utils_RandomChar(), Utils_RandomChar(), Utils_RandomChar());
    return 1;
}

stock Vehicle_SetType(const vehicleid, const type)
{
    vehicleData[vehicleid][veh_Type] = type;
    return 1;
}

stock Vehicle_GetType(const vehicleid)
{
    return vehicleData[vehicleid][veh_Type];
}

stock Vehicle_SetKey(const vehicleid, const key[])
{
    vehicleData[vehicleid][veh_Key][0] = EOS;
    strcat(vehicleData[vehicleid][veh_Key], key);
    return 1;
}

stock Vehicle_GetKey(const vehicleid, key[], const size = 24)
{
    strcopy(key, vehicleData[vehicleid][veh_Key], size);
    return 1;
}

stock Vehicle_GetNameById(const vehicleid, name[], const size = 17)
{
    if(IsValidVehicle(vehicleid))
    {
        strcopy(name, vehicleNames[GetVehicleModel(vehicleid) - 400], size);
    }
    return 1;
}

stock Vehicle_GetNameByModel(const model, name[], const size = 17)
{
    strcopy(name, vehicleNames[model - 400], size);
    return 1;
}

stock Vehicle_ToggleParam(const vehicleid, const params)
{
    if(IsValidVehicle(vehicleid))
    {
        vehicleData[vehicleid][veh_Params][params] = !vehicleData[vehicleid][veh_Params][params];
    
        SetVehicleParamsEx(vehicleid, vehicleData[vehicleid][veh_Params][0], vehicleData[vehicleid][veh_Params][1], vehicleData[vehicleid][veh_Params][2], vehicleData[vehicleid][veh_Params][3], vehicleData[vehicleid][veh_Params][4], vehicleData[vehicleid][veh_Params][5], vehicleData[vehicleid][veh_Params][6]);
        return 1;
    }
    return 0;
}

stock Vehicle_SetKeyInserted(const vehicleid, const bool:insert)
{
    vehicleData[vehicleid][veh_Key_Inserted] = insert;
    return 1;
}
   
stock bool:Vehicle_IsKeyInserted(const vehicleid)
{
   return vehicleData[vehicleid][veh_Key_Inserted];
}

stock bool:Vehicle_GetParam(const vehicleid, const param)
{
    if(IsValidVehicle(vehicleid))
    {
        if(param >= 0 && param < 7)
        {
            return vehicleData[vehicleid][veh_Params][param];
        }
    }
    return false;
}

stock Vehicle_SetParams(const vehicleid, const bool:engine, const bool:lights, const bool:alarm, const bool:doors, const bool:bonnet, const bool:boot, const bool:objective)
{
    if(IsValidVehicle(vehicleid))
    {
        vehicleData[vehicleid][veh_Params][0] = engine;
        vehicleData[vehicleid][veh_Params][1] = lights;
        vehicleData[vehicleid][veh_Params][2] = alarm;
        vehicleData[vehicleid][veh_Params][3] = doors;
        vehicleData[vehicleid][veh_Params][4] = bonnet;
        vehicleData[vehicleid][veh_Params][5] = boot;
        vehicleData[vehicleid][veh_Params][6] = objective;

        SetVehicleParamsEx(vehicleid, engine, lights, alarm, doors, bonnet, boot, objective);
    }
    return 1;
}

stock Vehicle_SetDamages(const vehicleid, const panels, const doors, const lights, const tires)
{
    if(IsValidVehicle(vehicleid))
    {
        vehicleData[vehicleid][veh_Damages][0] = panels;
        vehicleData[vehicleid][veh_Damages][1] = doors;
        vehicleData[vehicleid][veh_Damages][2] = lights;
        vehicleData[vehicleid][veh_Damages][3] = tires;

        UpdateVehicleDamageStatus(vehicleid, panels, doors, lights, tires);
    }
    return 1;
}

stock Vehicle_SetInterior(const vehicleid, const interior)
{
    if(IsValidVehicle(vehicleid))
    {
        vehicleData[vehicleid][veh_Interior] = interior;
        LinkVehicleToInterior(vehicleid, interior);
    }
    return 1;
}

stock Vehicle_SetWorld(const vehicleid, const world)
{
    if(IsValidVehicle(vehicleid))
    {
        vehicleData[vehicleid][veh_World] = world;
        SetVehicleVirtualWorld(vehicleid, world);
    }
    return 1;
}

stock Vehicle_GetInterior(const vehicleid)
{
    return vehicleData[vehicleid][veh_Interior];
}

stock Vehicle_GetWorld(const vehicleid)
{
    return vehicleData[vehicleid][veh_World];
}

stock Vehicle_GetPrimaryColor(const vehicleid)
{
    return vehicleData[vehicleid][veh_Primary_Color];
}

stock Vehicle_GetSecondaryColor(const vehicleid)
{
    return vehicleData[vehicleid][veh_Secondary_Color];
}

stock Vehicle_GetFuel(const vehicleid, &Float:fuel)
{
    fuel = vehicleData[vehicleid][veh_Fuel];
    return 1;
}

#include <YSI_Coding\y_hooks>

hook OnGameModeInit()
{
    print("INFO: ./modules/vehicle loaded");
    return 1;
}

#if defined _utils_included
    #endinput
#endif

#define _utils_included

stock Utils_CountCommonPrefix(const a[], const b[])
{
    if(a[0] != b[0])
        return 0;

    new count = 0,
        minLength = (strlen(a) < strlen(b)) ? strlen(a) : strlen(b);

    for(new i = 0; i != minLength; i++)
    {
        if(a[i] == b[i])
        {
            count++;
        }
        else break;
    }
    return count;
}

stock Utils_GetPlayerForwardPos(const playerid, &Float:_x, &Float:_y, const Float:distance)
{
    new Float:_a;
    GetPlayerPos(playerid, _x, _y, _a);
    GetPlayerFacingAngle(playerid, _a);
    if (GetPlayerVehicleID(playerid))
    {
        GetVehicleZAngle(GetPlayerVehicleID(playerid), _a);
    }
    _x += (distance * floatsin(-_a, degrees));
    _y += (distance * floatcos(-_a, degrees));
    return 1;
}

stock Utils_ModelToWeapon(const model)
{
    switch(model) 
    {
        case 331: return 1;
        case 333: return 2;
        case 334: return 3;
        case 335: return 4;
        case 336: return 5;
        case 337: return 6;
        case 338: return 7;
        case 339: return 8;
        case 341: return 9;
        case 321: return 10;
        case 322: return 11;
        case 323: return 12;
        case 324: return 13;
        case 325: return 14;
        case 326: return 15;
        case 342: return 16;
        case 343: return 17;
        case 344: return 18;
        case 346: return 22;
        case 347: return 23;
        case 348: return 24;
        case 349: return 25;
        case 350: return 26;
        case 351: return 27;
        case 352: return 28;
        case 353: return 29;
        case 355: return 30;
        case 356: return 31;
        case 372: return 32;
        case 357: return 33;
        case 358: return 34;
        case 359: return 35;
        case 360: return 36;
        case 361: return 37;
        case 362: return 38;
        case 363: return 39;
        case 364: return 40;
        case 365: return 41;
        case 366: return 42;
        case 367: return 43;
        case 368: return 44;
        case 369: return 45;
        case 371: return 46;
    }
    return -1;
}

stock Utils_GetWeaponSlot(const weaponid)
{
    new slot = -1;
    switch(weaponid)
    {
        case 0, 1: slot = 0;
        case 2 .. 9: slot = 1;
        case 10 .. 15: slot = 10;
        case 16 .. 18, 39: slot = 8;
        case 22 .. 24: slot =2;
        case 25 .. 27: slot = 3;
        case 28, 29, 32: slot = 4;
        case 30, 31: slot = 5;
        case 33, 34: slot = 6;
        case 35 .. 38: slot = 7;
        case 40: slot = 12;
        case 41 .. 43: slot = 9;
        case 44 .. 46: slot = 11;
    }
    return slot;
}

stock Utils_RandomChar()
{
    return ((random(1000) % 2) == 0) ? (65 + random(26)) : (97 + random(26));
}

stock Utils_GetVehicleRelativePos(vehicleid, &Float:x, &Float:y, &Float:z, Float:xoff=0.0, Float:yoff=0.0, Float:zoff=0.0)
{
    new Float:rot;
    GetVehicleZAngle(vehicleid, rot);
    rot = 360 - rot;
    GetVehiclePos(vehicleid, x, y, z);
    x = floatsin(rot,degrees) * yoff + floatcos(rot,degrees) * xoff + x;
    y = floatcos(rot,degrees) * yoff - floatsin(rot,degrees) * xoff + y;
    z = zoff + z;
	return 1;
}
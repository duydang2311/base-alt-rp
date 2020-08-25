#include <portal>

static enum _:E_HOUSE_DATA
{
    house_UUID[UUID_LEN],
    house_Owner_UUID[UUID_LEN],
    house_Key[24],
    house_Price,
    house_Room_Price,
    house_Max_Room,
    bool:house_Locked
}

new List:houses;

stock House_GenerateKey(key[], const size = 24)
{
    format(key, size, "HOUSE-%c%c%c%c-%c%c%c%c", Utils_RandomChar(), Utils_RandomChar(), Utils_RandomChar(), Utils_RandomChar(), Utils_RandomChar(), Utils_RandomChar(), Utils_RandomChar(), Utils_RandomChar());
    return 1;
}

stock House_Create()
{
    new house[E_HOUSE_DATA];

    UUID(house[house_UUID], UUID_LEN);
    house[house_Owner_UUID][0] = EOS;
    House_GenerateKey(house[house_Key]);
    house[house_Price] = 0;
    house[house_Room_Price] = 0;
    house[house_Max_Room] = 4;
    house[house_Locked] = true;

    new query[74 + 37 + 24 + 10];
    mysql_format(MySQL_GetHandle(), query, sizeof(query), "INSERT INTO houses (uuid, owner_uuid, house_key) VALUES ('%e', NULL, '%e')", house[house_UUID], house[house_Key]);
    mysql_tquery(MySQL_GetHandle(), query);

    return list_add_arr(houses, house);
}

stock bool:House_DestroyByIndex(const index)
{
    if(index < 0 || index >= list_size(houses))
    {
        return false;
    }

    new house[E_HOUSE_DATA];
    if(list_get_arr_safe(houses, index, house))
    {
        HousePortal_DestroyByHouseUUID(house[house_UUID]);

        new query[36 + 37 + 10];
        mysql_format(MySQL_GetHandle(), query, sizeof(query), "DELETE FROM houses WHERE uuid = '%e'", house[house_UUID]);
        mysql_tquery(MySQL_GetHandle(), query);

        list_remove_deep(houses, index);

        return true;
    }
    return false;
}

stock bool:House_GetUUID(const index, uuid[], const size = UUID_LEN)
{
    if(index < 0 || index >= list_size(houses))
    {
        return false;
    }

    new house[E_HOUSE_DATA];
    if(list_get_arr_safe(houses, index, house))
    {
        uuid[0] = EOS;
        strcat(uuid, house[house_UUID], size);

        return true;
    }
    return false;
}

stock bool:House_SetPrice(const index, const price)
{
    if(index < 0 || index >= list_size(houses))
    {
        return false;
    }

    new uuid[UUID_LEN];
    House_GetUUID(index, uuid, sizeof(uuid));
    list_set_cell(houses, index, house_Price, price);

    new query[54 + 37 + 20];
    mysql_format(MySQL_GetHandle(), query, sizeof(query), "UPDATE houses SET price = %d WHERE uuid = '%e' LIMIT 1", price, uuid);
    mysql_tquery(MySQL_GetHandle(), query);

    return true;
}

stock bool:House_SetRoomPrice(const index, const price)
{
    if(index < 0 || index >= list_size(houses))
    {
        return false;
    }

    new uuid[UUID_LEN];
    House_GetUUID(index, uuid, sizeof(uuid));
    list_set_cell(houses, index, house_Room_Price, price);

    new query[54 + 37 + 20];
    mysql_format(MySQL_GetHandle(), query, sizeof(query), "UPDATE houses SET room_price = %d WHERE uuid = '%e' LIMIT 1", price, uuid);
    mysql_tquery(MySQL_GetHandle(), query);

    return true;
}

stock bool:House_SetMaxRoom(const index, const amount)
{
    if(index < 0 || index >= list_size(houses))
    {
        return false;
    }

    new uuid[UUID_LEN];
    House_GetUUID(index, uuid, sizeof(uuid));
    list_set_cell(houses, index, house_Max_Room, amount);

    new query[54 + 37 + 20];
    mysql_format(MySQL_GetHandle(), query, sizeof(query), "UPDATE houses SET max_room = %d WHERE uuid = '%e' LIMIT 1", amount, uuid);
    mysql_tquery(MySQL_GetHandle(), query);

    return true;
}

stock bool:House_GetKey(const index, key[], const size = 24)
{
    if(index < 0 || index >= list_size(houses))
    {
        return false;
    }

    new house[E_HOUSE_DATA];
    if(list_get_arr_safe(houses, index, house))
    {
        key[0] = EOS;
        strcat(key, house[house_Key], size);
        
        return true;
    }

    return false;
}

stock House_GetByOwnerUUID(const owner_uuid[])
{
    new house[E_HOUSE_DATA];
    for(new Iter:i = list_iter(houses); iter_inside(i); iter_move_next(i))
    {
        if(iter_get_arr_safe(i, house))
        {
            if(!isnull(house[house_Owner_UUID]))
            {
                if(!strcmp(house[house_Owner_UUID], owner_uuid))
                {
                    return iter_get_key(i);
                }
            }
        }
    }
    return -1;
}

stock House_GetOwnerUUID(const index, uuid[], const size = UUID_LEN)
{
    uuid[0] = EOS;
    if(index < 0 || index >= list_size(houses))
    {
        return 1;
    }

    new house[E_HOUSE_DATA];
    if(list_get_arr_safe(houses, index, house))
    {
        strcat(uuid, house[house_Owner_UUID], size);
    }
    return 1;
}

forward House_OnInitiated(index);
public House_OnInitiated(index)
{
    
    return 1;
}

#include <YSI_Coding\y_hooks>

hook OnGameModeInit()
{
    mysql_query(MySQL_GetHandle(), "CREATE TABLE IF NOT EXISTS houses (\
        uuid VARCHAR(37) NOT NULL PRIMARY KEY DEFAULT '',\
        owner_uuid VARCHAR(37) NULL,\
        house_key VARCHAR(24) NOT NULL DEFAULT '',\
        price INT UNSIGNED NOT NULL DEFAULT 0,\
        room_price INT UNSIGNED NOT NULL DEFAULT 0,\
        max_room TINYINT UNSIGNED NOT NULL DEFAULT 4,\
        locked BOOLEAN NOT NULL DEFAULT 1,\
        created_date TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP,\
        FOREIGN KEY (owner_uuid) REFERENCES characters(uuid) ON UPDATE CASCADE ON DELETE CASCADE\
    );", false);

    houses = list_new();
    task_yield(1);

    print("> Retrieving and initiating houses...");
    
    mysql_query(MySQL_GetHandle(), "SELECT * FROM houses", true);

    new rows = cache_num_rows();
    if(rows > 0)
    {
        new house[E_HOUSE_DATA];
        for(new i = 0; i < rows; i++)
        {
            cache_get_value_name(i, "uuid", house[house_UUID], UUID_LEN);
            cache_get_value_name(i, "owner_uuid", house[house_Owner_UUID], UUID_LEN);
            cache_get_value_name(i, "house_key", house[house_Key], 24);

            cache_get_value_name_int(i, "price", house[house_Price]);
            cache_get_value_name_int(i, "room_price", house[house_Room_Price]);
            cache_get_value_name_int(i, "max_room", house[house_Max_Room]);
            cache_get_value_name_int(i, "locked", house[house_Locked]);

            CallLocalFunction("House_OnInitiated", "i", list_add_arr(houses, house));
        }
    }
    printf("> %d houses have been initiated succesfully", rows);
    
    print("INFO: ./modules/house loaded");
    return 1;
}
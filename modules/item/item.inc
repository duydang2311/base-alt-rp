#if defined _module_item_included
    #endinput
#endif

#define _module_item_included

#include <PawnPlus>
#include <pp-mysql>

#include <mysql>

const ITEM_NAME_SIZE = 64;
const ITEM_DESCRIPTION_SIZE = 64;
const ITEM_TYPE_NONE = 0;
const ITEM_TYPE_WEAPON = 1;

enum _:E_ITEM_DATA
{
	item_Model,
	item_Name[ITEM_NAME_SIZE],
	item_Description[ITEM_DESCRIPTION_SIZE],
	item_Type,
	item_Capacity,
    bool:item_Mergeable,
    bool:item_Force_Visible
}

enum E_ITEM_ATTACHMENT_DATA
{
    item_Attachment_Model,
    item_Attachment_Type,
    item_Attachment_Bone,
    Float:item_Offset_X,
    Float:item_Offset_Y,
    Float:item_Offset_Z,
    Float:item_Rot_X,
    Float:item_Rot_Y,
    Float:item_Rot_Z,
    Float:item_ScaleX,
    Float:item_ScaleY,
    Float:item_ScaleZ
}

/*static const items[][E_ITEM_DATA] =
{
	// model [int] - name [string] - description [string] - type [int] - capacity [int] - combine [bool]
	{331, "Brass Knuckles", "Vu khi can chien", ITEM_TYPE_WEAPON, 1, false},
	{333, "Golf Club", "Vu khi can chien", ITEM_TYPE_WEAPON, 1, false},
	{334, "Nightstick", "Vu khi can chien", ITEM_TYPE_WEAPON, 1, false},
	{335, "Knife", "Vu khi can chien", ITEM_TYPE_WEAPON, 1, false},
	{336, "Baseball Bat", "Vu khi can chien", ITEM_TYPE_WEAPON, 1, false},
	{337, "Shovel", "Vu khi can chien", ITEM_TYPE_WEAPON, 1, false},
	{338, "Pool Cue", "Vu khi can chien", ITEM_TYPE_WEAPON, 1, false},
	{339, "Katana", "Vu khi can chien", ITEM_TYPE_WEAPON, 1, false},
	{341, "Chainsaw", "Vu khi can chien", ITEM_TYPE_WEAPON, 1, false},
	{321, "Purple Dildo", "Vu khi can chien", ITEM_TYPE_WEAPON, 1, false},
	{322, "Dildo", "Vu khi can chien", ITEM_TYPE_WEAPON, 1, false},
	{323, "Vibrator", "Vu khi can chien", ITEM_TYPE_WEAPON, 1, false},
	{324, "Silver Vibrator", "Vu khi can chien", ITEM_TYPE_WEAPON, 1, false},
	{325, "Flowers", "Vu khi can chien", ITEM_TYPE_WEAPON, 1, false},
	{326, "Cane", "Vu khi can chien", ITEM_TYPE_WEAPON, 1, false},
	{342, "Grenade", "Bom", ITEM_TYPE_WEAPON, 1, false},
	{343, "Tear Gas", "Bom khoi", ITEM_TYPE_WEAPON, 1, false},
	{344, "Molotov Cocktail", "Bom lua", ITEM_TYPE_WEAPON, 1, false},
	{346, "9mm", "Sung luc", ITEM_TYPE_WEAPON, 1, false},
	{347, "Silenced 9mm", "Sung luc", ITEM_TYPE_WEAPON, 1, false},
	{348, "Desert Eagle", "Sung luc", ITEM_TYPE_WEAPON, 1, false},
	{349, "Shotgun", "Sung Shotgun", ITEM_TYPE_WEAPON, 1, false},
	{350, "Sawnoff Shotgun", "Sung Shotgun", ITEM_TYPE_WEAPON, 1, false},
	{351, "Combat Shotgun", "Sung Shotgun", ITEM_TYPE_WEAPON, 1, false},
	{352, "Uzi", "Sung tieu lien", ITEM_TYPE_WEAPON, 1, false},
	{353, "MP5", "Sung tieu lien", ITEM_TYPE_WEAPON, 1, false},
	{355, "AK-47", "Sung truong", ITEM_TYPE_WEAPON, 1, false},
	{356, "M4", "Sung truong", ITEM_TYPE_WEAPON, 1, false},
	{372, "Tec-9", "Sung tieu lien", ITEM_TYPE_WEAPON, 1, false},
	{357, "Country Rifle", "Sung ban tia", ITEM_TYPE_WEAPON, 1, false},
	{358, "Sniper Rifle", "Sung ban tia", ITEM_TYPE_WEAPON, 1, false},
	{359, "RPG", "Sung chong tang", ITEM_TYPE_WEAPON, 1, false},
	{360, "HS Rocket", "Sung chong tang", ITEM_TYPE_WEAPON, 1, false},
	{361, "Flamethrower", "Sung thoi lua", ITEM_TYPE_WEAPON, 1, false},
	{362, "Minigun", "Sung may", ITEM_TYPE_WEAPON, 1, false},
	{363, "Satchel Charge", "Bom kich hoat", ITEM_TYPE_WEAPON, 1, false},
	{364, "Detonator", "Thiet bi kich hoat bom", ITEM_TYPE_WEAPON, 1, false},
	{365, "Spraycan", "Binh xit", ITEM_TYPE_WEAPON, 1, false},
	{366, "Fire Extinguisher", "Binh chua chay", ITEM_TYPE_WEAPON, 1, false},
	{367, "Camera", "Thiet bi chup hinh", ITEM_TYPE_WEAPON, 1, false},
	{368, "Night Vision Goggles", "Kinh nhin bong toi", ITEM_TYPE_WEAPON, 1, false},
	{369, "Thermal Goggles", "Kinh nhin nhiet do", ITEM_TYPE_WEAPON, 1, false},
	{1212, "Money", "Tien mat, co the gop vat pham", ITEM_TYPE_NONE, 1, true}
};

static const defaultItemAttachments[][E_ITEM_ATTACHMENT_DATA] =
{
	// model [int] - type [int] - bone [int] - offset[3] [float] - rot[3] [float] - scale[3] [float]
	{333, 0, 1, -0.41, -0.18, -0.10, -17.30, 63.00, -0.00, 1.00, 1.00, 1.00},
	{334, 0, 7, 0.14, -0.03, -0.12, 4.69, 90.00, 83.00, 1.00, 1.00, 1.00},
	{336, 0, 1, -0.18, -0.17, -0.09, 0.00, 55.20, 0.00, 1.00, 1.00, 1.00},
	{337, 0, 1, -0.18, -0.05, -0.03, 0.00, 59.800, -83.00, 1.00, 1.00, 1.00},
	{338, 0, 1, -0.18, -0.17, -0.09, 0.00, 55.20, 0.00, 1.00, 1.00, 1.00},
	{338, 1, 5, 0.06, 0.00, 0.01, 126.70, -6.00, -85.59, 1.00, 1.00, 1.00},
	{338, 2, 6, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 1.00, 1.00, 1.00},
	{339, 0, 1, -0.21, -0.15, -0.07, 0.00, 61.60, 0.00, 1.00, 1.00, 1.00},
	{341, 0, 1, -0.42, -0.19, -0.10, 0.00, 0.00, 0.00, 1.00, 1.00, 1.00},
	{321, 0, 7, 0.03, 0.00, -0.10, 6.30, 97.50, 86.20, 1.00, 1.00, 1.00},
	{322, 0, 7, 0.03, 0.00, -0.10, 6.30, 97.50, 86.20, 1.00, 1.00, 1.00},
	{323, 0, 7, 0.03, 0.00, -0.10, 6.30, 97.50, 86.20, 1.00, 1.00, 1.00},
	{324, 0, 7, 0.03, 0.00, -0.10, 6.30, 97.50, 86.20, 1.00, 1.00, 1.00},
	{325, 0, 7, 0.15, 0.00, -0.08, 0.00, -117.20, 0.00, 1.00, 1.00, 1.00},
	{326, 0, 1, 0.34, -0.11, 0.02, 0.00, -123.50, 0.00, 1.00, 1.00, 1.00},
	{342, 0, 7, 0.05, 0.00, -0.08, 134.90, -124.60, 0.00, 1.00, 1.00, 1.00},
	{343, 0, 7, 0.05, 0.00, -0.08, 134.90, -124.60, 0.00, 1.00, 1.00, 1.00},
	{344, 0, 7, 0.05, 0.00, -0.08, 134.90, -124.60, 0.00, 1.00, 1.00, 1.00},
	{346, 0, 8, 0.00, -0.04, 0.09, -96.00, 0.00, -6.60, 1.00, 1.00, 1.00},
	{347, 0, 8, 0.00, -0.04, 0.09, -96.00, 0.00, -6.60, 1.00, 1.00, 1.00},
	{348, 0, 8, 0.00, -0.04, 0.09, -96.00, 0.00, -6.60, 1.00, 1.00, 1.00},
	{349, 0, 1, 0.16, -0.15, 0.08, 0.00, 161.70, 0.00, 1.00, 1.00, 1.00},
	{350, 0, 8, -0.04, -0.09, 0.10, -86.70, 0.00, 0.00, 1.00, 1.00, 1.00},
	{351, 0, 1, 0.16, -0.15, 0.08, 0.00, 161.70, 0.00, 1.00, 1.00, 1.00},
	{352, 0, 7, 0.04, -0.02, -0.05, -93.70, -0.09, 0.00, 1.00, 1.00, 1.00},
	{353, 0, 7, 0.04, -0.02, -0.05, -93.70, -0.09, 0.00, 1.00, 1.00, 1.00},
	{355, 0, 1, 0.18, -0.13, -0.04, 0.00, -149.50, 0.00, 1.00, 1.00, 1.00},
	{356, 0, 1, 0.18, -0.13, -0.04, 0.00, -149.50, 0.00, 1.00, 1.00, 1.00},
	{372, 0, 7, 0.04, -0.02, -0.05, -93.70, -0.09, 0.00, 1.00, 1.00, 1.00},
	{358, 0, 1, 0.18, -0.13, -0.04, 0.00, -149.50, 0.00, 1.00, 1.00, 1.00}
};*/

stock Task:Item_IsValid(const id)
{
    new Task:t = task_new();
    task_detach();
    task_yield(t);
    
    await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT COUNT(*) FROM items WHERE id = %d LIMIT 1", id));

    new rows;
    cache_get_value_name_int(0, "COUNT(*)", rows);
    task_set_result(t, rows);
    return Task:0;
}

stock Task:Item_RetrieveName(const id)
{
    new Task:my_task = task_new();
    task_detach();
    task_yield(my_task);

    await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT name FROM items WHERE id = %d LIMIT 1", id));

    if(cache_num_rows() > 0)
    {
        new name[64];
        cache_get_value_name(0, "name", name, sizeof(name));
        task_set_result_str(my_task, name);
    }
    else
    {
        task_set_result_str(my_task, "");
    }
    return Task:0;
}

stock Task:Item_RetrieveDescription(const id)
{
    new Task:my_task = task_new();
    task_detach();
    task_yield(my_task);

    await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT description FROM items WHERE id = %d LIMIT 1", id));

    if(cache_num_rows() > 0)
    {
        new description[64];
        cache_get_value_name(0, "description", description, sizeof(description));
        task_set_result_str(my_task, description);
    }
    else
    {
        task_set_result_str(my_task, "");
    }
    return Task:0;
}

stock Task:Item_RetrieveModel(const id)
{
	new Task:my_task = task_new();
    task_detach();
    task_yield(my_task);

    await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT model FROM items WHERE id = %d LIMIT 1", id));

    if(cache_num_rows() > 0)
    {
        new model;
        cache_get_value_name_int(0, "model", model);
        task_set_result(my_task, model);
    }
    else
    {
        task_set_result(my_task, 0);
    }

    return Task:0;
}

stock Task:Item_RetrieveType(const id)
{
	new Task:my_task = task_new();
    task_detach();
    task_yield(my_task);

    await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT type FROM items WHERE id = %d LIMIT 1", id));

    if(cache_num_rows() > 0)
    {
        new type;
        cache_get_value_name_int(0, "type", type);
        task_set_result(my_task, type);
    }
    else
    {
        task_set_result(my_task, -1);
    }

    return Task:0;
}

stock Task:Item_RetrieveCapacity(const id)
{
	new Task:my_task = task_new();
    task_detach();
    task_yield(my_task);

    await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT capacity FROM items WHERE id = %d LIMIT 1", id));

    if(cache_num_rows() > 0)
    {
        new capacity;
        cache_get_value_name_int(0, "capacity", capacity);
        task_set_result(my_task, capacity);
    }
    else
    {
        task_set_result(my_task, 0);
    }

    return Task:0;
}

stock Task:Item_RetrieveMergeable(const id)
{
	new Task:my_task = task_new();
    task_detach();
    task_yield(my_task);

    await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT mergeable FROM items WHERE id = %d LIMIT 1", id));

    if(cache_num_rows() > 0)
    {
        new mergeable;
        cache_get_value_name_int(0, "mergeable", mergeable);
        task_set_result(my_task, mergeable);
    }
    else
    {
        task_set_result(my_task, -1);
    }

    return Task:0;
}

stock Task:Item_RetrieveData(const id)
{
    new Task:my_task = task_new();
    task_detach();
    task_yield(my_task);

    await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT * FROM items WHERE id = %d LIMIT 1", id));

    new item[E_ITEM_DATA];
    if(cache_num_rows() > 0)
    {
        cache_get_value_name_int(0, "model", item[item_Model]);
        cache_get_value_name_int(0, "type", item[item_Type]);
        cache_get_value_name_int(0, "capacity", item[item_Capacity]);
        cache_get_value_name_int(0, "mergeable", item[item_Mergeable]);

        cache_get_value_name(0, "name", item[item_Name], 64);
        cache_get_value_name(0, "description", item[item_Description], 64);
        task_set_result_arr(my_task, item);
    }
    else
    {
        task_set_result_arr(my_task, item);
    }

    return Task:0;
}

#include <YSI_Coding\y_hooks>

hook OnGameModeInit()
{
    mysql_query(MySQL_GetHandle(), "CREATE TABLE IF NOT EXISTS items (\
        id BIGINT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT,\
        model INT NOT NULL DEFAULT 0,\
        name VARCHAR(64) NOT NULL DEFAULT '',\
        description VARCHAR(64) NOT NULL DEFAULT '',\
        type TINYINT NOT NULL DEFAULT 0,\
        capacity TINYINT NOT NULL DEFAULT 1,\
        mergeable BOOLEAN NOT NULL DEFAULT 0\
    );", false);

    /*new string[256];
    for(new i = 0; i < sizeof(items); i++)
    {
        mysql_format(MySQL_GetHandle(), string, sizeof(string), "INSERT INTO items (model, name, description, type, capacity, mergeable) VALUES (%d, '%e', '%e', %d, %d, %d)", items[i][item_Model], items[i][item_Name], items[i][item_Description], items[i][item_Type], items[i][item_Capacity], items[i][item_Mergeable]);
        mysql_query(MySQL_GetHandle(), string, false);
    }*/
    
    print("INFO: ./modules/item loaded");
    return 1;
}
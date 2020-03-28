#if defined _module_inventory_included
    #endinput
#endif

#define _module_inventory_included

#include <PawnPlus>
#include <pp-mysql>
#include <Pawn.CMD>
#include <samp-precise-timers>

#include <config-character-spawn-types>
#include <config-cmd-flags>

#include <mysql>
#include <character>
#include <item>

const MAX_CHARACTER_INVENTORY_ITEMS = 45;
const MAX_INVENTORY_ITEMS_PER_PAGE = 6;

static enum E_ATTACHMENT_DATA
{
    attachment_Bone,
    Float:attachment_Offset_X,
    Float:attachment_Offset_Y,
    Float:attachment_Offset_Z,
    Float:attachment_Rot_X,
    Float:attachment_Rot_Y,
    Float:attachment_Rot_Z,
    Float:attachment_Scale_X,
    Float:attachment_Scale_Y,
    Float:attachment_Scale_Z
}

static enum _:E_CHARACTER_INVENTORY_DATA
{   
    item_UUID[UUID_LEN],
    item_Id,
    item_Key[24],
    item_Amount,
    item_Number_Of_Uses,

    item_State,
    bool:item_Visible,
    item_Attachment_Index
}

static Map:characterInventories;
static currentInventoryPage[MAX_PLAYERS] = {1, ...};

static PlayerText:TD_Inventory_Container[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...},
    PlayerText:TD_Inventory_Items[MAX_PLAYERS][MAX_INVENTORY_ITEMS_PER_PAGE] = {PlayerText:INVALID_TEXT_DRAW, ...},
    PlayerText:TD_Inventory_Page_Container[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...},
    PlayerText:TD_Inventory_Page[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...},
    PlayerText:TD_Inventory_Previous_Page[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...},
    PlayerText:TD_Inventory_Next_Page[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...},
    PlayerText:TD_Inventory_Use[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...},
    PlayerText:TD_Inventory_Drop[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...},
    PlayerText:TD_Inventory_Visible[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...},
    PlayerText:TD_Inventory_Adjust[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...},
    PlayerText:TD_Inventory_Description[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...};

stock Task:Inventory_RetrieveData(const playerid)
{
    new Task:t = task_new();
    task_detach();
    task_yield(t);
    
    if(map_has_key(characterInventories, playerid))
    {
        map_remove_deep(characterInventories, playerid);
    }

    new List:itemList = list_new(),
        uuid[UUID_LEN];
    Character_GetUUID(playerid, uuid, sizeof(uuid));

    await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT * FROM inventory WHERE character_uuid = '%e'", uuid));

    new rows = cache_num_rows();
    if(rows > 0)
    {
        new result[E_CHARACTER_INVENTORY_DATA];
        for(new i = 0; i < rows; i++)
        {
            cache_get_value_name(i, "item_uuid", result[item_UUID], UUID_LEN);
            cache_get_value_name(i, "item_key", result[item_Key], 24);

            cache_get_value_name_int(i, "item_id", result[item_Id]);
            cache_get_value_name_int(i, "item_amount", result[item_Amount]);
            cache_get_value_name_int(i, "item_number_of_uses", result[item_Number_Of_Uses]);
            cache_get_value_name_int(i, "item_state", result[item_State]);
            cache_get_value_name_int(i, "item_visible", result[item_Visible]);

            result[item_Attachment_Index] = -1;
            list_add_arr(itemList, result);
        }
    }

    map_add(characterInventories, playerid, itemList);
    task_set_result(t, rows);
    return Task:0;
}

stock Task:Inventory_Add(const playerid, const item_id, const amount, const number_of_uses, const key[] = "")
{
    if(!map_has_key(characterInventories, playerid))
    {
        return Task:0;
    }

    new Task:t = task_new();
    task_detach();
    task_yield(t);

    await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT * FROM items WHERE id = %d LIMIT 1", item_id));

    new rows = cache_num_rows();
    if(rows > 0)
    {
        new List:itemList = list_new(),
            item[E_CHARACTER_INVENTORY_DATA],
            uuid[UUID_LEN];
        if(map_get_safe(characterInventories, playerid, itemList))
        {
            if(task_await(Item_RetrieveMergeable(item_id)))
            {
                for(new Iter:i = list_iter(itemList); iter_inside(i); iter_move_next(i))
                {
                    iter_get_arr(i, item);
                    if(item[item_Id] == item_id)
                    {
                        iter_set_cell_safe(i, item_Amount, item[item_Amount] + amount);
                        iter_set_cell_safe(i, item_Number_Of_Uses, item[item_Number_Of_Uses] + number_of_uses);
                        await mysql_aquery_s(MySQL_GetHandle(), str_format("UPDATE inventory SET item_amount = %d, item_number_of_uses = %d WHERE item_uuid = '%e' LIMIT 1", item[item_Amount] + amount, item[item_Number_Of_Uses] + number_of_uses, item[item_UUID]));

                        task_set_result(t, rows);
                        return Task:0;
                    }
                }
            }
            
            UUID(item[item_UUID], UUID_LEN);
            Character_GetUUID(playerid, uuid, sizeof(uuid));
            strcopy(item[item_Key], key, 24);
            item[item_Id] = item_id;
            item[item_Amount] = amount;
            item[item_Number_Of_Uses] = number_of_uses;
            item[item_State] = 0;
            item[item_Visible] = false;
            item[item_Attachment_Index] = -1;

            list_add_arr(itemList, item);

            await mysql_aquery_s(MySQL_GetHandle(), str_format("INSERT INTO inventory (item_uuid, item_id, item_amount, item_number_of_uses, item_key, character_uuid) VALUES ('%e', %d, %d, %d, '%e', '%e')", item[item_UUID], item[item_Id], item[item_Amount], item[item_Number_Of_Uses], item[item_Key], uuid));
        }
    }
    
    task_set_result(t, rows);
    return Task:0;
}

stock Task:Inventory_Remove(const playerid, const item_uuid[])
{
    new Task:t = task_new();
    task_detach();
    task_yield(t);
    
    if(!map_has_key(characterInventories, playerid))
    {
        task_set_result(t, 0);
    }

    new List:itemList = list_new(),
        item[E_CHARACTER_INVENTORY_DATA];
    if(map_get_safe(characterInventories, playerid, itemList))
    {
        for(new Iter:i = list_iter(itemList); iter_inside(i); iter_move_next(i))
        {
            iter_get_arr(i, item);
            if(!strcmp(item[item_UUID], item_uuid))
            {
                iter_erase_deep(i);
                break;
            }
        }
    }

    await mysql_aquery_s(MySQL_GetHandle(), str_format("DELETE FROM inventory WHERE item_uuid = '%e'", item_uuid));

    task_set_result(t, cache_affected_rows());
    return Task:0;
}

stock Task:Inventory_RemoveByIndex(const playerid, const index)
{
    new Task:t = task_new();
    task_detach();
    task_yield(t);

    new List:itemList = list_new(),
        item[E_CHARACTER_INVENTORY_DATA];
    if(map_get_safe(characterInventories, playerid, itemList))
    {
        if(list_get_arr_safe(itemList, index, item))
        {
            list_remove_deep(itemList, index);

            await mysql_aquery_s(MySQL_GetHandle(), str_format("DELETE FROM inventory WHERE item_uuid = '%e'", item[item_UUID]));
            task_set_result(t, cache_affected_rows());

            return Task:0;
        }
    }
    task_set_result(t, 0);
    return Task:0;
}

stock Inventory_Show(const playerid, page = 0)
{
    new List:inventory = list_new();
    if(!map_get_safe(characterInventories, playerid, inventory))
    {
        return 0;
    }
    
    if(TD_Inventory_Container[playerid] == PlayerText:INVALID_TEXT_DRAW)
    {
        TD_Inventory_Container[playerid] = CreatePlayerTextDraw(playerid, 375.000000, 165.000000, "_");
        PlayerTextDrawFont(playerid, TD_Inventory_Container[playerid], 1);
        PlayerTextDrawLetterSize(playerid, TD_Inventory_Container[playerid], 0.000000, 23.850000);
        PlayerTextDrawTextSize(playerid, TD_Inventory_Container[playerid], 515.000000, 0.000000);
        PlayerTextDrawSetOutline(playerid, TD_Inventory_Container[playerid], 1);
        PlayerTextDrawSetShadow(playerid, TD_Inventory_Container[playerid], 0);
        PlayerTextDrawAlignment(playerid, TD_Inventory_Container[playerid], 1);
        PlayerTextDrawColor(playerid, TD_Inventory_Container[playerid], -1);
        PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Container[playerid], 255);
        PlayerTextDrawBoxColor(playerid, TD_Inventory_Container[playerid], 135);
        PlayerTextDrawUseBox(playerid, TD_Inventory_Container[playerid], 1);
        PlayerTextDrawSetProportional(playerid, TD_Inventory_Container[playerid], 1);
        PlayerTextDrawSetSelectable(playerid, TD_Inventory_Container[playerid], 0);
    }

    if(TD_Inventory_Page_Container[playerid] == PlayerText:INVALID_TEXT_DRAW)
    {
        TD_Inventory_Page_Container[playerid] = CreatePlayerTextDraw(playerid, 495.000000, 390.000000, "_");
        PlayerTextDrawFont(playerid, TD_Inventory_Page_Container[playerid], 1);
        PlayerTextDrawLetterSize(playerid, TD_Inventory_Page_Container[playerid], 0.000000, 1.000000);
        PlayerTextDrawTextSize(playerid, TD_Inventory_Page_Container[playerid], 170.000000, 40.000000);
        PlayerTextDrawSetOutline(playerid, TD_Inventory_Page_Container[playerid], 1);
        PlayerTextDrawSetShadow(playerid, TD_Inventory_Page_Container[playerid], 0);
        PlayerTextDrawAlignment(playerid, TD_Inventory_Page_Container[playerid], 2);
        PlayerTextDrawColor(playerid, TD_Inventory_Page_Container[playerid], -1);
        PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Page_Container[playerid], 255);
        PlayerTextDrawBoxColor(playerid, TD_Inventory_Page_Container[playerid], 255);
        PlayerTextDrawUseBox(playerid, TD_Inventory_Page_Container[playerid], 1);
        PlayerTextDrawSetProportional(playerid, TD_Inventory_Page_Container[playerid], 1);
        PlayerTextDrawSetSelectable(playerid, TD_Inventory_Page_Container[playerid], 0);
    }

    if(TD_Inventory_Previous_Page[playerid] == PlayerText:INVALID_TEXT_DRAW)
    {
        TD_Inventory_Previous_Page[playerid] = CreatePlayerTextDraw(playerid, 472.000000, 390.000000, "ld_beat:left");
        PlayerTextDrawFont(playerid, TD_Inventory_Previous_Page[playerid], 4);
        PlayerTextDrawLetterSize(playerid, TD_Inventory_Previous_Page[playerid], 0.600000, 2.000000);
        PlayerTextDrawTextSize(playerid, TD_Inventory_Previous_Page[playerid], 12.000000, 12.000000);
        PlayerTextDrawSetOutline(playerid, TD_Inventory_Previous_Page[playerid], 1);
        PlayerTextDrawSetShadow(playerid, TD_Inventory_Previous_Page[playerid], 0);
        PlayerTextDrawAlignment(playerid, TD_Inventory_Previous_Page[playerid], 1);
        PlayerTextDrawColor(playerid, TD_Inventory_Previous_Page[playerid], -1);
        PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Previous_Page[playerid], 255);
        PlayerTextDrawBoxColor(playerid, TD_Inventory_Previous_Page[playerid], 50);
        PlayerTextDrawUseBox(playerid, TD_Inventory_Previous_Page[playerid], 1);
        PlayerTextDrawSetProportional(playerid, TD_Inventory_Previous_Page[playerid], 1);
        PlayerTextDrawSetSelectable(playerid, TD_Inventory_Previous_Page[playerid], 1);
    }

    if(TD_Inventory_Next_Page[playerid] == PlayerText:INVALID_TEXT_DRAW)
    {
        TD_Inventory_Next_Page[playerid] = CreatePlayerTextDraw(playerid, 506.000000, 390.000000, "ld_beat:right");
        PlayerTextDrawFont(playerid, TD_Inventory_Next_Page[playerid], 4);
        PlayerTextDrawLetterSize(playerid, TD_Inventory_Next_Page[playerid], 0.600000, 2.000000);
        PlayerTextDrawTextSize(playerid, TD_Inventory_Next_Page[playerid], 12.000000, 12.000000);
        PlayerTextDrawSetOutline(playerid, TD_Inventory_Next_Page[playerid], 1);
        PlayerTextDrawSetShadow(playerid, TD_Inventory_Next_Page[playerid], 0);
        PlayerTextDrawAlignment(playerid, TD_Inventory_Next_Page[playerid], 1);
        PlayerTextDrawColor(playerid, TD_Inventory_Next_Page[playerid], -1);
        PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Next_Page[playerid], 255);
        PlayerTextDrawBoxColor(playerid, TD_Inventory_Next_Page[playerid], 50);
        PlayerTextDrawUseBox(playerid, TD_Inventory_Next_Page[playerid], 1);
        PlayerTextDrawSetProportional(playerid, TD_Inventory_Next_Page[playerid], 1);
        PlayerTextDrawSetSelectable(playerid, TD_Inventory_Next_Page[playerid], 1);
    }

    new max_page,
        size = list_size(inventory),
        item_models[MAX_INVENTORY_ITEMS_PER_PAGE] = {0, ...},
        items[E_CHARACTER_INVENTORY_DATA],
        index = 0,
        text_page[12];
    
    /*new Float:divide = floatround(size) / floatMAX_INVENTORY_ITEMS_PER_PAGE;
    new rounded = floatround(divide);
    
    if(divide > rounded)
    {
        max_page = rounded + 1;
    }
    else
    {
        max_page = rounded;
    }*/

    max_page = floatround(float(size) / float(MAX_INVENTORY_ITEMS_PER_PAGE), floatround_ceil);

    printf("list_size: %d, MAX_INVENTORY_ITEMS_PER_PAGE: %d, max_page %d", size, MAX_INVENTORY_ITEMS_PER_PAGE, max_page);
    if(page <= 0)
    {
        page = (currentInventoryPage[playerid] > max_page) ? max_page : currentInventoryPage[playerid];
    }
    else
    {
        page = (page > max_page) ? max_page : currentInventoryPage[playerid];
    }

    format(text_page, sizeof(text_page), "%02d/%02d", page, max_page);

    new Iter:i = list_iter(inventory, (page - 1) * MAX_INVENTORY_ITEMS_PER_PAGE);
    pawn_guard(i);
    for(; iter_inside(i); iter_move_next(i))
    {
        iter_get_arr(i, items);
        item_models[index] = await Item_RetrieveModel(items[item_Id]);
        index++;
        if(index >= MAX_INVENTORY_ITEMS_PER_PAGE)
        {
            break;
        }
    }
    iter_delete(i);

    if(TD_Inventory_Items[playerid][0] == PlayerText:INVALID_TEXT_DRAW)
    {
        if(0 < index)
        {
            TD_Inventory_Items[playerid][0] = CreatePlayerTextDraw(playerid, 374.000000, 164.000000, "Preview_Model");
            PlayerTextDrawFont(playerid, TD_Inventory_Items[playerid][0], 5);
            PlayerTextDrawLetterSize(playerid, TD_Inventory_Items[playerid][0], 0.600000, 2.000000);
            PlayerTextDrawTextSize(playerid, TD_Inventory_Items[playerid][0], 70.000000, 70.000000);
            PlayerTextDrawSetOutline(playerid, TD_Inventory_Items[playerid][0], 0);
            PlayerTextDrawSetShadow(playerid, TD_Inventory_Items[playerid][0], 0);
            PlayerTextDrawAlignment(playerid, TD_Inventory_Items[playerid][0], 1);
            PlayerTextDrawColor(playerid, TD_Inventory_Items[playerid][0], -1);
            PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Items[playerid][0], 125);
            PlayerTextDrawBoxColor(playerid, TD_Inventory_Items[playerid][0], 255);
            PlayerTextDrawUseBox(playerid, TD_Inventory_Items[playerid][0], 0);
            PlayerTextDrawSetProportional(playerid, TD_Inventory_Items[playerid][0], 1);
            PlayerTextDrawSetSelectable(playerid, TD_Inventory_Items[playerid][0], 1);
            PlayerTextDrawSetPreviewModel(playerid, TD_Inventory_Items[playerid][0], item_models[0]);
            PlayerTextDrawSetPreviewRot(playerid, TD_Inventory_Items[playerid][0], -10.000000, 0.000000, -20.000000, 1.000000);
            PlayerTextDrawSetPreviewVehCol(playerid, TD_Inventory_Items[playerid][0], 1, 1);

            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][0]);
        }
    }
    else
    {
        if(0 < index)
        {
            PlayerTextDrawSetPreviewModel(playerid, TD_Inventory_Items[playerid][0], item_models[0]);
            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][0]);
        }
        else
        {
            PlayerTextDrawDestroy(playerid, TD_Inventory_Items[playerid][0]);
            TD_Inventory_Items[playerid][0] = PlayerText:INVALID_TEXT_DRAW;
        }
    }

    if(TD_Inventory_Items[playerid][1] == PlayerText:INVALID_TEXT_DRAW)
    {
        if(1 < index)
        {
            TD_Inventory_Items[playerid][1] = CreatePlayerTextDraw(playerid, 446.000000, 164.000000, "Preview_Model");
            PlayerTextDrawFont(playerid, TD_Inventory_Items[playerid][1], 5);
            PlayerTextDrawLetterSize(playerid, TD_Inventory_Items[playerid][1], 0.600000, 2.000000);
            PlayerTextDrawTextSize(playerid, TD_Inventory_Items[playerid][1], 70.000000, 70.000000);
            PlayerTextDrawSetOutline(playerid, TD_Inventory_Items[playerid][1], 0);
            PlayerTextDrawSetShadow(playerid, TD_Inventory_Items[playerid][1], 0);
            PlayerTextDrawAlignment(playerid, TD_Inventory_Items[playerid][1], 1);
            PlayerTextDrawColor(playerid, TD_Inventory_Items[playerid][1], -1);
            PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Items[playerid][1], 125);
            PlayerTextDrawBoxColor(playerid, TD_Inventory_Items[playerid][1], 255);
            PlayerTextDrawUseBox(playerid, TD_Inventory_Items[playerid][1], 0);
            PlayerTextDrawSetProportional(playerid, TD_Inventory_Items[playerid][1], 1);
            PlayerTextDrawSetSelectable(playerid, TD_Inventory_Items[playerid][1], 1);
            PlayerTextDrawSetPreviewModel(playerid, TD_Inventory_Items[playerid][1], item_models[1]);
            PlayerTextDrawSetPreviewRot(playerid, TD_Inventory_Items[playerid][1], -10.000000, 0.000000, -20.000000, 1.000000);
            PlayerTextDrawSetPreviewVehCol(playerid, TD_Inventory_Items[playerid][1], 1, 1);

            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][1]);
        }
    }
    else
    {
        if(1 < index)
        {
            PlayerTextDrawSetPreviewModel(playerid, TD_Inventory_Items[playerid][1], item_models[1]);
            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][1]);
        }   
        else
        {
            PlayerTextDrawDestroy(playerid, TD_Inventory_Items[playerid][1]);
            TD_Inventory_Items[playerid][1] = PlayerText:INVALID_TEXT_DRAW;
        }
    }

    if(TD_Inventory_Items[playerid][2] == PlayerText:INVALID_TEXT_DRAW)
    {
        if(2 < index)
        {
            TD_Inventory_Items[playerid][2] = CreatePlayerTextDraw(playerid, 374.000000, 237.000000, "Preview_Model");
            PlayerTextDrawFont(playerid, TD_Inventory_Items[playerid][2], 5);
            PlayerTextDrawLetterSize(playerid, TD_Inventory_Items[playerid][2], 0.600000, 2.000000);
            PlayerTextDrawTextSize(playerid, TD_Inventory_Items[playerid][2], 70.000000, 70.000000);
            PlayerTextDrawSetOutline(playerid, TD_Inventory_Items[playerid][2], 0);
            PlayerTextDrawSetShadow(playerid, TD_Inventory_Items[playerid][2], 0);
            PlayerTextDrawAlignment(playerid, TD_Inventory_Items[playerid][2], 1);
            PlayerTextDrawColor(playerid, TD_Inventory_Items[playerid][2], -1);
            PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Items[playerid][2], 125);
            PlayerTextDrawBoxColor(playerid, TD_Inventory_Items[playerid][2], 255);
            PlayerTextDrawUseBox(playerid, TD_Inventory_Items[playerid][2], 0);
            PlayerTextDrawSetProportional(playerid, TD_Inventory_Items[playerid][2], 1);
            PlayerTextDrawSetSelectable(playerid, TD_Inventory_Items[playerid][2], 1);
            PlayerTextDrawSetPreviewModel(playerid, TD_Inventory_Items[playerid][2], item_models[2]);
            PlayerTextDrawSetPreviewRot(playerid, TD_Inventory_Items[playerid][2], -10.000000, 0.000000, -20.000000, 1.000000);
            PlayerTextDrawSetPreviewVehCol(playerid, TD_Inventory_Items[playerid][2], 1, 1);

            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][2]);
        }
    }
    else
    {
        if(2 < index)
        {
            PlayerTextDrawSetPreviewModel(playerid, TD_Inventory_Items[playerid][2], item_models[2]);
            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][2]);
        }   
        else
        {
            PlayerTextDrawDestroy(playerid, TD_Inventory_Items[playerid][2]);
            TD_Inventory_Items[playerid][2] = PlayerText:INVALID_TEXT_DRAW;
        }
    }

    if(TD_Inventory_Items[playerid][3] == PlayerText:INVALID_TEXT_DRAW && 3 < index)
    {
        if(3 < index)
        {
            TD_Inventory_Items[playerid][3] = CreatePlayerTextDraw(playerid, 446.000000, 237.000000, "Preview_Model");
            PlayerTextDrawFont(playerid, TD_Inventory_Items[playerid][3], 5);
            PlayerTextDrawLetterSize(playerid, TD_Inventory_Items[playerid][3], 0.600000, 2.000000);
            PlayerTextDrawTextSize(playerid, TD_Inventory_Items[playerid][3], 70.000000, 70.000000);
            PlayerTextDrawSetOutline(playerid, TD_Inventory_Items[playerid][3], 0);
            PlayerTextDrawSetShadow(playerid, TD_Inventory_Items[playerid][3], 0);
            PlayerTextDrawAlignment(playerid, TD_Inventory_Items[playerid][3], 1);
            PlayerTextDrawColor(playerid, TD_Inventory_Items[playerid][3], -1);
            PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Items[playerid][3], 125);
            PlayerTextDrawBoxColor(playerid, TD_Inventory_Items[playerid][3], 255);
            PlayerTextDrawUseBox(playerid, TD_Inventory_Items[playerid][3], 0);
            PlayerTextDrawSetProportional(playerid, TD_Inventory_Items[playerid][3], 1);
            PlayerTextDrawSetSelectable(playerid, TD_Inventory_Items[playerid][3], 1);
            PlayerTextDrawSetPreviewModel(playerid, TD_Inventory_Items[playerid][3], item_models[3]);
            PlayerTextDrawSetPreviewRot(playerid, TD_Inventory_Items[playerid][3], -10.000000, 0.000000, -20.000000, 1.000000);
            PlayerTextDrawSetPreviewVehCol(playerid, TD_Inventory_Items[playerid][3], 1, 1);

            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][3]);
        }
    }
    else
    {
        if(3 < index)
        {
            PlayerTextDrawSetPreviewModel(playerid, TD_Inventory_Items[playerid][3], item_models[3]);
            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][3]);
        }   
        else
        {
            PlayerTextDrawDestroy(playerid, TD_Inventory_Items[playerid][3]);
            TD_Inventory_Items[playerid][3] = PlayerText:INVALID_TEXT_DRAW;
        }
    }

    if(TD_Inventory_Items[playerid][4] == PlayerText:INVALID_TEXT_DRAW && 4 < index)
    {
        if(4 < index)
        {
            TD_Inventory_Items[playerid][4] = CreatePlayerTextDraw(playerid, 374.000000, 310.000000, "Preview_Model");
            PlayerTextDrawFont(playerid, TD_Inventory_Items[playerid][4], 5);
            PlayerTextDrawLetterSize(playerid, TD_Inventory_Items[playerid][4], 0.600000, 2.000000);
            PlayerTextDrawTextSize(playerid, TD_Inventory_Items[playerid][4], 70.000000, 70.000000);
            PlayerTextDrawSetOutline(playerid, TD_Inventory_Items[playerid][4], 0);
            PlayerTextDrawSetShadow(playerid, TD_Inventory_Items[playerid][4], 0);
            PlayerTextDrawAlignment(playerid, TD_Inventory_Items[playerid][4], 1);
            PlayerTextDrawColor(playerid, TD_Inventory_Items[playerid][4], -1);
            PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Items[playerid][4], 125);
            PlayerTextDrawBoxColor(playerid, TD_Inventory_Items[playerid][4], 255);
            PlayerTextDrawUseBox(playerid, TD_Inventory_Items[playerid][4], 0);
            PlayerTextDrawSetProportional(playerid, TD_Inventory_Items[playerid][4], 1);
            PlayerTextDrawSetSelectable(playerid, TD_Inventory_Items[playerid][4], 1);
            PlayerTextDrawSetPreviewModel(playerid, TD_Inventory_Items[playerid][4], item_models[4]);
            PlayerTextDrawSetPreviewRot(playerid, TD_Inventory_Items[playerid][4], -10.000000, 0.000000, -20.000000, 1.000000);
            PlayerTextDrawSetPreviewVehCol(playerid, TD_Inventory_Items[playerid][4], 1, 1);

            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][4]);
        }
    }
    else
    {
        if(4 < index)
        {
            PlayerTextDrawSetPreviewModel(playerid, TD_Inventory_Items[playerid][4], item_models[4]);
            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][4]);
        }   
        else
        {
            PlayerTextDrawDestroy(playerid, TD_Inventory_Items[playerid][4]);
            TD_Inventory_Items[playerid][4] = PlayerText:INVALID_TEXT_DRAW;
        }
    }


    if(TD_Inventory_Items[playerid][5] == PlayerText:INVALID_TEXT_DRAW && 5 < index)
    {
        if(5 < index)
        {
            TD_Inventory_Items[playerid][5] = CreatePlayerTextDraw(playerid, 446.000000, 310.000000, "Preview_Model");
            PlayerTextDrawFont(playerid, TD_Inventory_Items[playerid][5], 5);
            PlayerTextDrawLetterSize(playerid, TD_Inventory_Items[playerid][5], 0.600000, 2.000000);
            PlayerTextDrawTextSize(playerid, TD_Inventory_Items[playerid][5], 70.000000, 70.000000);
            PlayerTextDrawSetOutline(playerid, TD_Inventory_Items[playerid][5], 0);
            PlayerTextDrawSetShadow(playerid, TD_Inventory_Items[playerid][5], 0);
            PlayerTextDrawAlignment(playerid, TD_Inventory_Items[playerid][5], 1);
            PlayerTextDrawColor(playerid, TD_Inventory_Items[playerid][5], -1);
            PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Items[playerid][5], 125);
            PlayerTextDrawBoxColor(playerid, TD_Inventory_Items[playerid][5], 255);
            PlayerTextDrawUseBox(playerid, TD_Inventory_Items[playerid][5], 0);
            PlayerTextDrawSetProportional(playerid, TD_Inventory_Items[playerid][5], 1);
            PlayerTextDrawSetSelectable(playerid, TD_Inventory_Items[playerid][5], 1);
            PlayerTextDrawSetPreviewModel(playerid, TD_Inventory_Items[playerid][5], item_models[5]);
            PlayerTextDrawSetPreviewRot(playerid, TD_Inventory_Items[playerid][5], -10.000000, 0.000000, -20.000000, 1.000000);
            PlayerTextDrawSetPreviewVehCol(playerid, TD_Inventory_Items[playerid][5], 1, 1);

            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][5]);
        }
    }
    else
    {
        if(5 < index)
        {
            PlayerTextDrawSetPreviewModel(playerid, TD_Inventory_Items[playerid][5], item_models[5]);
            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][5]);
        }   
        else
        {
            PlayerTextDrawDestroy(playerid, TD_Inventory_Items[playerid][5]);
            TD_Inventory_Items[playerid][5] = PlayerText:INVALID_TEXT_DRAW;
        }
    }

    if(TD_Inventory_Page[playerid] == PlayerText:INVALID_TEXT_DRAW)
    {
        TD_Inventory_Page[playerid] = CreatePlayerTextDraw(playerid, 495.000000, 390.000000, text_page);
        PlayerTextDrawFont(playerid, TD_Inventory_Page[playerid], 1);
        PlayerTextDrawLetterSize(playerid, TD_Inventory_Page[playerid], 0.200000, 1.000000);
        PlayerTextDrawTextSize(playerid, TD_Inventory_Page[playerid], 9.000000, 22.000000);
        PlayerTextDrawSetOutline(playerid, TD_Inventory_Page[playerid], 1);
        PlayerTextDrawSetShadow(playerid, TD_Inventory_Page[playerid], 0);
        PlayerTextDrawAlignment(playerid, TD_Inventory_Page[playerid], 2);
        PlayerTextDrawColor(playerid, TD_Inventory_Page[playerid], -1);
        PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Page[playerid], 255);
        PlayerTextDrawBoxColor(playerid, TD_Inventory_Page[playerid], 0);
        PlayerTextDrawUseBox(playerid, TD_Inventory_Page[playerid], 1);
        PlayerTextDrawSetProportional(playerid, TD_Inventory_Page[playerid], 1);
        PlayerTextDrawSetSelectable(playerid, TD_Inventory_Page[playerid], 1);
    }
    else
    {
        PlayerTextDrawSetString(playerid, TD_Inventory_Page[playerid], text_page);
    }

    PlayerTextDrawShow(playerid, TD_Inventory_Container[playerid]);
    PlayerTextDrawShow(playerid, TD_Inventory_Page_Container[playerid]);
    PlayerTextDrawShow(playerid, TD_Inventory_Page[playerid]);
    PlayerTextDrawShow(playerid, TD_Inventory_Previous_Page[playerid]);
    PlayerTextDrawShow(playerid, TD_Inventory_Next_Page[playerid]);

    if(GetPVarType(playerid, "TD_Inventory_Select"))
    {
        index = GetPVarInt(playerid, "TD_Inventory_Select");
        if(index >= (page - 1) * MAX_INVENTORY_ITEMS_PER_PAGE && index < (page - 1) * MAX_INVENTORY_ITEMS_PER_PAGE + MAX_INVENTORY_ITEMS_PER_PAGE)
        {
            PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Items[playerid][index - (page - 1) * MAX_INVENTORY_ITEMS_PER_PAGE], COLOR_ALT);
            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][index - (page - 1) * MAX_INVENTORY_ITEMS_PER_PAGE]);
        }
    }

    SetPVarInt(playerid, "TD_Inventory_Active", 1);
    SelectTextDraw(playerid, COLOR_ALT_HOVER);
    return 1;
}

stock Inventory_SwitchPage(const playerid, page)
{
    new List:inventory = list_new();
    if(!map_get_safe(characterInventories, playerid, inventory))
    {
        Inventory_Hide(playerid);
    }

    if(GetPVarType(playerid, "TD_Inventory_Select"))
    {
        new index = GetPVarInt(playerid, "TD_Inventory_Select");
        if(index >= (currentInventoryPage[playerid] - 1) * MAX_INVENTORY_ITEMS_PER_PAGE && index < (currentInventoryPage[playerid] - 1) * MAX_INVENTORY_ITEMS_PER_PAGE + MAX_INVENTORY_ITEMS_PER_PAGE)
        {
            PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Items[playerid][index - (currentInventoryPage[playerid] - 1) * MAX_INVENTORY_ITEMS_PER_PAGE], 125);
        }
    }

    new max_page = floatround(float(list_size(inventory)) / floatround(MAX_INVENTORY_ITEMS_PER_PAGE), floatround_ceil),
        item_models[MAX_INVENTORY_ITEMS_PER_PAGE] = {0, ...},
        items[E_CHARACTER_INVENTORY_DATA],
        index = 0,
        text_page[12];

    page = (page > max_page) ? max_page : page;
    currentInventoryPage[playerid] = page;

    format(text_page, sizeof(text_page), "%02d/%02d", page, max_page);

    new Iter:i = list_iter(inventory, (page - 1) * MAX_INVENTORY_ITEMS_PER_PAGE);
    pawn_guard(i);
    for(; iter_inside(i); iter_move_next(i))
    {
        iter_get_arr(i, items);
        item_models[index] = await Item_RetrieveModel(items[item_Id]);
        index++;
        if(index >= MAX_INVENTORY_ITEMS_PER_PAGE)
        {
            break;
        }
    }
    iter_delete(i);

    printf("switchpage - index %d", index);
    if(TD_Inventory_Items[playerid][0] == PlayerText:INVALID_TEXT_DRAW)
    {
        if(0 < index)
        {
            TD_Inventory_Items[playerid][0] = CreatePlayerTextDraw(playerid, 374.000000, 164.000000, "Preview_Model");
            PlayerTextDrawFont(playerid, TD_Inventory_Items[playerid][0], 5);
            PlayerTextDrawLetterSize(playerid, TD_Inventory_Items[playerid][0], 0.600000, 2.000000);
            PlayerTextDrawTextSize(playerid, TD_Inventory_Items[playerid][0], 70.000000, 70.000000);
            PlayerTextDrawSetOutline(playerid, TD_Inventory_Items[playerid][0], 0);
            PlayerTextDrawSetShadow(playerid, TD_Inventory_Items[playerid][0], 0);
            PlayerTextDrawAlignment(playerid, TD_Inventory_Items[playerid][0], 1);
            PlayerTextDrawColor(playerid, TD_Inventory_Items[playerid][0], -1);
            PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Items[playerid][0], 125);
            PlayerTextDrawBoxColor(playerid, TD_Inventory_Items[playerid][0], 255);
            PlayerTextDrawUseBox(playerid, TD_Inventory_Items[playerid][0], 0);
            PlayerTextDrawSetProportional(playerid, TD_Inventory_Items[playerid][0], 1);
            PlayerTextDrawSetSelectable(playerid, TD_Inventory_Items[playerid][0], 1);
            PlayerTextDrawSetPreviewModel(playerid, TD_Inventory_Items[playerid][0], item_models[0]);
            PlayerTextDrawSetPreviewRot(playerid, TD_Inventory_Items[playerid][0], -10.000000, 0.000000, -20.000000, 1.000000);
            PlayerTextDrawSetPreviewVehCol(playerid, TD_Inventory_Items[playerid][0], 1, 1);

            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][0]);
        }
    }
    else
    {
        if(0 < index)
        {
            PlayerTextDrawSetPreviewModel(playerid, TD_Inventory_Items[playerid][0], item_models[0]);
            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][0]);
        }
        else
        {
            PlayerTextDrawDestroy(playerid, TD_Inventory_Items[playerid][0]);
            TD_Inventory_Items[playerid][0] = PlayerText:INVALID_TEXT_DRAW;
        }
    }

    if(TD_Inventory_Items[playerid][1] == PlayerText:INVALID_TEXT_DRAW)
    {
        if(1 < index)
        {
            TD_Inventory_Items[playerid][1] = CreatePlayerTextDraw(playerid, 446.000000, 164.000000, "Preview_Model");
            PlayerTextDrawFont(playerid, TD_Inventory_Items[playerid][1], 5);
            PlayerTextDrawLetterSize(playerid, TD_Inventory_Items[playerid][1], 0.600000, 2.000000);
            PlayerTextDrawTextSize(playerid, TD_Inventory_Items[playerid][1], 70.000000, 70.000000);
            PlayerTextDrawSetOutline(playerid, TD_Inventory_Items[playerid][1], 0);
            PlayerTextDrawSetShadow(playerid, TD_Inventory_Items[playerid][1], 0);
            PlayerTextDrawAlignment(playerid, TD_Inventory_Items[playerid][1], 1);
            PlayerTextDrawColor(playerid, TD_Inventory_Items[playerid][1], -1);
            PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Items[playerid][1], 125);
            PlayerTextDrawBoxColor(playerid, TD_Inventory_Items[playerid][1], 255);
            PlayerTextDrawUseBox(playerid, TD_Inventory_Items[playerid][1], 0);
            PlayerTextDrawSetProportional(playerid, TD_Inventory_Items[playerid][1], 1);
            PlayerTextDrawSetSelectable(playerid, TD_Inventory_Items[playerid][1], 1);
            PlayerTextDrawSetPreviewModel(playerid, TD_Inventory_Items[playerid][1], item_models[1]);
            PlayerTextDrawSetPreviewRot(playerid, TD_Inventory_Items[playerid][1], -10.000000, 0.000000, -20.000000, 1.000000);
            PlayerTextDrawSetPreviewVehCol(playerid, TD_Inventory_Items[playerid][1], 1, 1);

            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][1]);
        }
    }
    else
    {
        if(1 < index)
        {
            PlayerTextDrawSetPreviewModel(playerid, TD_Inventory_Items[playerid][1], item_models[1]);
            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][1]);
        }   
        else
        {
            PlayerTextDrawDestroy(playerid, TD_Inventory_Items[playerid][1]);
            TD_Inventory_Items[playerid][1] = PlayerText:INVALID_TEXT_DRAW;
        }
    }

    if(TD_Inventory_Items[playerid][2] == PlayerText:INVALID_TEXT_DRAW)
    {
        if(2 < index)
        {
            TD_Inventory_Items[playerid][2] = CreatePlayerTextDraw(playerid, 374.000000, 237.000000, "Preview_Model");
            PlayerTextDrawFont(playerid, TD_Inventory_Items[playerid][2], 5);
            PlayerTextDrawLetterSize(playerid, TD_Inventory_Items[playerid][2], 0.600000, 2.000000);
            PlayerTextDrawTextSize(playerid, TD_Inventory_Items[playerid][2], 70.000000, 70.000000);
            PlayerTextDrawSetOutline(playerid, TD_Inventory_Items[playerid][2], 0);
            PlayerTextDrawSetShadow(playerid, TD_Inventory_Items[playerid][2], 0);
            PlayerTextDrawAlignment(playerid, TD_Inventory_Items[playerid][2], 1);
            PlayerTextDrawColor(playerid, TD_Inventory_Items[playerid][2], -1);
            PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Items[playerid][2], 125);
            PlayerTextDrawBoxColor(playerid, TD_Inventory_Items[playerid][2], 255);
            PlayerTextDrawUseBox(playerid, TD_Inventory_Items[playerid][2], 0);
            PlayerTextDrawSetProportional(playerid, TD_Inventory_Items[playerid][2], 1);
            PlayerTextDrawSetSelectable(playerid, TD_Inventory_Items[playerid][2], 1);
            PlayerTextDrawSetPreviewModel(playerid, TD_Inventory_Items[playerid][2], item_models[2]);
            PlayerTextDrawSetPreviewRot(playerid, TD_Inventory_Items[playerid][2], -10.000000, 0.000000, -20.000000, 1.000000);
            PlayerTextDrawSetPreviewVehCol(playerid, TD_Inventory_Items[playerid][2], 1, 1);

            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][2]);
        }
    }
    else
    {
        if(2 < index)
        {
            PlayerTextDrawSetPreviewModel(playerid, TD_Inventory_Items[playerid][2], item_models[2]);
            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][2]);
        }   
        else
        {
            PlayerTextDrawDestroy(playerid, TD_Inventory_Items[playerid][2]);
            TD_Inventory_Items[playerid][2] = PlayerText:INVALID_TEXT_DRAW;
        }
    }

    if(TD_Inventory_Items[playerid][3] == PlayerText:INVALID_TEXT_DRAW && 3 < index)
    {
        if(3 < index)
        {
            TD_Inventory_Items[playerid][3] = CreatePlayerTextDraw(playerid, 446.000000, 237.000000, "Preview_Model");
            PlayerTextDrawFont(playerid, TD_Inventory_Items[playerid][3], 5);
            PlayerTextDrawLetterSize(playerid, TD_Inventory_Items[playerid][3], 0.600000, 2.000000);
            PlayerTextDrawTextSize(playerid, TD_Inventory_Items[playerid][3], 70.000000, 70.000000);
            PlayerTextDrawSetOutline(playerid, TD_Inventory_Items[playerid][3], 0);
            PlayerTextDrawSetShadow(playerid, TD_Inventory_Items[playerid][3], 0);
            PlayerTextDrawAlignment(playerid, TD_Inventory_Items[playerid][3], 1);
            PlayerTextDrawColor(playerid, TD_Inventory_Items[playerid][3], -1);
            PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Items[playerid][3], 125);
            PlayerTextDrawBoxColor(playerid, TD_Inventory_Items[playerid][3], 255);
            PlayerTextDrawUseBox(playerid, TD_Inventory_Items[playerid][3], 0);
            PlayerTextDrawSetProportional(playerid, TD_Inventory_Items[playerid][3], 1);
            PlayerTextDrawSetSelectable(playerid, TD_Inventory_Items[playerid][3], 1);
            PlayerTextDrawSetPreviewModel(playerid, TD_Inventory_Items[playerid][3], item_models[3]);
            PlayerTextDrawSetPreviewRot(playerid, TD_Inventory_Items[playerid][3], -10.000000, 0.000000, -20.000000, 1.000000);
            PlayerTextDrawSetPreviewVehCol(playerid, TD_Inventory_Items[playerid][3], 1, 1);

            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][3]);
        }
    }
    else
    {
        if(3 < index)
        {
            PlayerTextDrawSetPreviewModel(playerid, TD_Inventory_Items[playerid][3], item_models[3]);
            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][3]);
        }   
        else
        {
            PlayerTextDrawDestroy(playerid, TD_Inventory_Items[playerid][3]);
            TD_Inventory_Items[playerid][3] = PlayerText:INVALID_TEXT_DRAW;
        }
    }

    if(TD_Inventory_Items[playerid][4] == PlayerText:INVALID_TEXT_DRAW && 4 < index)
    {
        if(4 < index)
        {
            TD_Inventory_Items[playerid][4] = CreatePlayerTextDraw(playerid, 374.000000, 310.000000, "Preview_Model");
            PlayerTextDrawFont(playerid, TD_Inventory_Items[playerid][4], 5);
            PlayerTextDrawLetterSize(playerid, TD_Inventory_Items[playerid][4], 0.600000, 2.000000);
            PlayerTextDrawTextSize(playerid, TD_Inventory_Items[playerid][4], 70.000000, 70.000000);
            PlayerTextDrawSetOutline(playerid, TD_Inventory_Items[playerid][4], 0);
            PlayerTextDrawSetShadow(playerid, TD_Inventory_Items[playerid][4], 0);
            PlayerTextDrawAlignment(playerid, TD_Inventory_Items[playerid][4], 1);
            PlayerTextDrawColor(playerid, TD_Inventory_Items[playerid][4], -1);
            PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Items[playerid][4], 125);
            PlayerTextDrawBoxColor(playerid, TD_Inventory_Items[playerid][4], 255);
            PlayerTextDrawUseBox(playerid, TD_Inventory_Items[playerid][4], 0);
            PlayerTextDrawSetProportional(playerid, TD_Inventory_Items[playerid][4], 1);
            PlayerTextDrawSetSelectable(playerid, TD_Inventory_Items[playerid][4], 1);
            PlayerTextDrawSetPreviewModel(playerid, TD_Inventory_Items[playerid][4], item_models[4]);
            PlayerTextDrawSetPreviewRot(playerid, TD_Inventory_Items[playerid][4], -10.000000, 0.000000, -20.000000, 1.000000);
            PlayerTextDrawSetPreviewVehCol(playerid, TD_Inventory_Items[playerid][4], 1, 1);

            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][4]);
        }
    }
    else
    {
        if(4 < index)
        {
            PlayerTextDrawSetPreviewModel(playerid, TD_Inventory_Items[playerid][4], item_models[4]);
            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][4]);
        }   
        else
        {
            PlayerTextDrawDestroy(playerid, TD_Inventory_Items[playerid][4]);
            TD_Inventory_Items[playerid][4] = PlayerText:INVALID_TEXT_DRAW;
        }
    }


    if(TD_Inventory_Items[playerid][5] == PlayerText:INVALID_TEXT_DRAW && 5 < index)
    {
        if(5 < index)
        {
            TD_Inventory_Items[playerid][5] = CreatePlayerTextDraw(playerid, 446.000000, 310.000000, "Preview_Model");
            PlayerTextDrawFont(playerid, TD_Inventory_Items[playerid][5], 5);
            PlayerTextDrawLetterSize(playerid, TD_Inventory_Items[playerid][5], 0.600000, 2.000000);
            PlayerTextDrawTextSize(playerid, TD_Inventory_Items[playerid][5], 70.000000, 70.000000);
            PlayerTextDrawSetOutline(playerid, TD_Inventory_Items[playerid][5], 0);
            PlayerTextDrawSetShadow(playerid, TD_Inventory_Items[playerid][5], 0);
            PlayerTextDrawAlignment(playerid, TD_Inventory_Items[playerid][5], 1);
            PlayerTextDrawColor(playerid, TD_Inventory_Items[playerid][5], -1);
            PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Items[playerid][5], 125);
            PlayerTextDrawBoxColor(playerid, TD_Inventory_Items[playerid][5], 255);
            PlayerTextDrawUseBox(playerid, TD_Inventory_Items[playerid][5], 0);
            PlayerTextDrawSetProportional(playerid, TD_Inventory_Items[playerid][5], 1);
            PlayerTextDrawSetSelectable(playerid, TD_Inventory_Items[playerid][5], 1);
            PlayerTextDrawSetPreviewModel(playerid, TD_Inventory_Items[playerid][5], item_models[5]);
            PlayerTextDrawSetPreviewRot(playerid, TD_Inventory_Items[playerid][5], -10.000000, 0.000000, -20.000000, 1.000000);
            PlayerTextDrawSetPreviewVehCol(playerid, TD_Inventory_Items[playerid][5], 1, 1);

            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][5]);
        }
    }
    else
    {
        if(5 < index)
        {
            PlayerTextDrawSetPreviewModel(playerid, TD_Inventory_Items[playerid][5], item_models[5]);
            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][5]);
        }   
        else
        {
            PlayerTextDrawDestroy(playerid, TD_Inventory_Items[playerid][5]);
            TD_Inventory_Items[playerid][5] = PlayerText:INVALID_TEXT_DRAW;
        }
    }

    if(TD_Inventory_Page[playerid] == PlayerText:INVALID_TEXT_DRAW)
    {
        TD_Inventory_Page[playerid] = CreatePlayerTextDraw(playerid, 495.000000, 390.000000, text_page);
        PlayerTextDrawFont(playerid, TD_Inventory_Page[playerid], 1);
        PlayerTextDrawLetterSize(playerid, TD_Inventory_Page[playerid], 0.200000, 1.000000);
        PlayerTextDrawTextSize(playerid, TD_Inventory_Page[playerid], 9.000000, 22.000000);
        PlayerTextDrawSetOutline(playerid, TD_Inventory_Page[playerid], 1);
        PlayerTextDrawSetShadow(playerid, TD_Inventory_Page[playerid], 0);
        PlayerTextDrawAlignment(playerid, TD_Inventory_Page[playerid], 2);
        PlayerTextDrawColor(playerid, TD_Inventory_Page[playerid], -1);
        PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Page[playerid], 255);
        PlayerTextDrawBoxColor(playerid, TD_Inventory_Page[playerid], 0);
        PlayerTextDrawUseBox(playerid, TD_Inventory_Page[playerid], 1);
        PlayerTextDrawSetProportional(playerid, TD_Inventory_Page[playerid], 1);
        PlayerTextDrawSetSelectable(playerid, TD_Inventory_Page[playerid], 1);
    }
    else
    {
        PlayerTextDrawSetString(playerid, TD_Inventory_Page[playerid], text_page);
    }

    if(GetPVarType(playerid, "TD_Inventory_Select"))
    {
        index = GetPVarInt(playerid, "TD_Inventory_Select");
        if(index >= (currentInventoryPage[playerid] - 1) * MAX_INVENTORY_ITEMS_PER_PAGE && index < (currentInventoryPage[playerid] - 1) * MAX_INVENTORY_ITEMS_PER_PAGE + MAX_INVENTORY_ITEMS_PER_PAGE)
        {
            PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Items[playerid][index - (currentInventoryPage[playerid] - 1) * MAX_INVENTORY_ITEMS_PER_PAGE], COLOR_ALT);
            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][index - (currentInventoryPage[playerid] - 1) * MAX_INVENTORY_ITEMS_PER_PAGE]);
        }
    }

    return 1;
}

stock Inventory_Hide(const playerid)
{
    if(TD_Inventory_Container[playerid] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, TD_Inventory_Container[playerid]);
        TD_Inventory_Container[playerid] = PlayerText:INVALID_TEXT_DRAW;
    }

    if(TD_Inventory_Items[playerid][0] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, TD_Inventory_Items[playerid][0]);
        TD_Inventory_Items[playerid][0] = PlayerText:INVALID_TEXT_DRAW;
    }

    if(TD_Inventory_Items[playerid][1] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, TD_Inventory_Items[playerid][1]);
        TD_Inventory_Items[playerid][1] = PlayerText:INVALID_TEXT_DRAW;
    }

    if(TD_Inventory_Items[playerid][2] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, TD_Inventory_Items[playerid][2]);
        TD_Inventory_Items[playerid][2] = PlayerText:INVALID_TEXT_DRAW;
    }

    if(TD_Inventory_Items[playerid][3] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, TD_Inventory_Items[playerid][3]);
        TD_Inventory_Items[playerid][3] = PlayerText:INVALID_TEXT_DRAW;
    }

    if(TD_Inventory_Items[playerid][4] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, TD_Inventory_Items[playerid][4]);
        TD_Inventory_Items[playerid][4] = PlayerText:INVALID_TEXT_DRAW;
    }

    if(TD_Inventory_Items[playerid][5] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, TD_Inventory_Items[playerid][5]);
        TD_Inventory_Items[playerid][5] = PlayerText:INVALID_TEXT_DRAW;
    }

    if(TD_Inventory_Page_Container[playerid] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, TD_Inventory_Page_Container[playerid]);
        TD_Inventory_Page_Container[playerid] = PlayerText:INVALID_TEXT_DRAW;
    }

    if(TD_Inventory_Page[playerid] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, TD_Inventory_Page[playerid]);
        TD_Inventory_Page[playerid] = PlayerText:INVALID_TEXT_DRAW;
    }

    if(TD_Inventory_Previous_Page[playerid] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, TD_Inventory_Previous_Page[playerid]);
        TD_Inventory_Previous_Page[playerid] = PlayerText:INVALID_TEXT_DRAW;
    }

    if(TD_Inventory_Next_Page[playerid] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, TD_Inventory_Next_Page[playerid]);
        TD_Inventory_Next_Page[playerid] = PlayerText:INVALID_TEXT_DRAW;
    }

    if(TD_Inventory_Use[playerid] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, TD_Inventory_Use[playerid]);
        TD_Inventory_Use[playerid] = PlayerText:INVALID_TEXT_DRAW;
    }

    if(TD_Inventory_Drop[playerid] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, TD_Inventory_Drop[playerid]);
        TD_Inventory_Drop[playerid] = PlayerText:INVALID_TEXT_DRAW;
    }

    if(TD_Inventory_Visible[playerid] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, TD_Inventory_Visible[playerid]);
        TD_Inventory_Visible[playerid] = PlayerText:INVALID_TEXT_DRAW;
    }

    if(TD_Inventory_Adjust[playerid] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, TD_Inventory_Adjust[playerid]);
        TD_Inventory_Adjust[playerid] = PlayerText:INVALID_TEXT_DRAW;
    }

    if(TD_Inventory_Description[playerid] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, TD_Inventory_Description[playerid]);
        TD_Inventory_Description[playerid] = PlayerText:INVALID_TEXT_DRAW;
    }

    DeletePVar(playerid, "TD_Inventory_Active");
    return 1;
}

stock Inventory_SelectItem(const playerid, const index)
{
    {
        new List:inventory = list_new(),
            item[E_CHARACTER_INVENTORY_DATA];

        if(map_get_safe(characterInventories, playerid, inventory))
        {
            if(list_get_arr_safe(inventory, index, item))
            {
                if(item[item_State] != 0)
                {
                    PlayerTextDrawSetString(playerid, TD_Inventory_Use[playerid], "CAT_VAO");
                }
                else
                {
                    PlayerTextDrawSetString(playerid, TD_Inventory_Use[playerid], "SU_DUNG");
                }
                
                if(item[item_Visible])
                {
                    PlayerTextDrawSetString(playerid, TD_Inventory_Visible[playerid], "An");
                }
                else
                {
                    PlayerTextDrawSetString(playerid, TD_Inventory_Visible[playerid], "HIEN");
                }
            }
        }
    }
    
    // if item index is being displayed in current page, update its box color
    if(index >= (currentInventoryPage[playerid] - 1) * MAX_INVENTORY_ITEMS_PER_PAGE && index < (currentInventoryPage[playerid] - 1) * MAX_INVENTORY_ITEMS_PER_PAGE + MAX_INVENTORY_ITEMS_PER_PAGE)
    {
        PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Items[playerid][index - (currentInventoryPage[playerid] - 1) * MAX_INVENTORY_ITEMS_PER_PAGE], COLOR_ALT);
        PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][index - (currentInventoryPage[playerid] - 1) * MAX_INVENTORY_ITEMS_PER_PAGE]);
    }
    SetPVarInt(playerid, "TD_Inventory_Select", index);
    return 1;
}

stock Inventory_Deselect(const playerid)
{
    if(GetPVarType(playerid, "TD_Inventory_Select"))
    {
        new index = GetPVarInt(playerid, "TD_Inventory_Select");
        if(index >= (currentInventoryPage[playerid] - 1) * MAX_INVENTORY_ITEMS_PER_PAGE && index < (currentInventoryPage[playerid] - 1) * MAX_INVENTORY_ITEMS_PER_PAGE + MAX_INVENTORY_ITEMS_PER_PAGE)
        {
            PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Items[playerid][index - (currentInventoryPage[playerid] - 1) * MAX_INVENTORY_ITEMS_PER_PAGE], 125);
            PlayerTextDrawShow(playerid, TD_Inventory_Items[playerid][index - (currentInventoryPage[playerid] - 1) * MAX_INVENTORY_ITEMS_PER_PAGE]);
        }
        DeletePVar(playerid, "TD_Inventory_Select");
    }
    return 1;
}

stock Inventory_DescribeItem(const playerid, const index)
{
    new List:inventory = list_new();
    if(map_get_safe(characterInventories, playerid, inventory))
    {
        new inventory_item[E_CHARACTER_INVENTORY_DATA];
        if(list_get_arr_safe(inventory, index, inventory_item))
        {
            new valid = await Item_IsValid(inventory_item[item_Id]);

            if(valid > 0)
            {
                new string[47 + 12 + 14 + 64 + 24 + 64],
                    item[E_ITEM_DATA];

                await_arr(item) Item_RetrieveData(inventory_item[item_Id]);

                format(string, sizeof(string), "Vat_pham:_%s~n~So_luong:_%d~n~So_lan_su_dung:_%d", item[item_Name], inventory_item[item_Amount], inventory_item[item_Number_Of_Uses]);

                if(!isnull(inventory_item[item_Key]))
                {
                    format(string, sizeof(string), "%s~n~Key:_%s", string, inventory_item[item_Key]);
                }

                format(string, sizeof(string), "%s~n~Mo_ta:_%s", string, item[item_Description]);

                if(TD_Inventory_Description[playerid] == PlayerText:INVALID_TEXT_DRAW)
                {
                    TD_Inventory_Description[playerid] = CreatePlayerTextDraw(playerid, 525.000000, 165.000000, string);
                    PlayerTextDrawFont(playerid, TD_Inventory_Description[playerid], 1);
                    PlayerTextDrawLetterSize(playerid, TD_Inventory_Description[playerid], 0.250000, 1.000000);
                    PlayerTextDrawTextSize(playerid, TD_Inventory_Description[playerid], 610.000000, 17.500000);
                    PlayerTextDrawSetOutline(playerid, TD_Inventory_Description[playerid], 1);
                    PlayerTextDrawSetShadow(playerid, TD_Inventory_Description[playerid], 0);
                    PlayerTextDrawAlignment(playerid, TD_Inventory_Description[playerid], 1);
                    PlayerTextDrawColor(playerid, TD_Inventory_Description[playerid], -1);
                    PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Description[playerid], 255);
                    PlayerTextDrawBoxColor(playerid, TD_Inventory_Description[playerid], 170);
                    PlayerTextDrawUseBox(playerid, TD_Inventory_Description[playerid], 1);
                    PlayerTextDrawSetProportional(playerid, TD_Inventory_Description[playerid], 1);
                    PlayerTextDrawSetSelectable(playerid, TD_Inventory_Description[playerid], 0);
                    
                    PlayerTextDrawShow(playerid, TD_Inventory_Description[playerid]);
                }
                else
                {
                    PlayerTextDrawSetString(playerid, TD_Inventory_Description[playerid], string);
                }
            }
        }
        return 1;
    }
    return 0;
}

stock Inventory_HideDescription(const playerid)
{
    if(TD_Inventory_Description[playerid] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, TD_Inventory_Description[playerid]);
        TD_Inventory_Description[playerid] = PlayerText:INVALID_TEXT_DRAW;
    }
    return 1;
}

stock Inventory_ShowButtons(const playerid)
{
    if(TD_Inventory_Use[playerid] == PlayerText:INVALID_TEXT_DRAW)
    {
        TD_Inventory_Use[playerid] = CreatePlayerTextDraw(playerid, 387.500000, 390.000000, "SU_DUNG");
        PlayerTextDrawFont(playerid, TD_Inventory_Use[playerid], 1);
        PlayerTextDrawLetterSize(playerid, TD_Inventory_Use[playerid], 0.200000, 1.000000);
        PlayerTextDrawTextSize(playerid, TD_Inventory_Use[playerid], 11.000000, 25.000000);
        PlayerTextDrawSetOutline(playerid, TD_Inventory_Use[playerid], 1);
        PlayerTextDrawSetShadow(playerid, TD_Inventory_Use[playerid], 0);
        PlayerTextDrawAlignment(playerid, TD_Inventory_Use[playerid], 2);
        PlayerTextDrawColor(playerid, TD_Inventory_Use[playerid], -1);
        PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Use[playerid], 255);
        PlayerTextDrawBoxColor(playerid, TD_Inventory_Use[playerid], 255);
        PlayerTextDrawUseBox(playerid, TD_Inventory_Use[playerid], 1);
        PlayerTextDrawSetProportional(playerid, TD_Inventory_Use[playerid], 1);
        PlayerTextDrawSetSelectable(playerid, TD_Inventory_Use[playerid], 1);
    }

    if(TD_Inventory_Drop[playerid] == PlayerText:INVALID_TEXT_DRAW)
    {
        TD_Inventory_Drop[playerid] = CreatePlayerTextDraw(playerid, 422.500000, 390.000000, "VUT");
        PlayerTextDrawFont(playerid, TD_Inventory_Drop[playerid], 1);
        PlayerTextDrawLetterSize(playerid, TD_Inventory_Drop[playerid], 0.200000, 1.000000);
        PlayerTextDrawTextSize(playerid, TD_Inventory_Drop[playerid], 11.000000, 25.000000);
        PlayerTextDrawSetOutline(playerid, TD_Inventory_Drop[playerid], 1);
        PlayerTextDrawSetShadow(playerid, TD_Inventory_Drop[playerid], 0);
        PlayerTextDrawAlignment(playerid, TD_Inventory_Drop[playerid], 2);
        PlayerTextDrawColor(playerid, TD_Inventory_Drop[playerid], -1);
        PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Drop[playerid], 255);
        PlayerTextDrawBoxColor(playerid, TD_Inventory_Drop[playerid], 255);
        PlayerTextDrawUseBox(playerid, TD_Inventory_Drop[playerid], 1);
        PlayerTextDrawSetProportional(playerid, TD_Inventory_Drop[playerid], 1);
        PlayerTextDrawSetSelectable(playerid, TD_Inventory_Drop[playerid], 1);
    }

    if(TD_Inventory_Visible[playerid] == PlayerText:INVALID_TEXT_DRAW)
    {
        TD_Inventory_Visible[playerid] = CreatePlayerTextDraw(playerid, 387.500000, 410.000000, "AN/HIEN");
        PlayerTextDrawFont(playerid, TD_Inventory_Visible[playerid], 1);
        PlayerTextDrawLetterSize(playerid, TD_Inventory_Visible[playerid], 0.200000, 1.000000);
        PlayerTextDrawTextSize(playerid, TD_Inventory_Visible[playerid], 11.000000, 25.000000);
        PlayerTextDrawSetOutline(playerid, TD_Inventory_Visible[playerid], 1);
        PlayerTextDrawSetShadow(playerid, TD_Inventory_Visible[playerid], 0);
        PlayerTextDrawAlignment(playerid, TD_Inventory_Visible[playerid], 2);
        PlayerTextDrawColor(playerid, TD_Inventory_Visible[playerid], -1);
        PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Visible[playerid], 255);
        PlayerTextDrawBoxColor(playerid, TD_Inventory_Visible[playerid], 255);
        PlayerTextDrawUseBox(playerid, TD_Inventory_Visible[playerid], 1);
        PlayerTextDrawSetProportional(playerid, TD_Inventory_Visible[playerid], 1);
        PlayerTextDrawSetSelectable(playerid, TD_Inventory_Visible[playerid], 1);
    }

    if(TD_Inventory_Adjust[playerid] == PlayerText:INVALID_TEXT_DRAW)
    {
        TD_Inventory_Adjust[playerid] = CreatePlayerTextDraw(playerid, 440.000000, 410.000000, "DIEU_CHINH_VI_TRI");
        PlayerTextDrawFont(playerid, TD_Inventory_Adjust[playerid], 1);
        PlayerTextDrawLetterSize(playerid, TD_Inventory_Adjust[playerid], 0.200000, 1.000000);
        PlayerTextDrawTextSize(playerid, TD_Inventory_Adjust[playerid], 11.000000, 60.000000);
        PlayerTextDrawSetOutline(playerid, TD_Inventory_Adjust[playerid], 1);
        PlayerTextDrawSetShadow(playerid, TD_Inventory_Adjust[playerid], 0);
        PlayerTextDrawAlignment(playerid, TD_Inventory_Adjust[playerid], 2);
        PlayerTextDrawColor(playerid, TD_Inventory_Adjust[playerid], -1);
        PlayerTextDrawBackgroundColor(playerid, TD_Inventory_Adjust[playerid], 255);
        PlayerTextDrawBoxColor(playerid, TD_Inventory_Adjust[playerid], 255);
        PlayerTextDrawUseBox(playerid, TD_Inventory_Adjust[playerid], 1);
        PlayerTextDrawSetProportional(playerid, TD_Inventory_Adjust[playerid], 1);
        PlayerTextDrawSetSelectable(playerid, TD_Inventory_Adjust[playerid], 1);
    }

    PlayerTextDrawShow(playerid, TD_Inventory_Use[playerid]);
    PlayerTextDrawShow(playerid, TD_Inventory_Drop[playerid]);
    PlayerTextDrawShow(playerid, TD_Inventory_Visible[playerid]);
    PlayerTextDrawShow(playerid, TD_Inventory_Adjust[playerid]);
    return 1;
}

stock Inventory_HideButtons(const playerid)
{
    if(TD_Inventory_Use[playerid] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, TD_Inventory_Use[playerid]);
        TD_Inventory_Use[playerid] = PlayerText:INVALID_TEXT_DRAW;
    }

    if(TD_Inventory_Drop[playerid] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, TD_Inventory_Drop[playerid]);
        TD_Inventory_Drop[playerid] = PlayerText:INVALID_TEXT_DRAW;
    }

    if(TD_Inventory_Visible[playerid] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, TD_Inventory_Visible[playerid]);
        TD_Inventory_Visible[playerid] = PlayerText:INVALID_TEXT_DRAW;
    }

    if(TD_Inventory_Adjust[playerid] != PlayerText:INVALID_TEXT_DRAW)
    {
        PlayerTextDrawDestroy(playerid, TD_Inventory_Adjust[playerid]);
        TD_Inventory_Adjust[playerid] = PlayerText:INVALID_TEXT_DRAW;
    }
    return 1;
}

stock Inventory_DetachItem(const playerid, const index)
{
    new List:inventory = list_new(),
        item[E_CHARACTER_INVENTORY_DATA];

    if(map_get_safe(characterInventories, playerid, inventory))
    {
        if(list_get_arr_safe(inventory, index, item))
        {
            if(item[item_Visible])
            {
                if(item[item_State] != 0)
                {
                    return 0;
                }

                if(item[item_Attachment_Index] != -1)
                {
                    RemovePlayerAttachedObject(playerid, item[item_Attachment_Index]);
                }

                list_set_cell_safe(inventory, index, item_Attachment_Index, -1);
                list_set_cell_safe(inventory, index, item_Visible, 0);

                new query[68 + 37];
                mysql_format(MySQL_GetHandle(), query, sizeof(query), "UPDATE inventory SET item_Visible = 0 WHERE item_uuid = '%e' LIMIT 1", item[item_UUID]);
                mysql_tquery(MySQL_GetHandle(), query);
                return 1;
            }
        }
    }
    return 0;
}

stock Task:Inventory_AttachItem(const playerid, const index)
{
    new Task:t = task_new();
    task_detach();
    task_yield(t);
    
    new uuid[UUID_LEN],
        List:inventory = list_new(),
        item[E_CHARACTER_INVENTORY_DATA];
    Character_GetUUID(playerid, uuid, sizeof(uuid));

    if(map_get_safe(characterInventories, playerid, inventory))
    {
        if(list_get_arr_safe(inventory, index, item))
        {
            new attachment_index = (item[item_Attachment_Index] == -1) ? Player_GetFreeAttachmentIndex(playerid) : item[item_Attachment_Index];

            if(attachment_index == -1)
            {
                task_set_result(t, attachment_index);
                return Task:0;
            }

            await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT * FROM inventory_item_attachments WHERE character_uuid = '%e' AND item_uuid = '%e' AND item_state = %d LIMIT 1", uuid, item[item_UUID], item[item_State]));

            if(cache_num_rows() > 0)
            {
                new attachment[E_ATTACHMENT_DATA];

                cache_get_value_name_int(0, "item_attachment_bone", attachment[attachment_Bone]);

                cache_get_value_name_float(0, "item_offset_x", attachment[attachment_Offset_X]);
                cache_get_value_name_float(0, "item_offset_y", attachment[attachment_Offset_Y]);
                cache_get_value_name_float(0, "item_offset_z", attachment[attachment_Offset_Z]);
                cache_get_value_name_float(0, "item_rot_x", attachment[attachment_Rot_X]);
                cache_get_value_name_float(0, "item_rot_y", attachment[attachment_Rot_Y]);
                cache_get_value_name_float(0, "item_rot_z", attachment[attachment_Rot_Z]);
                cache_get_value_name_float(0, "item_scale_x", attachment[attachment_Scale_X]);
                cache_get_value_name_float(0, "item_scale_y", attachment[attachment_Scale_Y]);
                cache_get_value_name_float(0, "item_scale_z", attachment[attachment_Scale_Z]);
                
                SetPlayerAttachedObject(playerid, attachment_index, task_await(Item_RetrieveModel(item[item_Id])), attachment[attachment_Bone], attachment[attachment_Offset_X], attachment[attachment_Offset_Y], attachment[attachment_Offset_Z], attachment[attachment_Rot_X], attachment[attachment_Rot_Y], attachment[attachment_Rot_Z], attachment[attachment_Scale_X], attachment[attachment_Scale_Y], attachment[attachment_Scale_Z]);

                list_set_cell_safe(inventory, index, item_Attachment_Index, attachment_index);
                list_set_cell_safe(inventory, index, item_Visible, 1);

                new query[68 + 37];
                mysql_format(MySQL_GetHandle(), query, sizeof(query), "UPDATE inventory SET item_Visible = 1 WHERE item_uuid = '%e' LIMIT 1", item[item_UUID]);
                mysql_tquery(MySQL_GetHandle(), query);

                task_set_result(t, attachment_index);
            }
            else
            {
                await mysql_aquery_s(MySQL_GetHandle(), str_format("INSERT INTO inventory_item_attachments (character_uuid, item_uuid, item_state, item_attachment_bone) VALUES ('%e', '%e', %d, %d)", uuid, item[item_UUID], item[item_State], (item[item_State] == 0) ? 1 : item[item_State] + 4));
                
                SetPlayerAttachedObject(playerid, attachment_index, task_await(Item_RetrieveModel(item[item_Id])), (item[item_State] == 0) ? 1 : item[item_State] + 4, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0);

                list_set_cell_safe(inventory, index, item_Attachment_Index, attachment_index);
                list_set_cell_safe(inventory, index, item_Visible, 1);

                new query[68 + 37];
                mysql_format(MySQL_GetHandle(), query, sizeof(query), "UPDATE inventory SET item_Visible = 1 WHERE item_uuid = '%e' LIMIT 1", item[item_UUID]);
                mysql_tquery(MySQL_GetHandle(), query);

                task_set_result(t, attachment_index);
            }
        }
        else
        {
            task_set_result(t, -1);
        }
    }
    else
    {
        task_set_result(t, -1);
    }
    return Task:0;
}

stock Inventory_GetByIdAndKey(const playerid, const item_id, const item_key[])
{
    new List:inventory = list_new();
    if(map_get_safe(characterInventories, playerid, inventory))
    {
        new item[E_CHARACTER_INVENTORY_DATA];
        for(new Iter:i = list_iter(inventory); iter_inside(i); iter_move_next(i))
        {
            if(iter_get_arr_safe(i, item))
            {
                if(item[item_Id] == item_id)
                {
                    if(!isnull(item[item_Key]))
                    {
                        if(!strcmp(item[item_Key], item_key))
                        {
                            return iter_get_key(i);
                        }
                    }
                }
            }
        }
    }
    return -1;
}

#include <YSI_Coding\y_hooks>

hook OnGameModeInit()
{
    mysql_query(MySQL_GetHandle(), "CREATE TABLE IF NOT EXISTS inventory (\
        item_uuid VARCHAR(37) NOT NULL PRIMARY KEY DEFAULT '',\
        item_id BIGINT UNSIGNED NOT NULL DEFAULT 0,\
        item_key VARCHAR(24) NOT NULL DEFAULT '',\
        item_amount INT UNSIGNED NOT NULL DEFAULT 0,\
        item_number_of_uses INT UNSIGNED NOT NULL DEFAULT 0,\
        item_state BOOLEAN NOT NULL DEFAULT 0,\
        item_visible BOOLEAN NOT NULL DEFAULT 0,\
        created_date TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP,\
        character_uuid VARCHAR(37) NOT NULL DEFAULT '',\
        FOREIGN KEY (character_uuid) REFERENCES characters(uuid) ON UPDATE CASCADE ON DELETE CASCADE\
    );", false);

    mysql_query(MySQL_GetHandle(), "CREATE TABLE IF NOT EXISTS inventory_item_attachments (\
        item_uuid VARCHAR(37) NOT NULL DEFAULT '',\
        item_state BOOLEAN NOT NULL DEFAULT 0,\
        item_attachment_bone TINYINT NOT NULL DEFAULT 1,\
        item_offset_x FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        item_offset_y FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        item_offset_z FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        item_rot_x FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        item_rot_y FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        item_rot_z FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        item_scale_x FLOAT(11, 4) NOT NULL DEFAULT 1.0000,\
        item_scale_y FLOAT(11, 4) NOT NULL DEFAULT 1.0000,\
        item_scale_z FLOAT(11, 4) NOT NULL DEFAULT 1.0000,\
        created_date TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP,\
        character_uuid VARCHAR(37) NOT NULL DEFAULT '',\
        FOREIGN KEY (character_uuid) REFERENCES characters(uuid) ON UPDATE CASCADE ON DELETE CASCADE,\
        FOREIGN KEY (item_uuid) REFERENCES inventory(item_uuid) ON UPDATE CASCADE ON DELETE CASCADE,\
        PRIMARY KEY (character_uuid, item_uuid, item_state, item_attachment_bone)\
    );");

    characterInventories = map_new();
    print("INFO: ./modules/inventory loaded");
    return 1;
}

hook OnPlayerClickTD(playerid, Text:clickedid) 
{
    if(clickedid == Text:INVALID_TEXT_DRAW)
    {
        if(GetPVarType(playerid, "TD_Inventory_Active"))
        {
            if(GetPVarType(playerid, "TD_Inventory_Select"))
            {
                Inventory_Deselect(playerid);
                Inventory_HideDescription(playerid);
                Inventory_HideButtons(playerid);
                SelectTextDraw(playerid, COLOR_ALT_HOVER);
            }
            else
            {
                Inventory_Hide(playerid);
            }
        }
    }
    return 1;
}

hook OnPlayerClickPlayerTD(playerid, PlayerText:playertextid)
{
    if(GetPVarType(playerid, "TD_Inventory_Active"))
    {
        for(new i = 0; i < MAX_INVENTORY_ITEMS_PER_PAGE; i++)
        {
            if(TD_Inventory_Items[playerid][i] != PlayerText:INVALID_TEXT_DRAW)
            {
                if(playertextid == TD_Inventory_Items[playerid][i])
                {
                    new index = (currentInventoryPage[playerid] - 1) * MAX_INVENTORY_ITEMS_PER_PAGE + i;
                    if(GetPVarType(playerid, "TD_Inventory_Select"))
                    {
                        if(GetPVarInt(playerid, "TD_Inventory_Select") == index)
                        {
                            Inventory_Deselect(playerid);
                            Inventory_HideDescription(playerid);
                            Inventory_HideButtons(playerid);
                            return Y_HOOKS_BREAK_RETURN_1;
                        }
                    }
                    Inventory_Deselect(playerid);
                    Inventory_DescribeItem(playerid, index);
                    Inventory_ShowButtons(playerid);
                    Inventory_SelectItem(playerid, index);
                    return Y_HOOKS_BREAK_RETURN_1;
                }
            }
        }   
        if(playertextid == TD_Inventory_Previous_Page[playerid])
        {
            if(currentInventoryPage[playerid] - 1 >= 1)
            {
                Inventory_SwitchPage(playerid, currentInventoryPage[playerid] - 1);
            }
            return Y_HOOKS_BREAK_RETURN_1;
        }
        else if(playertextid == TD_Inventory_Next_Page[playerid])
        {
            new List:inventory = list_new();
            if(map_get_safe(characterInventories, playerid, inventory))
            {
                new max_page = floatround(float(list_size(inventory)) / float(MAX_INVENTORY_ITEMS_PER_PAGE), floatround_ceil);
                if(currentInventoryPage[playerid] + 1 <= max_page)
                {
                    Inventory_SwitchPage(playerid, currentInventoryPage[playerid] + 1);
                }
            }
            return Y_HOOKS_BREAK_RETURN_1;
        }
        else if(playertextid == TD_Inventory_Use[playerid])
        {
            if(!GetPVarType(playerid, "TD_Inventory_Select"))
            {
                return Y_HOOKS_BREAK_RETURN_1;
            }

            new index = GetPVarInt(playerid, "TD_Inventory_Select"),
                List:inventory = list_new(),
                bool:isStateUsed[2] = {false, ...};            

            if(map_get_safe(characterInventories, playerid, inventory))
            {
                new item[E_CHARACTER_INVENTORY_DATA];
                if(list_get_arr_safe(inventory, index, item))
                {
                    if(item[item_State] == 0)
                    {
                        for(new Iter:i = list_iter(inventory); iter_inside(i); iter_move_next(i))
                        {
                            if(isStateUsed[0] == true && isStateUsed[1] == true)
                            {
                                break;
                            }

                            iter_get_arr(i, item);
                            if(item[item_State] != 0)
                            {
                                isStateUsed[item[item_State] - 1] = true;
                            }
                        }

                        new result;
                        list_get_arr(inventory, index, item);

                        task_yield(Y_HOOKS_BREAK_RETURN_1);

                        if(isStateUsed[1] == false) // right hand
                        {
                            list_set_cell_safe(inventory, index, item_State, 2);
                            result = await Inventory_AttachItem(playerid, index);

                            if(result == -1)
                            {
                                list_set_cell_safe(inventory, index, item_State, 0);
                                Player_Msg(playerid, COLOR_RED, "> ERROR: Gan vat pham that bai, co the nhan vat cua ban da het slot de gan vat pham.");
                            }
                            else
                            {
                                new query[66 + 37];
                                mysql_format(MySQL_GetHandle(), query, sizeof(query), "UPDATE inventory SET item_state = 2 WHERE ITEM_uuid = '%e' LIMIT 1", item[item_UUID]);
                                mysql_tquery(MySQL_GetHandle(), query);

                                printf("type: %d", task_await(Item_RetrieveType(item[item_Id])));
                                if(task_await(Item_RetrieveType(item[item_Id])) == ITEM_TYPE_WEAPON)
                                {
                                    printf("number of uses: %d", item[item_Number_Of_Uses]);
                                    if(item[item_Number_Of_Uses] > 0)
                                    {
                                        new weapon_id = Utils_ModelToWeapon(task_await(Item_RetrieveModel(item[item_Id])));
                                        printf("model: %d, weapon id: %d", task_await(Item_RetrieveModel(item[item_Id])), weapon_id);
                                        if(weapon_id != -1)
                                        {
                                            GivePlayerWeapon(playerid, weapon_id, item[item_Number_Of_Uses]);
                                        } 
                                    }
                                }

                                PlayerTextDrawSetString(playerid, TD_Inventory_Use[playerid], "CAT_VAO");
                                PlayerTextDrawSetString(playerid, TD_Inventory_Visible[playerid], "AN");
                            }
                        }
                        else if(isStateUsed[0] == false) // left hand
                        {
                            task_yield(Y_HOOKS_BREAK_RETURN_1);
                            list_set_cell_safe(inventory, index, item_State, 1);
                            result = await Inventory_AttachItem(playerid, index);

                            if(result == -1)
                            {
                                list_set_cell_safe(inventory, index, item_State, 0);
                                Player_Msg(playerid, COLOR_RED, "> ERROR: Gan vat pham that bai, co the nhan vat cua ban da het slot de gan vat pham.");
                            }
                            else
                            {
                                new query[66 + 37];
                                mysql_format(MySQL_GetHandle(), query, sizeof(query), "UPDATE inventory SET item_state = 1 WHERE ITEM_uuid = '%e' LIMIT 1", item[item_UUID]);
                                mysql_tquery(MySQL_GetHandle(), query);

                                PlayerTextDrawSetString(playerid, TD_Inventory_Use[playerid], "CAT_VAO");
                                PlayerTextDrawSetString(playerid, TD_Inventory_Visible[playerid], "AN");
                            }
                        }
                        else
                        {
                            Player_Msg(playerid, COLOR_RED, "> ERROR: Hai tay cua ban khong con trong.");
                        }
                    }
                    else
                    {   
                        if(item[item_State] == 2)
                        {
                            if(task_await(Item_RetrieveType(item[item_Id])) == ITEM_TYPE_WEAPON)
                            {
                                new weapon_id = Utils_ModelToWeapon(task_await(Item_RetrieveModel(item[item_Id])));
                                if(weapon_id != -1)
                                {
                                    new unused,
                                        ammo;
                                    GetPlayerWeaponData(playerid, Utils_GetWeaponSlot(weapon_id), unused, ammo);
                                    
                                    ResetPlayerWeapons(playerid);

                                    list_set_cell_safe(inventory, index, item_Number_Of_Uses, ammo);
                                }
                            }
                        }
                        
                        list_set_cell_safe(inventory, index, item_State, 0);
                        PlayerTextDrawSetString(playerid, TD_Inventory_Use[playerid], "SU_DUNG");

                        Inventory_AttachItem(playerid, index);

                        new query[66 + 37];
                        mysql_format(MySQL_GetHandle(), query, sizeof(query), "UPDATE inventory SET item_state = 0 WHERE ITEM_uuid = '%e' LIMIT 1", item[item_UUID]);
                        mysql_tquery(MySQL_GetHandle(), query);
                    }
                }
            }
            return Y_HOOKS_BREAK_RETURN_1;
        }
        else if(playertextid == TD_Inventory_Drop[playerid])
        {
            if(!GetPVarType(playerid, "TD_Inventory_Select"))
            {
                return Y_HOOKS_BREAK_RETURN_1;
            }

            new index = GetPVarInt(playerid, "TD_Inventory_Select"),
                List:inventory = list_new(),
                item[E_CHARACTER_INVENTORY_DATA];
            
            if(map_get_safe(characterInventories, playerid, inventory))
            {
                if(list_get_arr_safe(inventory, index, item))
                {
                    task_yield(Y_HOOKS_BREAK_RETURN_1);

                    if(item[item_Attachment_Index] != -1)
                    {
                        RemovePlayerAttachedObject(playerid, item[item_Attachment_Index]);
                    }

                    if(item[item_State] == 2 && task_await(Item_RetrieveType(item[item_Id])) == ITEM_TYPE_WEAPON)
                    {
                        ResetPlayerWeapons(playerid);
                    }

                    await Inventory_Remove(playerid, item[item_UUID]);
                    Inventory_SwitchPage(playerid, currentInventoryPage[playerid]);
                }
            }
            return Y_HOOKS_BREAK_RETURN_1;
        }
        else if(playertextid == TD_Inventory_Visible[playerid])
        {
            if(GetPVarType(playerid, "TD_Inventory_Select"))
            {
                new List:inventory = list_new(),
                    item[E_CHARACTER_INVENTORY_DATA],
                    index = GetPVarInt(playerid, "TD_Inventory_Select");

                if(map_get_safe(characterInventories, playerid, inventory))
                {
                    if(list_get_arr_safe(inventory, index, item))
                    {
                        if(item[item_Visible])
                        {
                            new name[64];
                            if(Inventory_DetachItem(playerid, index) == 1)
                            {   
                                PlayerTextDrawSetString(playerid, TD_Inventory_Visible[playerid], "HIEN");
                                
                                task_yield(Y_HOOKS_BREAK_RETURN_1);
                                await_arr(name) Item_RetrieveName(item[item_Id]);
                                Player_Msg(playerid, COLOR_YELLOW, sprintf("> Info: Ban da an vat pham '%s'.", name));
                            }
                            else
                            {
                                Player_Msg(playerid, COLOR_RED, "> ERROR: Vat pham dang nam ngoai tay, khong the an di.");
                            }
                        }
                        else
                        {
                            task_yield(Y_HOOKS_BREAK_RETURN_1);
                            new result = await Inventory_AttachItem(playerid, index);

                            if(result == -1)
                            {
                                Player_Msg(playerid, COLOR_RED, "> ERROR: Nhan vat cua ban khong con attachment index trong, hay an mot vai item khac.");
                            }
                            else
                            {
                                new name[64];
                                await_arr(name) Item_RetrieveName(item[item_Id]);
                                Player_Msg(playerid, COLOR_YELLOW, sprintf("> Info: Ban da gan vat pham '%s'.", name));
                                PlayerTextDrawSetString(playerid, TD_Inventory_Visible[playerid], "An");
                            }
                        }
                    }
                }
            }
            return Y_HOOKS_BREAK_RETURN_1;    
        }
        else if(playertextid == TD_Inventory_Adjust[playerid])
        {
            if(GetPVarType(playerid, "TD_Inventory_Select"))
            {
                new index = GetPVarInt(playerid, "TD_Inventory_Select"),
                    List:inventory = list_new(),
                    item[E_CHARACTER_INVENTORY_DATA];

                if(map_get_safe(characterInventories, playerid, inventory))
                {
                    if(list_get_arr_safe(inventory, index, item))
                    {
                        if(item[item_Visible] && item[item_Attachment_Index] != -1)
                        {
                            if(item[item_State] == 0)
                            {
                                Inventory_Hide(playerid);
                                ShowPlayerDialog(playerid, DIALOG_INVENTORY_ITEM_ADJUST, DIALOG_STYLE_LIST, "Inventory | Item | Adjust | Bone", "Than\nDau\nBap tay trai\nBap tay phai\nDui trai\nDui phai\nChan trai\nChan phai\nBap chan trai\nBap chan phai\nCang tay trai\nCang tay phai\nVai trai\nVai phai\nCo\nHam", "Chon", "<<");
                            }
                            else
                            {
                                Inventory_Hide(playerid);

                                new bone = item[item_State] + 4;
                                CancelSelectTextDraw(playerid);
                                
                                SetPlayerAttachedObject(playerid, item[item_Attachment_Index], task_await(Item_RetrieveModel(item[item_Id])), bone, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
                                EditAttachedObject(playerid, item[item_Attachment_Index]);
                                SetPVarInt(playerid, "TD_Inventory_Adjusting_Item", index);
                            }
                        }
                        else
                        {
                            Player_Msg(playerid, COLOR_RED, "> ERROR: Vat pham khong trong trang thai hien thi.");
                        }
                    }
                }
            }
            return Y_HOOKS_BREAK_RETURN_1;
        }
    }
    return 1;
}

hook OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    switch(dialogid)
    {
        case DIALOG_INVENTORY_ITEM_CP:
        {
            if(response)
            {

            }

            SelectTextDraw(playerid, COLOR_ALT_HOVER);
        }
        case DIALOG_INVENTORY_ITEM_ADJUST:
        {
            if(response)
            {
                if(!GetPVarType(playerid, "TD_Inventory_Select"))
                {
                    return Inventory_Show(playerid, 0);
                }

                new bone;
                if(listitem < 4)
                {
                    bone = listitem + 1;
                }
                else
                {
                    bone = listitem + 3;
                }

                new List:inventory = list_new(),
                    item[E_CHARACTER_INVENTORY_DATA],
                    index = GetPVarInt(playerid, "TD_Inventory_Select");

                if(map_get_safe(characterInventories, playerid, inventory))
                {
                    if(list_get_arr_safe(inventory, index, item))
                    {
                        if(item[item_Visible] && item[item_Attachment_Index] != -1)
                        {
                            CancelSelectTextDraw(playerid);
                            
                            SetPlayerAttachedObject(playerid, item[item_Attachment_Index], task_await(Item_RetrieveModel(item[item_Id])), bone, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
                            EditAttachedObject(playerid, item[item_Attachment_Index]);
                            SetPVarInt(playerid, "TD_Inventory_Adjusting_Item", index);
                            return 1;
                        }
                    }
                }
                
                Inventory_Show(playerid);
            }
            else
            {
                Inventory_Show(playerid);
            }
        }
    }
    return 1;
}

hook OnPlayerEditAttachedObj(playerid, response, index, modelid, boneid, Float:fOffsetX, Float:fOffsetY, Float:fOffsetZ, Float:fRotX, Float:fRotY, Float:fRotZ, Float:fScaleX, Float:fScaleY, Float:fScaleZ)
{
    if(GetPVarType(playerid, "TD_Inventory_Adjusting_Item"))
    {
        new List:inventory = list_new(),
            item[E_CHARACTER_INVENTORY_DATA],
            item_index = GetPVarInt(playerid, "TD_Inventory_Adjusting_Item");
        if(map_get_safe(characterInventories, playerid, inventory))
        {
            if(list_get_arr_safe(inventory, item_index, item))
            {
                if(response)
                {
                    SetPlayerAttachedObject(playerid, item[item_Attachment_Index], task_await(Item_RetrieveModel(item[item_Id])), boneid, fOffsetX, fOffsetY, fOffsetZ, fRotX, fRotY, fRotZ, fScaleX, fScaleY, fScaleZ);

                    new query[326 + 37 + 37 + 60],
                        uuid[UUID_LEN];
                    Character_GetUUID(playerid, uuid, sizeof(uuid));

                    mysql_format(MySQL_GetHandle(), query, sizeof(query), "UPDATE inventory_item_attachments SET item_attachment_bone = %d, item_offset_x = %.4f, item_offset_y = %.4f, item_offset_z = %.4f, item_rot_x = %.4f, item_rot_y = %.4f, item_rot_z = %.4f, item_scale_x = %.4f, item_scale_y = %.4f, item_scale_z = %.4f WHERE item_uuid = '%e' AND character_uuid = '%e' AND item_state = %d LIMIT 1", boneid, fOffsetX, fOffsetY, fOffsetZ, fRotX, fRotY, fRotZ, fScaleX, fScaleY, fScaleZ, item[item_UUID], uuid, item[item_State]);
                    mysql_tquery(MySQL_GetHandle(), query);
                }
                else
                {
                    Inventory_AttachItem(playerid, item_index);
                }
            }
        }

        DeletePVar(playerid, "TD_Inventory_Adjusting_Item");
        Inventory_Show(playerid);
        return Y_HOOKS_BREAK_RETURN_1;
    }
    return 1;
}

hook Character_OnSpawn(const playerid, const type)
{    
    switch(type)
    {
        case CHARACTER_SPAWN, CHARACTER_SWITCH:
        {
            task_yield(1);
            await Inventory_RetrieveData(playerid);

            new List:inventory = list_new();
            if(map_get_safe(characterInventories, playerid, inventory))
            {
                if(list_size(inventory) > 0)
                {
                    new item[E_CHARACTER_INVENTORY_DATA],
                        uuid[UUID_LEN];
                    Character_GetUUID(playerid, uuid, sizeof(uuid));

                    new Iter:i = list_iter(inventory),
                        count = 0;

                    pawn_guard(i);
                    for(; iter_inside(i); iter_move_next(i))
                    {
                        iter_get_arr(i, item);

                        if(item[item_State] != 0)
                        {
                            count++;
                            if(task_await(Inventory_AttachItem(playerid, iter_get_key(i))) != -1)
                            {
                                if(item[item_State] == 2)
                                {
                                    if(task_await(Item_RetrieveType(item[item_Id])) == ITEM_TYPE_WEAPON)
                                    {
                                        if(item[item_Number_Of_Uses] > 0)
                                        {
                                            new weapon_id = Utils_ModelToWeapon(task_await(Item_RetrieveModel(item[item_Id])));
                                            if(weapon_id != -1)
                                            {
                                                GivePlayerWeapon(playerid, weapon_id, item[item_Number_Of_Uses]);
                                            } 
                                        }
                                    }
                                }
                            }
                        }

                        if(count >= 2)
                        {
                            break;
                        }
                    }

                    iter_to_first(i);
                    for(; iter_inside(i); iter_move_next(i))
                    {
                        iter_get_arr(i, item);

                        if(item[item_Visible] && item[item_State] == 0)
                        {
                            if(task_await(Inventory_AttachItem(playerid, iter_get_key(i))) == -1)
                            {
                                Inventory_DetachItem(playerid, iter_get_key(i));
                            }
                        }
                    }
                }
            }
        }
    }

    Inventory_Hide(playerid);
    return 1;
}

forward Inventory_OnCharacterLogout(const playerid, const ammo[], const size);
public Inventory_OnCharacterLogout(const playerid, const ammo[], const size)
{
    Inventory_Hide(playerid);

    new List:inventory = list_new(),
        item[E_CHARACTER_INVENTORY_DATA];

    if(map_get_safe(characterInventories, playerid, inventory))
    {
        new Iter:i = list_iter(inventory);
        pawn_guard(i);
        
        for(; iter_inside(i); iter_move_next(i))
        {
            if(iter_get_arr_safe(i, item))
            {
                if(item[item_State] == 2)
                {
                    if(task_await(Item_RetrieveType(item[item_Id])) == ITEM_TYPE_WEAPON)
                    {
                        new weapon_id = Utils_ModelToWeapon(task_await(Item_RetrieveModel(item[item_Id])));
                        if(weapon_id != -1)
                        {
                            new query[80 + 37];
                            mysql_format(MySQL_GetHandle(), query, sizeof(query), "UPDATE inventory SET item_number_of_uses = %d WHERE item_uuid = '%e' LIMIT 1", ammo[Utils_GetWeaponSlot(weapon_id)], item[item_UUID]);
                            mysql_tquery(MySQL_GetHandle(), query);

                            break;
                        }
                    }
                }
            }
        }
    }

    if(map_has_key(characterInventories, playerid))
    {
        map_remove_deep(characterInventories, playerid);
    }
    return 1;
}

hook Character_OnLogout(const playerid)
{
    new weapon[13],
        ammo[13];
    for(new i = 0; i != 13; i++)
    {
        GetPlayerWeaponData(playerid, i, weapon[i], ammo[i]);
    }

    SetPreciseTimer("Inventory_OnCharacterLogout", 1, false, "iaA", playerid, ammo, 13);
    return 1;
}
#include <YSF> // for model sizes
#include <panel>

#include <config-panel-ids>
#include <config-dialog-ids>

#include <house>

enum _:E_FURNITURE_DATA
{
    furn_House_UUID[UUID_LEN],
    furn_UUID[UUID_LEN],
    furn_Object_Id
}

new Map:houseFurnitures;

stock Furniture_Add(const house_index, const model, const Float:x, const Float:y, const Float:z, const Float:rx, const Float:ry, const Float:rz, const world, const interior)
{
    new furniture[E_FURNITURE_DATA],
        List:furnitures = list_new();
    House_GetUUID(house_index, furniture[furn_House_UUID], UUID_LEN);
    UUID(furniture[furn_UUID], UUID_LEN);
    furniture[furn_Object_Id] = CreateDynamicObject(model, x, y, z, rx, ry, rz, world, interior, _, 200.0, 200.0);

    new index = -1;
    if(map_has_key(houseFurnitures, house_index))
    {
        if(map_get_safe(houseFurnitures, house_index, furnitures))
        {
            index = list_add_arr(furnitures, furniture);
        }
    }
    else
    {
        index = list_add_arr(furnitures, furniture);
        map_set(houseFurnitures, house_index, furnitures);
    }

    new query[181 + 37 + 37 + 90];
    mysql_format(MySQL_GetHandle(), query, sizeof(query), "INSERT INTO house_furnitures (uuid, house_uuid, model, pos_x, pos_y, pos_z, rot_x, rot_y, rot_z, world, interior) VALUES ('%e', '%e', %d, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %d, %d)", furniture[furn_UUID], furniture[furn_House_UUID], model, x, y, z, rx, ry, rz, world, interior);
    mysql_tquery(MySQL_GetHandle(), query);
    return index;
}

stock bool:Furniture_Remove(const house_index, const furn_index)
{
    new List:furnitures = list_new();
    if(map_get_safe(houseFurnitures, house_index, furnitures))
    {
        if(furn_index < 0 || furn_index >= list_size(furnitures))
        {
            return false;
        }

        new furniture[E_FURNITURE_DATA];
        if(list_get_arr_safe(furnitures, furn_index, furniture))
        {
            new query[54 + 40];
            mysql_format(MySQL_GetHandle(), query, sizeof(query), "DELETE FROM house_furnitures WHERE uuid = '%e' LIMIT 1", furniture[furn_UUID]);
            mysql_tquery(MySQL_GetHandle(), query);
            
            DestroyDynamicObject(furniture[furn_Object_Id]);
            list_remove_deep(furnitures, furn_index);

            return true;
        }
    }
    return false;
}

stock bool:Furniture_ShowPanel(const house_index, const forplayerid)
{
    new List:furnitures = list_new();
    if(map_get_safe(houseFurnitures, house_index, furnitures))
    {
        new furniture[E_FURNITURE_DATA],
            models[MAX_PANEL_ITEMS],
            count = 0;

        for(new Iter:i = list_iter(furnitures, 0); iter_inside(i); iter_move_next(i))
        {
            if(iter_get_arr_safe(i, furniture))
            {
                models[count] = Streamer_GetIntData(STREAMER_TYPE_OBJECT, furniture[furn_Object_Id], E_STREAMER_MODEL_ID);
                count++;

                if(count >= MAX_PANEL_ITEMS)
                {
                    break;
                }
            }
        }
        Panel_Show(forplayerid, FURNITURE_PANEL_ID, "Furnitures", models, count, floatround(float(list_size(furnitures)) / float(MAX_PANEL_ITEMS), floatround_ceil));
        SelectTextDraw(forplayerid, COLOR_ALT_HOVER);
        SetPVarInt(forplayerid, "furniturePanel_House_Index", house_index);
        return true;
    }
    return false;
}

#include <YSI_Coding\y_hooks>

hook OnGameModeInit()
{
    mysql_query(MySQL_GetHandle(), "CREATE TABLE IF NOT EXISTS house_furnitures (\
        uuid VARCHAR(37) NOT NULL PRIMARY KEY DEFAULT '',\
        model INT NOT NULL DEFAULT 0,\
        pos_x FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        pos_y FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        pos_z FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        rot_x FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        rot_y FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        rot_z FLOAT(11, 4) NOT NULL DEFAULT 0.0000,\
        world INT NOT NULL DEFAULT 0,\
        interior INT NOT NULL DEFAULT 0,\
        created_date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,\
        house_uuid VARCHAR(37) NULL,\
        FOREIGN KEY (house_uuid) REFERENCES houses(uuid) ON UPDATE CASCADE ON DELETE CASCADE\
    );");

    houseFurnitures = map_new();
    print("INFO: ./modules/house-furniture loaded");
    return 1;
}

hook House_OnInitiated(index)
{
    new uuid[UUID_LEN],
        List:furnitures = list_new();
    House_GetUUID(index, uuid, sizeof(uuid));

    task_yield(1);
    await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT * FROM house_furnitures WHERE house_uuid = '%e'", uuid));

    new rows = cache_num_rows();
    if(rows > 0)
    {
        new furniture[E_FURNITURE_DATA],
            model,
            world,
            interior,
            Float:pos[3],
            Float:rot[3];
        for(new i = 0; i < rows; i++)
        {
            cache_get_value_name(i, "uuid", furniture[furn_UUID], UUID_LEN);
            cache_get_value_name(i, "house_uuid", furniture[furn_House_UUID], UUID_LEN);
            
            cache_get_value_name_int(i, "model", model);
            cache_get_value_name_int(i, "world", world);
            cache_get_value_name_int(i, "interior", interior);
            
            cache_get_value_name_float(i, "pos_x", pos[0]);
            cache_get_value_name_float(i, "pos_y", pos[1]);
            cache_get_value_name_float(i, "pos_z", pos[2]);
            cache_get_value_name_float(i, "rot_x", rot[0]);
            cache_get_value_name_float(i, "rot_y", rot[1]);
            cache_get_value_name_float(i, "rot_z", rot[2]);

            furniture[furn_Object_Id] = CreateDynamicObject(model, pos[0], pos[1], pos[2], rot[0], rot[1], rot[2], world, interior, _, 200.0, 200.0);

            list_add_arr(furnitures, furniture);
            printf("house %d, model %d, object_id %d, collision sphere radius %.4f", index, model, furniture[furn_Object_Id], GetColSphereRadius(model));
        }
    }

    map_set(houseFurnitures, index, furnitures);
    return 1;
}

hook Panel_OnSelect(playerid, panel_id, item_id)
{
    switch(panel_id)
    {
        case FURNITURE_PANEL_ID:
        {
            switch(item_id)
            {
                case PREVIOUS_BUTTON_ITEM:
                {
                    if(GetPVarType(playerid, "furniturePanel_House_Index"))
                    {
                        new houseid = GetPVarInt(playerid, "furniturePanel_House_Index"),
                            List:furnitures = list_new();
                        if(map_get_safe(houseFurnitures, houseid, furnitures))
                        {
                            new current_page = Panel_GetCurrentPage(playerid);
                            if(current_page > 1)
                            {
                                new models[MAX_PANEL_ITEMS],
                                    furniture[E_FURNITURE_DATA],
                                    count = 0;
                                for(new Iter:i = list_iter(furnitures, (current_page - 2) * MAX_PANEL_ITEMS); iter_inside(i); iter_move_next(i))
                                {
                                    if(iter_get_arr_safe(i, furniture))
                                    {
                                        models[count] = Streamer_GetIntData(STREAMER_TYPE_OBJECT, furniture[furn_Object_Id], E_STREAMER_MODEL_ID);
                                        count++;

                                        if(count >= MAX_PANEL_ITEMS)
                                        {
                                            break;
                                        }
                                    }
                                }
                                Panel_PreviousPage(playerid, models, count);
                            }
                        }
                    }
                }
                case NEXT_BUTTON_ITEM:
                {
                    if(GetPVarType(playerid, "furniturePanel_House_Index"))
                    {
                        new houseid = GetPVarInt(playerid, "furniturePanel_House_Index"),
                            List:furnitures = list_new();

                        if(map_get_safe(houseFurnitures, houseid, furnitures))
                        {
                            new current_page = Panel_GetCurrentPage(playerid);
                            if(current_page < Panel_GetMaxPage(playerid))
                            {
                                new models[MAX_PANEL_ITEMS],
                                    furniture[E_FURNITURE_DATA],
                                    count = 0;
                                for(new Iter:i = list_iter(furnitures, (current_page) * MAX_PANEL_ITEMS); iter_inside(i); iter_move_next(i))
                                {
                                    if(iter_get_arr_safe(i, furniture))
                                    {
                                        models[count] = Streamer_GetIntData(STREAMER_TYPE_OBJECT, furniture[furn_Object_Id], E_STREAMER_MODEL_ID);
                                        count++;

                                        if(count >= MAX_PANEL_ITEMS)
                                        {
                                            break;
                                        }
                                    }
                                }
                                Panel_NextPage(playerid, models, count);
                            }
                        }
                    }
                }
                default:
                {
                    Panel_Destroy(playerid);
                    CancelSelectTextDraw(playerid);
                    ShowPlayerDialog(playerid, DIALOG_FURNITURE_PANEL, DIALOG_STYLE_LIST, "Furniture Panel", "Hien thi vi tri\nChinh sua vi tri\nXoa noi that", "Chon", "Tat");
                    SetPVarInt(playerid, "furniturePanel_Item_Index", item_id);
                }
            }
        }
    }
    return 1;
}

hook Panel_OnDestroy(playerid, panel_id)
{
    switch(panel_id)
    {
        case FURNITURE_PANEL_ID:
        {
            DeletePVar(playerid, "furniturePanel_House_Index");
        }
    }
    return 1;
}

hook OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
    switch(dialogid)
    {
        case DIALOG_FURNITURE_PANEL:
        {
            if(GetPVarType(playerid, "furniturePanel_Item_Index") && GetPVarType(playerid, "furniturePanel_House_Index"))
            {
                if(response)
                {
                    new item_index = GetPVarInt(playerid, "furniturePanel_Item_Index"),
                        house_index = GetPVarInt(playerid, "furniturePanel_House_Index");
                    switch(listitem)
                    {
                        case 0: // Hien thi vi tri
                        {
                            new List:furnitures = list_new();
                            if(map_get_safe(houseFurnitures, house_index, furnitures))
                            {
                                new furniture[E_FURNITURE_DATA];
                                if(list_get_arr_safe(furnitures, item_index, furniture))
                                {
                                    new Float:pos[3];
                                    GetDynamicObjectPos(furniture[furn_Object_Id], pos[0], pos[1], pos[2]);
                                    SetPlayerCheckpoint(playerid, pos[0], pos[1], pos[2], 2.0);
                                }
                            }
                            DeletePVar(playerid, "furniturePanel_House_Index");
                        }
                        case 1: // Chinh sua vi tri
                        {
                            new List:furnitures = list_new();
                            if(map_get_safe(houseFurnitures, house_index, furnitures))
                            {
                                new furniture[E_FURNITURE_DATA];
                                if(list_get_arr_safe(furnitures, item_index, furniture))
                                {
                                    EditDynamicObject(playerid, furniture[furn_Object_Id]);
                                }
                            }
                        }
                        case 2: // Xoa noi that
                        {
                            Furniture_Remove(house_index, item_index);
                            if(GetPVarType(playerid, "furniturePanel_House_Index"))
                            {
                                new cmd[20];
                                format(cmd, sizeof(cmd), "/furniture %d", GetPVarInt(playerid, "furniturePanel_House_Index"));
                                PC_EmulateCommand(playerid, cmd);
                            }
                        }
                    }
                }
                else
                {
                    if(GetPVarType(playerid, "furniturePanel_House_Index"))
                    {
                        new cmd[20];
                        format(cmd, sizeof(cmd), "/furniture %d", GetPVarInt(playerid, "furniturePanel_House_Index"));
                        PC_EmulateCommand(playerid, cmd);
                    }
                    DeletePVar(playerid, "furniturePanel_Item_Index");
                }
            } 
            return Y_HOOKS_BREAK_RETURN_1;
        }
    }
    return 1;
}

hook OnPlayerEditDynObject(playerid, objectid, response, Float:x, Float:y, Float:z, Float:rx, Float:ry, Float:rz)
{
    if(GetPVarType(playerid, "furniturePanel_Item_Index"))
    {
        switch(response)
        {
            case EDIT_RESPONSE_FINAL:
            {
                new item_index = GetPVarInt(playerid, "furniturePanel_Item_Index"),
                    house_index = GetPVarInt(playerid, "furniturePanel_House_Index"),
                    List:furnitures = list_new();
                if(map_get_safe(houseFurnitures, house_index, furnitures))
                {
                    new furniture[E_FURNITURE_DATA];
                    if(list_get_arr_safe(furnitures, item_index, furniture))
                    {
                        if(furniture[furn_Object_Id] != objectid)
                        {
                            goto furniture_cancel_edit;
                        }
                        
                        new query[136 + 77 + 37];
                        mysql_format(MySQL_GetHandle(), query, sizeof(query), "UPDATE house_furnitures SET pos_x = %.4f, pos_y = %.4f, pos_z = %.4f, rot_x = %.4f, rot_y = %.4f, rot_z = %.4f WHERE uuid = '%e' LIMIT 1", x, y, z, rx, ry, rz, furniture[furn_UUID]);
                        mysql_tquery(MySQL_GetHandle(), query);
                    }
                }
                ShowPlayerDialog(playerid, DIALOG_FURNITURE_PANEL, DIALOG_STYLE_LIST, "Furniture Panel", "Hien thi vi tri\nChinh sua vi tri\nXoa noi that", "Chon", "Tat");
            }
            case EDIT_RESPONSE_CANCEL:
            {
                furniture_cancel_edit:
                new item_index = GetPVarInt(playerid, "furniturePanel_Item_Index"),
                    house_index = GetPVarInt(playerid, "furniturePanel_House_Index"),
                    List:furnitures = list_new();
                if(map_get_safe(houseFurnitures, house_index, furnitures))
                {
                    new furniture[E_FURNITURE_DATA];
                    if(list_get_arr_safe(furnitures, item_index, furniture))
                    {
                        new Float:old_pos[3],
                            Float:old_rot[3];
                        Streamer_GetFloatData(STREAMER_TYPE_OBJECT, furniture[furn_Object_Id], E_STREAMER_X, old_pos[0]);
                        Streamer_GetFloatData(STREAMER_TYPE_OBJECT, furniture[furn_Object_Id], E_STREAMER_Y, old_pos[1]);
                        Streamer_GetFloatData(STREAMER_TYPE_OBJECT, furniture[furn_Object_Id], E_STREAMER_Z, old_pos[2]);
                        Streamer_GetFloatData(STREAMER_TYPE_OBJECT, furniture[furn_Object_Id], E_STREAMER_R_X, old_rot[0]);
                        Streamer_GetFloatData(STREAMER_TYPE_OBJECT, furniture[furn_Object_Id], E_STREAMER_R_Y, old_rot[1]);
                        Streamer_GetFloatData(STREAMER_TYPE_OBJECT, furniture[furn_Object_Id], E_STREAMER_R_Z, old_rot[2]);

                        SetDynamicObjectPos(furniture[furn_Object_Id], old_pos[0], old_pos[1], old_pos[2]);
                        SetDynamicObjectRot(furniture[furn_Object_Id], old_rot[0], old_rot[1], old_rot[2]);
                    } 
                }
                ShowPlayerDialog(playerid, DIALOG_FURNITURE_PANEL, DIALOG_STYLE_LIST, "Furniture Panel", "Hien thi vi tri\nChinh sua vi tri\nXoa noi that", "Chon", "Tat");
            }
        }
        return Y_HOOKS_BREAK_RETURN_1;
    }
    return 1;
}
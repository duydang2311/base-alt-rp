static enum _:E_HOUSE_PORTAL_DATA
{
    hpotl_Id, // index of the portal in portal list
    hpotl_House_UUID
}

static List:housePortals;

stock Task:HousePortal_Create(const house_uuid[], const size = UUID_LEN)
{
    new Task:t = task_new();
    task_detach();
    task_yield(t);

    await mysql_aquery_s(MySQL_GetHandle(), str_format("SELECT COUNT(*) FROM houses WHERE uuid = '%e' LIMIT 1", house_uuid));

    {
        new rows;
        cache_get_value_name_int(0, "COUNT(*)", rows);

        if(rows == 0)
        {
            task_set_result(t, -1);
            return Task:0;
        }
    }

    new index = Portal_Create(),
        house_portal[E_HOUSE_PORTAL_DATA],
        uuid[UUID_LEN];

    Portal_SetType(index, PORTAL_TYPE_HOUSE);

    Portal_GetUUID(index, uuid, sizeof(uuid));

    house_portal[hpotl_Id] = index;
    strcopy(house_portal[hpotl_House_UUID], house_uuid, size);

    new query[71 + 37 + 37 + 10];
    mysql_format(MySQL_GetHandle(), query, sizeof(query), "INSERT INTO house_portals (portal_uuid, house_uuid) VALUES ('%e', '%e')", uuid, house_uuid);
    mysql_tquery(MySQL_GetHandle(), query);

    list_add_arr(housePortals, house_portal);

    task_set_result(t, index);
    return Task:0;
}

stock HousePortal_Destroy(const index)
{
    new house_portal[E_HOUSE_PORTAL_DATA];
    for(new Iter:i = list_iter(housePortals); iter_inside(i); iter_move_next(i))
    {
        if(iter_get_arr_safe(i, house_portal))
        {
            if(house_portal[hpotl_Id] == index)
            {
                iter_erase_deep(i);
                return 1;
            }
        }
    }
    return 0;
}

#include <YSI_Coding\y_hooks>

hook OnGameModeInit()
{
    mysql_query(MySQL_GetHandle(), "CREATE TABLE IF NOT EXISTS house_portals (\
        portal_uuid VARCHAR(37) NOT NULL PRIMARY KEY DEFAULT '',\
        house_uuid VARCHAR(37) NOT NULL DEFAULT '',\
        created_date TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP,\
        FOREIGN KEY (portal_uuid) REFERENCES portals(uuid) ON UPDATE CASCADE ON DELETE CASCADE,\
        FOREIGN KEY (house_uuid) REFERENCES houses(uuid) ON UPDATE CASCADE ON DELETE CASCADE\
    );", false);

    housePortals = list_new();
    return 1;
}
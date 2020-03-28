#include <YSI_Coding\y_hooks>

hook OnGameModeInit()
{
    mysql_query(MySQL_GetHandle(), "CREATE TABLE IF NOT EXISTS houses (\
        uuid VARCHAR(37) NOT NULL PRIMARY KEY DEFAULT '',\
        owner_uuid VARCHAR(37) NOT NULL DEFAULT '',\
        created_date TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP,\
        FOREIGN KEY (owner_uuid) REFERENCES characters(uuid) ON UPDATE CASCADE ON DELETE CASCADE\
    );", false);
    return 1;
}
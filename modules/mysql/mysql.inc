#if defined _module_mysql_included
    #endinput
#endif

#define _module_mysql_included

#include <a_mysql>

static MySQL:mysql_handle;

stock MySQL:MySQL_GetHandle()
{
    return MySQL:mysql_handle;
}

public OnQueryError(errorid, const error[], const callback[], const query[], MySQL:handle)
{
    print("ERROR: OnQueryError");
    printf("     > errorid: %d", errorid);
    printf("     > error: %s", error);
    printf("     > callback: %s", callback);
    printf("     > query: %s", query);
    printf("     > handle: %d", _:handle);
    return 1;
}

#include <YSI_Coding\y_hooks>

hook OnGameModeInit()
{    
    mysql_handle = mysql_connect("localhost", "root", "123456", "alt-rp");
    if(mysql_errno(mysql_handle) != 0 || mysql_handle == MYSQL_INVALID_HANDLE)
    {
        print("ERROR: Failed to initiate MySQL connection");
        SendRconCommand("exit");
        return 0;
    }
    print("INFO: MySQL connection initiated succesfully");

    print("INFO: ./modules/mysql loaded");
    return 1;
}

hook OnGameModeExit()
{
    mysql_close(mysql_handle);
    print("INFO: MySQL connection terminated");
    return 1;
}
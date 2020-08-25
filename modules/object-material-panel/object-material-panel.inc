#if defined _included_obj_material_panel
    #endinput
#endif

#define _included_obj_material_panel

#include <config-panel-ids>

#include <panel>

stock MaterialPanel_Show(const playerid, const mpanel_id)
{
    
    return 1;
}

#include <YSI_Coding\y_hooks>

hook OnGameModeInit()
{
    print("INFO: ./modules/object-material-panel loaded");
    return 1;
}
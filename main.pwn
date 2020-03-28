#define PP_SYNTAX_AWAIT
#define YSI_NO_HEAP_MALLOC

#include <a_samp>

const MAX_CHARACTERS_PER_ACCOUNT = 3;

#include <mysql>
#include <player>
#include <account>
#include <character>
#include <item>
#include <player-cmd>
#include <vehicle>
#include <portal>
#include <house>

main() {}

#include <YSI_Coding\y_hooks>

hook OnGameModeInit()
{
    ManualVehicleEngineAndLights();
	ShowPlayerMarkers(0);
	DisableNameTagLOS();
	DisableInteriorEnterExits();
	AllowInteriorWeapons(1);
	EnableStuntBonusForAll(0);
	EnableVehicleFriendlyFire();
    return 1;
}
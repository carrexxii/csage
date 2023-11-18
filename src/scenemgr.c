#include "scenes.h"
#include "scenemgr.h"

static enum SceneName current_scene;
// static void (*scenes[])(void) = {
// 	[SCENE_MENU]           = scene_menu,
// 	[SCENE_GAME]           = scene_game,
// 	[SCENE_EDITOR]         = scene_editor,
// 	[SCENE_MENUBACKGROUND] = scene_menubackground,
// };

void scenemgr_init()
{
	current_scene = SCENE_GAME;
}

void scenemgr_loop()
{

}

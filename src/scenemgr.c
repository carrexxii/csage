#include "taskmgr.h"
#include "input.h"
#include "camera.h"
#include "gfx/particles.h"
#include "entities/entity.h"
#include "gfx/model.h"
#include "gfx/renderer.h"
#include "gfx/ui/ui.h"
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
	taskmgr_init();
	taskmgr_add_task(camera_update);
	taskmgr_add_task(ui_update);
	taskmgr_add_task(particles_update);
	taskmgr_add_task(entities_update);
	taskmgr_add_task(models_update); // TODO: Change the animation to a separate thing

	current_scene = SCENE_GAME;
}

noreturn void scenemgr_loop()
{
	DEBUG(1, "\nBeginning main loop (load time: %lums)\n"
	           "--------------------------------------", SDL_GetTicks64());
	uint64 dt, nt, ot = 0.0, acc = 0.0;
	while (1) {
		input_poll();
		nt = SDL_GetTicks64();
		dt = nt - ot;
		ot = nt;
		acc += dt;
		while (acc >= DT_MS) {
			while (!taskmgr_reset());
			acc -= DT_MS;
		}
		renderer_draw();
	}
}

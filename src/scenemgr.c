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
	ui_build();
	
	taskmgr_init();
	taskmgr_add_task(camera_update);
	taskmgr_add_task(ui_update);
	taskmgr_add_task(particles_update);
	taskmgr_add_task(entities_update);
	taskmgr_add_task(models_update); // TODO: Change the animation to a separate thing

	global_light.ambient[0] = 1.0f;
	global_light.ambient[1] = 1.0f;
	global_light.ambient[2] = 1.0f;
	global_light.ambient[3] = 0.02f;
	global_light.pos[0] = 100.0f;
	global_light.pos[1] = 0.0f;
	global_light.pos[2] = 0.0f;
	global_light.pos[3] = 2.0f;
	global_light.colour[0] = 1.0f;
	global_light.colour[1] = 0.2f;
	global_light.colour[2] = 0.2f;

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
			// global_light.pos[0] = fmod((global_light.pos[0] + 0.1), 20.0f);
			// global_light.pos[1] = fmod((global_light.pos[1] + 0.1), 20.0f);
			// global_light.pos[2] = -fmod((global_light.pos[2] + 0.1), 20.0f);
			// global_light.ambient[3] = fmod((global_light.ambient[3] + 0.01), 1.0f);
			while (!taskmgr_reset());
			acc -= DT_MS;
		}
		renderer_draw();
	}
}

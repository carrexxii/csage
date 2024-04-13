#ifndef SCENEMGR_H
#define SCENEMGR_H

#include "SDL3/SDL.h"

#include "common.h"

typedef enum SceneType {
	SCENE_NONE,
	SCENE_MAINMENU,
	SCENE_MENU,
	SCENE_GAME,
	SCENE_EDITOR,
	SCENE_SCRATCH,
} SceneType;

typedef struct Level {
	String name;
} Level;

void scenemgr_init(SDL_Window* window);
[[noreturn]] void scenemgr_loop(void);
void scenemgr_defer(void (*fn)(void*), void* data);
void scenemgr_free(void);

#endif


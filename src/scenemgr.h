#ifndef SCENEMGR_H
#define SCENEMGR_H

#include "util/string.h"

enum SceneType {
	SCENE_NONE,
	SCENE_MAINMENU,
	SCENE_MENU,
	SCENE_GAME,
	SCENE_EDITOR,
	SCENE_SCRATCH,
};

struct Level {
	String name;
};

void scenemgr_init(void);
noreturn void scenemgr_loop(void);
void scenemgr_defer(void (*fn)(void*), void* data);
void scenemgr_free(void);

#endif

#ifndef SCENEMGR_H
#define SCENEMGR_H

enum SceneName {
	SCENE_NONE,
	SCENE_MAINMENU,
	SCENE_MENU,
	SCENE_GAME,
	SCENE_EDITOR,
};

enum SceneType {
	SCENETYPE_NONE,
	SCENETYPE_COMPLETE,
	SCENETYPE_OVERLAY,
};

struct Scene {
	enum SceneType type;
	void (*update)(void);
	void (*draw)(void);
};

void scenemgr_init();
noreturn void scenemgr_loop();

#endif

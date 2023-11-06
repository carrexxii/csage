#ifndef SCENEMGR_H
#define SCENEMGR_H

enum SceneName {
	SCENE_MENU,
	SCENE_GAME,
	SCENE_EDITOR,
	SCENE_MENUBACKGROUND,
};

enum SceneType {
	SCENETYPE_NONE,
	SCENETYPE_COMPLETE,
	SCENETYPE_OVERLAY,
};

struct Scene {
	void (*update)(void);
	void (*draw)(void);
	enum SceneType type;
};

void scenemgr_init();
void scenemgr_loop();

#endif

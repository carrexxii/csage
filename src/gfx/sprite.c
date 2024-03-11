#include "lua.h"
#include "sprite.h"

struct Sprite* sprite_new(char* name)
{
	char buf[256];
	// sprintf(buf, "%s.png", name);
	sprintf(buf, SPRITE_PATH "/%s.lua", name);
	lua_getglobal(lua_state, "load_sprite");
	lua_pushstring(lua_state, buf);
	if (lua_pcall(lua_state, 1, 1, 0))
		ERROR("[LUA] Failed in call to \"load_sprite\": \n\t%s", lua_tostring(lua_state, -1));

	if (lua_isnoneornil(lua_state, -1))
		ERROR("[RES] Failed to load \"%s\" (%s)", name, buf);
	struct Sprite* sprite = *(void**)lua_topointer(lua_state, -1);
	lua_pop(lua_state, 1);

	return sprite;
}

struct Sprite* sprite_load(char* name, isize animc, isize* framecs, Recti* frames)
{
	DEBUG(3, "[RES] Loading new sprite \"%s\" with %ld animations:", name, animc);

	char buf[PATH_BUFFER_SIZE];
	snprintf(buf, PATH_BUFFER_SIZE, SPRITE_PATH "/sheets/%s.png", name);
	struct Sprite* sprite = smalloc(sizeof(struct Sprite) + animc*sizeof(struct Animation));
	*sprite = (struct Sprite){
		.animc = animc,
		.sheet = texture_new_from_image(buf),
	};

	for (int a = 0; a < animc; a++) {
		DEBUG(3, "\tAnimation %d with %ld frames", a, framecs[a]);
		sprite->anims[a].framec = framecs[a];
		sprite->anims[a].frames = smalloc(framecs[a]*sizeof(struct Frame));
		for (int f = 0; f < framecs[a]; f++) {
			sprite->anims[a].frames[f] = (struct Frame){
				.duration = 0,
				.x = frames[f].x,
				.y = frames[f].y,
				.w = frames[f].w,
				.h = frames[f].h,
			};
		}
	}

	return sprite;
}

void sprite_free(struct Sprite* sprite)
{
	for (int a = 0; a < sprite->animc; a++) {
		sprite->anims[a].framec = 0;
		free(sprite->anims[a].frames);
	}

	texture_free(&sprite->sheet);
	sprite->animc = 0;
}

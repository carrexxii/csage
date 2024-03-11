#ifndef GFX_SPRITE_H
#define GFX_SPRITE_H

#include "maths/types.h"
#include "texture.h"

struct Frame {
	int16 x, y, w, h;
	int16 duration;
};

struct Animation {
	isize framec;
	struct Frame* frames;
};

struct Sprite {
	isize time;
	isize animc;
	struct Texture sheet;
	struct Animation anims[];
};

struct Sprite* sprite_new(char* name);
struct Sprite* sprite_load(char* name, isize animc, isize* framecs, Recti* frames);
void sprite_free(struct Sprite* sprite);

#endif

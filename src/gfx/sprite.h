#ifndef GFX_SPRITE_H
#define GFX_SPRITE_H

#include "maths/types.h"
#include "util/string.h"
#include "util/varray.h"
#include "pipeline.h"
#include "texture.h"
#include "camera.h"

#define MAX_SPRITE_SHEETS     (INT8_MAX + 1)
#define MAX_SPRITE_GROUPS     (INT8_MAX + 1)
#define MAX_SPRITE_FRAMES     (INT8_MAX + 1)
#define DEFAULT_SPRITE_SHEETS 8
#define DEFAULT_SPRITES       16

#define STRING_OF_SPRITE_ANIMATION(x) (    \
	x == SPRITE_NONE   ? "SPRITE_NONE"   : \
	x == SPRITE_WALK   ? "SPRITE_WALK"   : \
	x == SPRITE_RUN    ? "SPRITE_RUN"    : \
	x == SPRITE_ATTACK1? "SPRITE_ATTACK1": \
	x == SPRITE_ATTACK2? "SPRITE_ATTACK2": \
	"<Unknown animation>")
enum SpriteAnimation {
	SPRITE_NONE,
	SPRITE_WALK,
	SPRITE_RUN,
	SPRITE_ATTACK1,
	SPRITE_ATTACK2,
};

struct SpriteFrame {
	uint16 x, y, w, h;
};

struct SpriteState {
	enum SpriteAnimation type;
	int framec;
	struct SpriteFrame* frames;
};

struct SpriteGroup {
	char name[32];
	int statec;
	struct SpriteState* states;
};

struct Sprite {
	Vec2 pos;
	int  id;
	float pad1;
};

struct SpriteSheet {
	char name[32];
	int w, h;
	int groupc;
	struct VArray       sprites;
	struct SpriteGroup* groups;
	struct Texture*     tex;
	struct Pipeline pipeln;
	SBO sprite_sheet_data;
	SBO sprite_data;
};

void   sprites_record_commands(VkCommandBuffer cmd_buf);
void   sprites_free(void);
int    sprite_sheet_new(char* name);
uint64 sprite_new(char* restrict sheet_name, char* restrict group_name, Vec2 pos);
void   sprite_destroy(int sprite);

#endif

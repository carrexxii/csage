#ifndef GFX_SPRITE_H
#define GFX_SPRITE_H

#include "maths/types.h"
#include "util/string.h"
#include "util/varray.h"
#include "pipeline.h"
#include "texture.h"
#include "camera.h"

/* Using a hash map for many different animations + directions might be good */

#define MAX_SPRITE_SHEETS     (INT8_MAX + 1)
#define MAX_SPRITE_GROUPS     (INT8_MAX + 1)
#define MAX_SPRITE_FRAMES     (INT8_MAX + 1)
#define DEFAULT_SPRITE_SHEETS 8
#define DEFAULT_SPRITES       16

// TODO: this needs to be a hashmap
enum SpriteStateType {
	SPRITE_NONE,
	SPRITE_IDLE,
	SPRITE_WALK,
	SPRITE_RUN,
	SPRITE_ATTACK1,
	SPRITE_ATTACK2,

	SPRITE_GRASS,
	SPRITE_DIRT,

	SPRITE_LAST,
};
static const char* string_of_animation_state[] = {
	[SPRITE_NONE]    = PPSTR(SPRITE_NONE),
	[SPRITE_IDLE]    = PPSTR(SPRITE_IDLE),
	[SPRITE_WALK]    = PPSTR(SPRITE_WALK),
	[SPRITE_RUN]     = PPSTR(SPRITE_RUN),
	[SPRITE_ATTACK1] = PPSTR(SPRITE_ATTACK1),
	[SPRITE_ATTACK2] = PPSTR(SPRITE_ATTACK2),

	[SPRITE_GRASS] = PPSTR(SPRITE_GRASS),
	[SPRITE_DIRT]  = PPSTR(SPRITE_DIRT),
};

struct SpriteFrame {
	uint16 x, y, w, h;
};

struct SpriteState {
	enum SpriteStateType type;
	enum Direction       dir;
	int duration;
	int gi;
	int framec;
	struct SpriteFrame* frames;
};

struct SpriteGroup {
	char name[32]; // Remove?
	int statec;
	struct SpriteState* states;
};

struct Sprite {
	Vec3   pos;
	int16  gi;
	int8   state, frame;
	int8   sheet, group;
	uint16 time;
	byte pad[12];
};

struct SpriteSheet {
	char name[32];
	int w, h, z;
	int groupc;
	bool needs_update;
	struct VArray       sprites;
	struct SpriteGroup* groups;
	struct Texture*     albedo;
	struct Texture*     normal;
	struct Pipeline pipeln;
	SBO sprite_sheet_data;
	SBO sprite_data;
};

void sprites_init(void);
int  sprites_get_sheet(char* sheet_name);
int  sprites_get_group(int sheet_id, char* group_name);
void sprites_record_commands(VkCommandBuffer cmd_buf);
void sprites_update(void);
void sprite_sheet_free(void);
void sprite_sheet_init_pipeline(struct SpriteSheet* sheet);
int  sprite_sheet_new(char* name, int z_lvl);
int  sprite_sheet_load(struct SpriteSheet* sheet_data);
struct Sprite* sprite_new(int sheet_id, int group_id, Vec3 pos);
struct Sprite* sprite_new_batch(int sheet_id, int group_id, int spritec, Vec3* poss, enum SpriteStateType* states);
void sprite_set_state(struct Sprite* sprite, enum SpriteStateType type, enum Direction dir);

#endif

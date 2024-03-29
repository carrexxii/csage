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
#define SPRITE_SCALE          45

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
struct SpriteSheet* sprites_get_sheet(const char* sheet_name);
int  sprites_get_sheet_id(struct SpriteSheet* sheet);
int  sprites_get_group(struct SpriteSheet* sheet, const char* group_name);
void sprites_record_commands(VkCommandBuffer cmd_buf);
void sprites_update(void);
void sprite_sheet_free(void);
void sprite_sheet_init_pipeline(struct SpriteSheet* sheet);
struct SpriteSheet* sprite_sheet_new(const char* name, int z_lvl);
struct SpriteSheet* sprite_sheet_load(struct SpriteSheet* sheet_data);
struct Sprite* sprite_new(struct SpriteSheet* sheet, int group_id, Vec3 pos);
struct Sprite* sprite_new_batch(struct SpriteSheet* sheet, int group_id, int spritec, Vec3* poss, enum SpriteStateType* states);
struct Sprite* sprite_new_by_gi(struct SpriteSheet* sheet, int gi, Vec3 pos);
void sprite_set_state(struct Sprite* sprite, enum SpriteStateType type, enum Direction dir);

#endif

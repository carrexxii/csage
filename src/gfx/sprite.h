#ifndef GFX_SPRITE_H
#define GFX_SPRITE_H

#include "maths/types.h"
#include "util/string.h"
#include "util/varray.h"
#include "image.h"
#include "pipeline.h"
#include "camera.h"

/* Using a hash map for many different animations + directions might be good */

#define MAX_SPRITE_SHEETS     (INT8_MAX + 1)
#define MAX_SPRITE_GROUPS     (INT8_MAX + 1)
#define MAX_SPRITE_FRAMES     (INT8_MAX + 1)
#define DEFAULT_SPRITE_SHEETS 8
#define DEFAULT_SPRITES       16
#define SPRITE_SCALE          45

// TODO: this needs to be a hashmap
#define STRING_OF_ANIMATION_STATE(x) ((x) < SPRITE_STATE_MAX? string_of_animation_state[x]: "<Unknown>")
enum SpriteStateType {
	SPRITE_STATE_IDLE,
	SPRITE_STATE_WALK,
	SPRITE_STATE_RUN,
	SPRITE_STATE_ATTACK1,
	SPRITE_STATE_ATTACK2,

	SPRITE_STATE_GRASS,
	SPRITE_STATE_DIRT,

	SPRITE_STATE_MAX,
};
static const char* string_of_animation_state[] = {
	[SPRITE_STATE_IDLE]    = PPSTR(SPRITE_STATE_IDLE),
	[SPRITE_STATE_WALK]    = PPSTR(SPRITE_STATE_WALK),
	[SPRITE_STATE_RUN]     = PPSTR(SPRITE_STATE_RUN),
	[SPRITE_STATE_ATTACK1] = PPSTR(SPRITE_STATE_ATTACK1),
	[SPRITE_STATE_ATTACK2] = PPSTR(SPRITE_STATE_ATTACK2),

	[SPRITE_STATE_GRASS] = PPSTR(SPRITE_STATE_GRASS),
	[SPRITE_STATE_DIRT]  = PPSTR(SPRITE_STATE_DIRT),
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
	char name[32];
	int statec;
	struct SpriteState* states;
};

struct Sprite {
	Vec2   pos;
	int16  gi;
	int8   state, frame;
	int8   sheet, group;
	uint16 time;
};

struct SpriteSheet {
	char name[32];
	int w, h, z;
	int groupc;
	struct VArray       sprites;
	struct SpriteGroup* groups;
	struct Image*       albedo;
	struct Image*       normal;
	struct Pipeline* atomic pipeln;
	SBO sprite_sheet_data;
	SBO sprite_data;
};

void sprites_init(void);
struct SpriteSheet* sprites_get_sheet(const char* sheet_name);
int  sprites_get_sheet_id(struct SpriteSheet* sheet);
int  sprites_get_group(struct SpriteSheet* sheet, const char* group_name);
void sprites_record_commands(VkCommandBuffer cmd_buf);
void sprites_update(void);

void sprite_sheets_free(void);
struct SpriteSheet* sprite_sheet_new(const char* name, int z_lvl);
struct SpriteSheet* sprite_sheet_load(struct SpriteSheet* sheet_data);

struct Sprite* sprite_new(struct SpriteSheet* sheet, int group_id, Vec2 pos);
struct Sprite* sprite_new_batch(struct SpriteSheet* sheet, int group_id, int spritec, Vec2* poss, enum SpriteStateType* states);
struct Sprite* sprite_new_by_gi(struct SpriteSheet* sheet, int gi, Vec2 pos);
void sprite_set_state(struct Sprite* sprite, enum SpriteStateType type, enum Direction dir);

#endif


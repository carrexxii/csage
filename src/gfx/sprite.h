#ifndef GFX_SPRITE_H
#define GFX_SPRITE_H

#include "common.h"
#include "maths/types.h"
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
typedef enum SpriteStateType {
	SPRITE_STATE_IDLE,
	SPRITE_STATE_WALK,
	SPRITE_STATE_RUN,
	SPRITE_STATE_ATTACK1,
	SPRITE_STATE_ATTACK2,

	SPRITE_STATE_GRASS,
	SPRITE_STATE_DIRT,

	SPRITE_STATE_MAX,
} SpriteStateType;
static const char* string_of_animation_state[] = {
	[SPRITE_STATE_IDLE]    = PPSTR(SPRITE_STATE_IDLE),
	[SPRITE_STATE_WALK]    = PPSTR(SPRITE_STATE_WALK),
	[SPRITE_STATE_RUN]     = PPSTR(SPRITE_STATE_RUN),
	[SPRITE_STATE_ATTACK1] = PPSTR(SPRITE_STATE_ATTACK1),
	[SPRITE_STATE_ATTACK2] = PPSTR(SPRITE_STATE_ATTACK2),

	[SPRITE_STATE_GRASS] = PPSTR(SPRITE_STATE_GRASS),
	[SPRITE_STATE_DIRT]  = PPSTR(SPRITE_STATE_DIRT),
};

typedef struct SpriteFrame {
	uint16 x, y, w, h;
} SpriteFrame;

typedef struct SpriteState {
	enum SpriteStateType type;
	enum DirectionMask   dir;
	int                  duration;
	int                  gi;
	int                  framec;
	struct SpriteFrame*  frames;
} SpriteState;

typedef struct SpriteGroup {
	char name[32];
	int statec;
	struct SpriteState* states;
} SpriteGroup;

typedef struct Sprite {
	Vec2   pos;
	int16  gi;
	int8   state, frame;
	int8   sheet, group;
	uint16 time;
} Sprite;

typedef struct SpriteSheet {
	char name[32];
	int w, h, z;
	int groupc;
	VArray       sprites;
	SpriteGroup* groups;
	Image*       albedo;
	Image*       normal;
	Pipeline*    pipeln;
	SBO sprite_sheet_data;
	SBO sprite_data;
} SpriteSheet;

void         sprites_init(void);
SpriteSheet* sprites_get_sheet(const char* sheet_name);
int          sprites_get_sheet_id(SpriteSheet* sheet);
int          sprites_get_group(SpriteSheet* restrict sheet, const char* restrict group_name);
void         sprites_record_commands(VkCommandBuffer cmd_buf);
void         sprites_update(void);

void         sprite_sheets_free(void);
SpriteSheet* sprite_sheet_new(const char* name, int z_lvl);
SpriteSheet* sprite_sheet_load(const SpriteSheet* sheet_data);

Sprite* sprite_new(SpriteSheet* sheet, int group_id, Vec2 pos);
Sprite* sprite_new_batch(SpriteSheet* restrict sheet, int group_id, int spritec, Vec2* poss, SpriteStateType* restrict states);
Sprite* sprite_new_by_gi(SpriteSheet* sheet, int gi, Vec2 pos);
void    sprite_set_state(Sprite* sprite, SpriteStateType type, DirectionMask dir);

#endif


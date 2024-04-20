#ifndef GFX_SPRITE_H
#define GFX_SPRITE_H

#include "common.h"
#include "soe.h"
#include "maths/types.h"
#include "buffers.h"
#include "image.h"
#include "pipeline.h"

#define MAX_SPRITE_SHEETS    8
#define DEFAULT_SPRITE_COUNT 8
#define SIZEOF_SHEET_HEADER  sizeof(int[4])
#define SPRITE_SCALE         45

typedef enum SpriteDir: uint8 {
	SPRITE_DIR_E  = 0,
	SPRITE_DIR_NE = 1,
	SPRITE_DIR_N  = 2,
	SPRITE_DIR_NW = 3,
	SPRITE_DIR_W  = 4,
	SPRITE_DIR_SW = 5,
	SPRITE_DIR_S  = 6,
	SPRITE_DIR_SE = 7,
	SPRITE_DIR_NONE,
} SpriteDir;

typedef struct SpriteFrame {
	uint16 x, y, w, h;
} SpriteFrame;

typedef struct SpriteGroup {
	uint16 duration;
	uint16 framec;
	uint16 gi;
} SpriteGroup;

typedef struct Sprite {
	Vec2   pos;
	uint8  dir, framec;
	uint16 gi, timer, duration;
} Sprite;

typedef struct SpriteSheet {
	char name[32];
	int16 w, h, z;
	int16 groupc;
	SpriteGroup* groups_data;
	SpriteFrame* frames;
	HTable groups;
	VArray sprites;

	Pipeline* pipeln;
	Image*    albedo;
	Image*    normal;
	SBO       sheet_sbo;
	SBO       sprite_sbo;
} SpriteSheet;

typedef struct SpriteSheetCreateInfo {
	int w, h;
	int spritec;
	char (* names)[32];
	int*         framecs;
	SpriteFrame* (* frames)[8];
	int16*       durations;
} SpriteSheetCreateInfo;

void sprites_init(void);
void sprites_update(void);
void sprites_record_commands(VkCommandBuffer cmd_buf);
void sprites_free(void);

SpriteSheet* sprite_sheet_new(const char* name, int z_lvl);
void         sprite_sheet_update_pipeln(SpriteSheet* sheet);
void         sprite_sheet_free(SpriteSheet* sheet);

Sprite* sprite_new(SpriteSheet* sheet, String name, Vec2 pos);
Sprite* sprite_new_batch(SpriteSheet* sheet, String name, isize spritec, Vec2* poss);
void    sprite_set_state(SpriteSheet* sheet, Sprite* sprite, String name, SpriteDir dir);
void    sprite_set_statei(SpriteSheet* sheet, Sprite* sprite, intptr i, SpriteDir dir);

static inline enum SpriteDir sprite_dir_of_mask(enum DirectionMask dir)
{
	switch (dir) {
	case DIR_E: return SPRITE_DIR_E; case DIR_NE: return SPRITE_DIR_NE;
	case DIR_N: return SPRITE_DIR_N; case DIR_NW: return SPRITE_DIR_NW;
	case DIR_W: return SPRITE_DIR_W; case DIR_SW: return SPRITE_DIR_SW;
	case DIR_S: return SPRITE_DIR_S; case DIR_SE: return SPRITE_DIR_SE;
	default:
		ERROR("[GFX] Cannot convert direction \"%s\" (%d) to SpriteDir", STRING_OF_DIRECTION_MASK(dir), dir);
		return SPRITE_DIR_NONE;
	}
}

#endif

// #include "common.h"
// #include "maths/types.h"
// #include "image.h"
// #include "pipeline.h"
// #include "camera.h"

// /* Using a hash map for many different animations + directions might be good */

// #define MAX_SPRITE_SHEETS     (INT8_MAX + 1)
// #define MAX_SPRITE_GROUPS     (INT8_MAX + 1)
// #define MAX_SPRITE_FRAMES     (INT8_MAX + 1)
// #define DEFAULT_SPRITE_SHEETS 8
// #define DEFAULT_SPRITES       16
// #define SPRITE_SCALE          45

// // TODO: this needs to be a hashmap
// typedef enum SpriteStateType {
// 	SPRITE_STATE_IDLE,
// 	SPRITE_STATE_WALK,
// 	SPRITE_STATE_RUN,
// 	SPRITE_STATE_ATTACK1,
// 	SPRITE_STATE_ATTACK2,

// 	SPRITE_STATE_GRASS,
// 	SPRITE_STATE_DIRT,

// 	SPRITE_STATE_MAX,
// } SpriteStateType;
// static const char* string_of_animation_state[] = {
// 	[SPRITE_STATE_IDLE]    = PPSTR(SPRITE_STATE_IDLE),
// 	[SPRITE_STATE_WALK]    = PPSTR(SPRITE_STATE_WALK),
// 	[SPRITE_STATE_RUN]     = PPSTR(SPRITE_STATE_RUN),
// 	[SPRITE_STATE_ATTACK1] = PPSTR(SPRITE_STATE_ATTACK1),
// 	[SPRITE_STATE_ATTACK2] = PPSTR(SPRITE_STATE_ATTACK2),

// 	[SPRITE_STATE_GRASS] = PPSTR(SPRITE_STATE_GRASS),
// 	[SPRITE_STATE_DIRT]  = PPSTR(SPRITE_STATE_DIRT),
// };

// typedef struct SpriteFrame {
// 	uint16 x, y, w, h;
// } SpriteFrame;

// typedef struct SpriteState {
// 	enum SpriteStateType type;
// 	enum DirectionMask   dir;
// 	int                  duration;
// 	int                  gi;
// 	int                  framec;
// 	struct SpriteFrame*  frames;
// } SpriteState;

// typedef struct SpriteGroup {
// 	char name[32];
// 	int statec;
// 	struct SpriteState* states;
// } SpriteGroup;

// typedef struct Sprite {
// 	Vec2   pos;
// 	int16  gi;
// 	int8   state, frame;
// 	int8   sheet, group;
// 	uint16 time;
// } Sprite;

// typedef struct SpriteSheet {
// 	char name[32];
// 	int w, h, z;
// 	int groupc;
// 	VArray       sprites;
// 	SpriteGroup* groups;
// 	Image*       albedo;
// 	Image*       normal;
// 	Pipeline*    pipeln;
// 	SBO sprite_sheet_data;
// 	SBO sprite_data;
// } SpriteSheet;

// void         sprites_init(void);
// SpriteSheet* sprites_get_sheet(const char* sheet_name);
// int          sprites_get_sheet_id(SpriteSheet* sheet);
// int          sprites_get_group(SpriteSheet* restrict sheet, const char* restrict group_name);
// void         sprites_record_commands(VkCommandBuffer cmd_buf);
// void         sprites_update(void);

// void         sprite_sheets_free(void);
// SpriteSheet* sprite_sheet_new(const char* name, int z_lvl);
// SpriteSheet* sprite_sheet_load(const SpriteSheet* sheet_data);

// Sprite* sprite_new(SpriteSheet* sheet, int group_id, Vec2 pos);
// Sprite* sprite_new_batch(SpriteSheet* restrict sheet, int group_id, int spritec, Vec2* poss, SpriteStateType* restrict states);
// Sprite* sprite_new_by_gi(SpriteSheet* sheet, int gi, Vec2 pos);
// void    sprite_set_state(Sprite* sprite, SpriteStateType type, DirectionMask dir);

// #endif


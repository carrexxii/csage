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
#define STRING_OF_SPRITE_STATE_TYPE(x) (   \
	x == SPRITE_NONE   ? "SPRITE_NONE"   : \
	x == SPRITE_IDLE   ? "SPRITE_IDLE"   : \
	x == SPRITE_WALK   ? "SPRITE_WALK"   : \
	x == SPRITE_RUN    ? "SPRITE_RUN"    : \
	x == SPRITE_ATTACK1? "SPRITE_ATTACK1": \
	x == SPRITE_ATTACK2? "SPRITE_ATTACK2": \
	"<Unknown animation>")
enum SpriteStateType {
	SPRITE_NONE,
	SPRITE_IDLE,
	SPRITE_WALK,
	SPRITE_RUN,
	SPRITE_ATTACK1,
	SPRITE_ATTACK2,
};

struct SpriteFrame {
	uint16 x, y, w, h;
};

struct SpriteState {
	enum SpriteStateType type;
	enum Direction       dir;
	int duration;
	int framec;
	struct SpriteFrame* frames;
};

struct SpriteGroup {
	char name[32];
	int statec;
	struct SpriteState* states;
};

struct Sprite {
	Vec3  pos;
	int16 start, frame;
	int16 group, state;
	int8 sheet;
	uint time;
	byte pad[4];
};

struct SpriteSheet {
	char name[32];
	int w, h;
	int groupc;
	struct VArray       sprites;
	struct SpriteGroup* groups;
	struct Texture      albedo;
	struct Texture      normal;
	struct Pipeline pipeln;
	SBO sprite_sheet_data;
	SBO sprite_data;
};

void sprites_init(void);
void sprites_record_commands(VkCommandBuffer cmd_buf);
void sprites_update(void);
void sprites_free(void);
int  sprite_sheet_new(char* name);
struct Sprite* sprite_new(char* sheet_name, char* group_name, Vec3 pos);
void sprite_set_state(struct Sprite* sprite, enum SpriteStateType type, enum Direction dir);
void sprite_destroy(int sprite);

#endif

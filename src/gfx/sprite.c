#include "vulkan/vulkan.h"

#include "lua.h"
#include "util/varray.h"
#include "vulkan.h"
#include "buffers.h"
#include "renderer.h"
#include "camera.h"
#include "sprite.h"

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
	uint16 duration;
};

struct SpriteState {
	enum SpriteAnimation type;
	int framec;
	struct SpriteFrame* frames;
};

struct Sprite {
	char* name;
	int statec;
	struct SpriteState* states;
};

struct SpriteSheet {
	char* name;
	int w, h;
	int spritec;
	struct Texture* tex;
	struct Sprite*  sprites;
};

static struct Pipeline pipeln;
static uint64 spritec;
static struct VArray sheets;
static struct VArray sprites;

void sprites_init()
{
	pipeln = (struct Pipeline){
		.vshader     = create_shader(SHADER_PATH "/sprite.vert"),
		.fshader     = create_shader(SHADER_PATH "/sprite.frag"),
		.topology    = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.push_stages = VK_SHADER_STAGE_FRAGMENT_BIT,
		.push_sz     = sizeof(int),
		.uboc        = 2,
		.sboc        = 0,
		.imgc        = 1,
	};

	sheets  = varray_new(DEFAULT_SPRITE_SHEETS, sizeof(struct SpriteSheet));
	sprites = varray_new(DEFAULT_SPRITES, sizeof(ID));
}

static void sprites_reinit()
{
	VkImageView img_views[sheets.len];
	struct SpriteSheet* sheet;
	for (int i = 0; i < sheets.len; i++) {
		sheet = varray_get(&sheets, i);
		img_views[i] = sheet->tex->image_view;
	}

	pipeln.dset_cap = sheets.len;
	pipeln_alloc_dsets(&pipeln);
	pipeln_create_dset(&pipeln, 2, (UBO[]){ global_camera_ubo, global_light_ubo },
								0, NULL,
								sheets.len, img_views);
	pipeln_init(&pipeln, renderpass);
}

void sprites_record_commands(VkCommandBuffer cmd_buf)
{
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.pipeln);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.layout, 0, 1, pipeln.dsets, 0, NULL);

	struct MapLayer* layer;
	struct MapChunk* chunk;
	for (int i = 0; i < sheets.len; i++) {
		vkCmdPushConstants(cmd_buf, pipeln.layout, pipeln.push_stages, 0, pipeln.push_sz, &i);
		vkCmdDraw(cmd_buf, 6, 1, 0, 0);
	}
}

void sprites_free()
{
	struct SpriteSheet* sheet;
	struct Sprite*      sprite;
	for (int i = 0; i < sheets.len; i++) {
		sheet = varray_get(&sheets, i);
		texture_free(sheet->tex);
		for (int j = 0; j < sheet->spritec; j++) {
			sprite = &sheet->sprites[j];
			for (int k = 0; k < sprite->statec; k++)
				free(sprite->states[k].frames);
			free(sprite);
		}
	}
	varray_free(&sheets);
	varray_free(&sprites);
	pipeln_free(&pipeln);
}

ID sprite_sheet_new(char* name)
{
	char buf[256];
	sprintf(buf, SPRITE_PATH "/%s.lua", name);
	lua_getglobal(lua_state, "load_sprite_sheet");
	lua_pushstring(lua_state, buf);
	if (lua_pcall(lua_state, 1, 1, 0))
		ERROR("[LUA] Failed in call to \"load_sprite_sheet\": \n\t%s", lua_tostring(lua_state, -1));

	if (lua_isnoneornil(lua_state, -1))
		ERROR("[RES] Failed to load \"%s\" (%s)", name, buf);
	struct SpriteSheet sheet = *(struct SpriteSheet*)lua_topointer(lua_state, -1);
	lua_pop(lua_state, 1);

	snprintf(buf, PATH_BUFFER_SIZE, SPRITE_PATH "/sheets/%s.png", name);
	sheet.tex = smalloc(sizeof(struct Texture));
	*sheet.tex = texture_new_from_image(buf);

	struct SpriteSheet s;
	s = sheet;
	s.sprites = smalloc(sheet.spritec*sizeof(struct Sprite));
	memcpy(s.sprites, sheet.sprites, sheet.spritec*sizeof(struct Sprite));
	for (int i = 0; i < sheet.spritec; i++) {
		s.sprites[i].statec = sheet.sprites[i].statec;
		s.sprites[i].states = smalloc(sheet.sprites[i].statec*sizeof(struct SpriteState));
		for (int j = 0; j < sheet.sprites[i].statec; j++) {
			s.sprites[i].states[j].type   = sheet.sprites[i].states[j].type;
			s.sprites[i].states[j].framec = sheet.sprites[i].states[j].framec;
			s.sprites[i].states[j].frames = smalloc(sheet.sprites[i].states[j].framec*sizeof(struct SpriteFrame));
			for (int k = 0; k < sheet.sprites[i].states[j].framec; k++) {
				s.sprites[i].states[j].frames[k] = sheet.sprites[i].states[j].frames[k];
				struct SpriteFrame* f = &s.sprites[i].states[j].frames[k];
			}
		}
	}

	varray_push(&sheets, &s);
	sprites_reinit();

	DEBUG(3, "[RES] Loaded new sprite sheet \"%s\" with %d sprites", sheet.name, sheet.spritec);
	return sheets.len - 1;
}

ID sprite_new(ID sheet)
{
	ID sprite = varray_push(&sprites, &spritec);

	return spritec++;
}

void sprite_destroy(ID sprite)
{
	ID id = -1;
	for (int i = 0; i < sprites.len; i++)
		if (*(ID*)varray_get(&sprites, i) == sprite)
			varray_set(&sprites, i, &id);
}


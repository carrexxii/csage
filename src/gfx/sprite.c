#include "vulkan/vulkan.h"

#include "resmgr.h"
#include "lua.h"
#include "maths/maths.h"
#include "util/varray.h"
#include "vulkan.h"
#include "buffers.h"
#include "renderer.h"
#include "camera.h"
#include "map.h"
#include "sprite.h"

static void sprite_sheet_print(struct SpriteSheet* sheet);

static int sheetc;
static struct SpriteSheet sheets[MAX_SPRITE_SHEETS];
static struct {
	int sheet;
} push_const;

void sprites_init()
{

}

int sprites_get_sheet(char* sheet_name)
{
	struct SpriteSheet* sheet = NULL;
	for (int i = 0; i < sheetc; i++) {
		sheet = &sheets[i];
		if (!strcmp(sheet->name, sheet_name))
			return i;
	}

	ERROR("[GFX] Sheet \"%s\" not found", sheet_name);
	return -1;
}

int sprites_get_group(int sheet_id, char* group_name)
{
	assert(sheet_id < sheetc);

	struct SpriteSheet* sheet = &sheets[sheet_id];
	struct SpriteGroup* group;
	for (int i = 0; i < sheet->groupc; i++) {
		group = &sheet->groups[i];
		if (!strcmp(group->name, group_name))
			return i;
	}

	ERROR("[GFX] Sprite group \"%s\" not found in sheet \"%s\"", group_name, sheets[sheet_id].name);
	return -1;
}

void sprites_record_commands(VkCommandBuffer cmd_buf)
{
	isize data_sz;
	struct SpriteSheet* sheet;
	struct Sprite*      sprite;
	for (int i = 0; i < sheetc; i++) {
		sheet = &sheets[i];
		if (!sheet->sprites.len)
			continue;
		else if (sheet->needs_update)
			sprite_sheet_init_pipeline(sheet);

		buffer_update(sheet->sprite_data, sheet->sprites.len*sizeof(struct Sprite), sheet->sprites.data, 0);
		push_const.sheet = i;

		vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, sheet->pipeln.pipeln);
		vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, sheet->pipeln.layout, 0, 1, sheet->pipeln.dsets, 0, NULL);
		vkCmdPushConstants(cmd_buf, sheet->pipeln.layout, sheet->pipeln.push_stages, 0, sheet->pipeln.push_sz, &push_const);
		vkCmdDraw(cmd_buf, sheet->sprites.len*6, 1, 0, 0);
	}
}

void sprites_update()
{
	struct SpriteSheet* sheet;
	struct SpriteGroup* group;
	struct Sprite*      sprite;
	for (int i = 0; i < sheetc; i++) {
		sheet = &sheets[i];

		for (int j = 0; j < sheet->sprites.len; j++) {
			sprite = varray_get(&sheet->sprites, j);
			group  = &sheet->groups[sprite->group];
			if (!group->states[sprite->state].duration)
				continue;

			sprite->time += DT_MS;
			if (sprite->frame >= group->states[sprite->state].framec - 1)
				sprite->time = 0;
			sprite->frame = sprite->time / group->states[sprite->state].duration;
		}
	}
}

/* -------------------------------------------------------------------- */

void sprite_sheet_init_pipeline(struct SpriteSheet* sheet)
{
	if (sheet->pipeln.pipeln)
		pipeln_free(&sheet->pipeln);

	sheet->pipeln = (struct Pipeline){
		.vshader     = load_shader(STRING(SHADER_PATH "/sprite.vert")),
		.fshader     = load_shader(STRING(SHADER_PATH "/sprite.frag")),
		.topology    = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.push_stages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		.push_sz     = sizeof(push_const),
		.dset_cap    = 1,
		.uboc        = 2,
		.sboc        = 4,
		.imgc        = 2,
	};
	pipeln_alloc_dsets(&sheet->pipeln);
	pipeln_create_dset(&sheet->pipeln,
		sheet->pipeln.uboc, (UBO[]){ global_camera_ubo, global_light_ubo },
	    sheet->pipeln.sboc, (SBO[]){ sheet->sprite_sheet_data, sheet->sprite_data,
	                                 spot_lights_sbo, point_lights_sbo, },
		sheet->pipeln.imgc, (VkImageView[]){ sheet->albedo.image_view, sheet->normal.image_view }
	);
	pipeln_init(&sheet->pipeln);
	sheet->needs_update = false;
}

int sprite_sheet_new(char* name, int z_lvl)
{
	char path[PATH_BUFFER_SIZE];
	snprintf(path, PATH_BUFFER_SIZE, SPRITE_PATH "/%s.lua", name);
	lua_getglobal(lua_state, "load_sprite_sheet");
	lua_pushstring(lua_state, path);
	if (lua_pcall(lua_state, 1, 1, 0)) {
		ERROR("[LUA] Failed in call to \"load_sprite_sheet\": \n\t%s", lua_tostring(lua_state, -1));
		return -1;
	}

	if (lua_isnoneornil(lua_state, -1)) {
		ERROR("[RES] Failed to load \"%s\" (%s)", name, path);
		return -1;
	}
	struct SpriteSheet* sheet_data = lua_topointer(lua_state, -1);
	sheet_data->z = z_lvl;
	lua_pop(lua_state, 1);

	return sprite_sheet_load(sheet_data);
}

int sprite_sheet_load(struct SpriteSheet* sheet_data)
{
	struct SpriteSheet* sheet = &sheets[sheetc];

	*sheet = *sheet_data;
	sheet->sprites = varray_new(DEFAULT_SPRITES, sizeof(struct Sprite));
	sheet->groups  = smalloc(sheet_data->groupc*sizeof(struct SpriteGroup));
	memcpy(sheet->groups, sheet_data->groups, sheet_data->groupc*sizeof(struct SpriteGroup));

	struct SpriteGroup* group;
	struct SpriteState* state;
	int total_framec = 0;
	for (int i = 0; i < sheet_data->groupc; i++) {
		group = &sheet->groups[i];
		group->statec = sheet_data->groups[i].statec;
		group->states = smalloc(sheet_data->groups[i].statec*sizeof(struct SpriteState));
		for (int j = 0; j < sheet_data->groups[i].statec; j++) {
			state = &group->states[j];
			state->gi       = sheet_data->groups[i].states[j].gi;
			state->type     = sheet_data->groups[i].states[j].type;
			state->dir      = sheet_data->groups[i].states[j].dir;
			state->duration = sheet_data->groups[i].states[j].duration;
			state->framec   = sheet_data->groups[i].states[j].framec;
			state->frames   = smalloc(sheet_data->groups[i].states[j].framec*sizeof(struct SpriteFrame));
			total_framec += sheet_data->groups[i].states[j].framec;
			for (int k = 0; k < sheet_data->groups[i].states[j].framec; k++)
				state->frames[k] = sheet_data->groups[i].states[j].frames[k];
		}
	}

	sheet->sprite_data       = sbo_new(DEFAULT_SPRITES * sizeof(struct Sprite));
	sheet->sprite_sheet_data = sbo_new(sizeof(int[4]) + total_framec * sizeof(struct SpriteFrame));
	buffer_update(sheet->sprite_sheet_data, sizeof(int[4]), (int[]){ sheet_data->w, sheet_data->h, sheet_data->z, 45 }, 0);
	isize framec = 0;
	isize total_copied = 0;
	for (int i = 0; i < sheet->groupc; i++) {
		for (int j = 0; j < sheet->groups[i].statec; j++) {
			framec = sheet->groups[i].states[j].framec;
			buffer_update(sheet->sprite_sheet_data, framec*sizeof(struct SpriteFrame),
			              sheet->groups[i].states[j].frames, sizeof(int[4]) + total_copied*sizeof(struct SpriteFrame));
			total_copied += framec;
		}
	}

	char path[PATH_BUFFER_SIZE];
	snprintf(path, PATH_BUFFER_SIZE, SPRITE_PATH "/sheets/%s.png", sheet_data->name);
	sheet->albedo = texture_new_from_image(path);
	snprintf(path, PATH_BUFFER_SIZE, SPRITE_PATH "/sheets/%s-normal.png", sheet_data->name);
	sheet->normal = texture_new_from_image(path);

	if (spot_lights_sbo.buf && point_lights_sbo.buf)
		sprite_sheet_init_pipeline(sheet);
	else
		sheet->needs_update = true;

	DEBUG(1, "[RES] Loaded new sprite sheet \"%s\" with %d sprites", sheet->name, sheet->groupc);
	return sheetc++;
}

void sprite_sheet_free()
{
	struct SpriteSheet* sheet;
	struct SpriteGroup* group;
	for (int i = 0; i < sheetc; i++) {
		sheet = &sheets[i];
		for (int j = 0; j < sheet->groupc; j++) {
			group = &sheet->groups[j];
			for (int k = 0; k < group->statec; k++)
				sfree(group->states[k].frames);
			sfree(group->states);
		}
		sfree(sheet->groups);

		varray_free(&sheet->sprites);
		sbo_free(&sheet->sprite_sheet_data);
		sbo_free(&sheet->sprite_data);
		texture_free(&sheet->albedo);
		texture_free(&sheet->normal);
		pipeln_free(&sheet->pipeln);
	}
}

static void sprite_sheet_print(struct SpriteSheet* sheet)
{
	DEBUG(1, "Sprite sheet \"%s\" is %dx%d with %d groups", sheet->name, sheet->w, sheet->h, sheet->groupc);
	struct SpriteGroup* group;
	struct SpriteState* state;
	struct SpriteFrame* frame;
	for (int i = 0; i < sheet->groupc; i++) {
		group = &sheet->groups[i];
		DEBUG(1, "\tGroup \"%s\" w/%d states", group->name, group->statec);
		for (int j = 0; j < group->statec; j++) {
			state = &group->states[j];
			DEBUG(1, "\tState \"%s\" w/%d frames in direction %u and frame time of %d",
			      string_of_animation_state[state->type], state->framec, state->dir, state->duration); // TODO: STRING_OF_DIRECTION
			for (int k = 0; k < state->framec; k++) {
				frame = &state->frames[k];
				fprintf(stderr, "(%d, %d, %d, %d) ", frame->x, frame->y, frame->w, frame->h);
			}
			fprintf(stderr, "\n");
		}
	}
	DEBUG(1, "\n");
}

/* -------------------------------------------------------------------- */

struct Sprite* sprite_new(int sheet_id, int group_id, Vec3 pos)
{
	assert(sheet_id < sheetc);
	assert(group_id < sheets[sheet_id].groupc);

	struct SpriteSheet* sheet = &sheets[sheet_id];
	struct SpriteGroup* group = &sheet->groups[group_id];
	struct Sprite sprite = {
		.pos   = pos,
		.gi    = group->states[0].gi,
		.group = group_id,
		.sheet = sheet_id,
	};
	isize spri = varray_push(&sheet->sprites, &sprite);

	// TODO: handle resizing properly
	if (sheet->sprite_data.sz <= (isize)(sheet->sprites.len * sizeof(struct Sprite))) {
		buffer_free(&sheet->sprite_data);
		sheet->sprite_data = sbo_new(sheet->sprites.len * sizeof(struct Sprite));
		sprite_sheet_init_pipeline(sheet);
	}

	DEBUG(5, "[GFX] Created new sprite (%d %s@%s) at (%.2f, %.2f)",
	      sprite.gi, sheet->name, sheet->groups[group_id].name, pos.x, pos.y);
	return varray_get(&sheet->sprites, spri);
}

struct Sprite* sprite_new_batch(int sheet_id, int group_id, int spritec, Vec3* poss, enum SpriteStateType* states)
{
	assert(sheet_id < sheetc);
	assert(group_id < sheets[sheet_id].groupc);

	struct SpriteSheet* sheet = &sheets[sheet_id];
	struct SpriteGroup* group = &sheet->groups[group_id];
	struct Sprite* sprites = smalloc(spritec * sizeof(struct Sprite));
	int state;
	for (int i = 0; i < spritec; i++) {
		sprites[i] = (struct Sprite){
			.pos   = poss[i],
			.gi    = group->states[states[i]].gi,
			.sheet = sheet_id,
			.group = group_id,
			.state = states[i],
		};
	}

	isize i = varray_push_many(&sheet->sprites, spritec, sprites);

	if (sheet->sprite_data.sz <= (isize)(sheet->sprites.len * sizeof(struct Sprite))) {
		buffer_free(&sheet->sprite_data);
		sheet->sprite_data = sbo_new(sheet->sprites.len * sizeof(struct Sprite));
		sprite_sheet_init_pipeline(sheet);
	}

	sfree(sprites);
	DEBUG(3, "[GFX] Created sprite batch of %d sprites from %s@%s",
	      spritec, sheets[sheet_id].name, sheets[sheet_id].groups[group_id].name);
	return varray_get(&sheet->sprites, i);
}

void sprite_set_state(struct Sprite* sprite, enum SpriteStateType type, enum Direction dir)
{
	// TODO: update
	struct SpriteSheet* sheet = &sheets[sprite->sheet];
	sprite->time = 0;
	int gi = 0;
	for (int i = 0; i < sheet->groupc; i++) {
		for (int j = 0; j < sheet->groups[i].statec; j++) {
			if (sheet->groups[i].states[j].type == type && sheet->groups[i].states[j].dir == dir) {
				sprite->state = j;
				sprite->gi    = gi;
				return;
			}
			gi += sheet->groups[i].states[j].framec;
		}
	}
}

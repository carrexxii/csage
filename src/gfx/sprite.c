#include "lua.h"
#include "resmgr.h"
#include "renderer.h"
#include "map.h"
#include "sprite.h"

static PipelineCreateInfo pipeln_ci;
static int         sheetc;
static SpriteSheet sheets[MAX_SPRITE_SHEETS];

static struct {
	int sheet;
} push_const;

void sprites_init()
{
	pipeln_ci = (PipelineCreateInfo){
		.vshader     = STRING(SHADER_PATH "/sprite.vert"),
		.fshader     = STRING(SHADER_PATH "/sprite.frag"),
		.ubo_stages  = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		.uboc        = 2,
		.sbo_stages  = VK_SHADER_STAGE_VERTEX_BIT,
		.sboc        = 4,
		.imgc        = 2,
		.push_sz     = sizeof(push_const),
		.push_stages = VK_SHADER_STAGE_VERTEX_BIT,
	};

	INFO(TERM_DARK_GREEN "[GFX] Initialized sprites");
}

void sprites_update()
{

}

void sprites_record_commands(VkCommandBuffer cmd_buf)
{
	isize        data_sz;
	Pipeline*    pl;
	SpriteSheet* sheet;
	Sprite*      sprite;
	for (int i = 0; i < sheetc; i++) {
		sheet = &sheets[i];
		if (!sheet->sprites.len)
			continue;

		buffer_update(sheet->sprite_data, sheet->sprites.len*sizeof(Sprite), sheet->sprites.data, 0);
		push_const.sheet = i;

		pl = sheet->pipeln;
		vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pl->pipeln);
		vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pl->layout, 0, 1, &pl->dset.set, 0, NULL);
		vkCmdPushConstants(cmd_buf, pl->layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push_const), &push_const);
		vkCmdDraw(cmd_buf, sheet->sprites.len*6, 1, 0, 0);
	}
}

void sprites_free()
{
	SpriteSheet* sheet;
	for (int i = 0; i < sheetc; i++) {
		sheet = &sheets[i];
		varray_free(&sheet->sprites);
		htable_free(&sheet->groups);
		sfree(sheet->groups_data);

		sbo_free(&sheet->sprite_sheet_data);
		sbo_free(&sheet->sprite_data);
		pipeln_free(sheet->pipeln);
	}
}

/* -------------------------------------------------------------------- */

SpriteSheet* sprite_sheet_new(const char* name, int z_lvl)
{
	SpriteSheet* sheet = &sheets[sheetc];
	strncpy(sheet->name, name, sizeof(sheet->name));

	char path[PATH_BUFFER_SIZE];
	snprintf(path, PATH_BUFFER_SIZE, SPRITE_PATH "/%s.lua", name); // TODO: implement this for String
	lua_getglobal(lua_state, "load_sprite_sheet");
	lua_pushstring(lua_state, path);
	if (lua_pcall(lua_state, 1, 1, 0) || lua_isnoneornil(lua_state, -1)) {
		ERROR("[LUA] Failed in call to \"load_sprite_sheet\" (%s:%s): \n\t%s",
		      name, path, lua_tostring(lua_state, -1));
		return NULL;
	}

	const SpriteSheetCreateInfo* sheetci = lua_topointer(lua_state, -1);
	lua_pop(lua_state, 1);

	snprintf(path, PATH_BUFFER_SIZE, SPRITE_PATH "/sheets/%s.png", name);
	sheet->albedo = load_image(STRING(path));
	snprintf(path, PATH_BUFFER_SIZE, SPRITE_PATH "/sheets/%s-normal.png", name);
	sheet->normal = load_image(STRING(path));

	sheet->w = sheetci->w;
	sheet->h = sheetci->h;
	sheet->z = z_lvl;
	sheet->groups_data = smalloc(sheetci->spritec*sizeof(SpriteGroup)); // TODO: array into htable?
	sheet->groups  = htable_new(2*sheetci->spritec); // FIXME: not sure what the correct size should be
	sheet->sprites = varray_new(DEFAULT_SPRITE_COUNT, sizeof(Sprite));

	/* Metadata for sprite groups */
	for (int i = 0; i < sheetci->spritec; i++) {
		htable_insert(&sheet->groups, STRING(sheetci->names[i]), i);
		sheet->groups_data[i].framec   = sheetci->framecs[i];
		sheet->groups_data[i].duration = sheetci->durations[i];
	}

	/* Flatten the frame data */
	SpriteFrame* frames;
	isize framec = 0;
	for (int i = 0; i < sheetci->spritec; i++)
		framec += 8*sheetci->framecs[i];
	DV(framec);
	frames = smalloc(framec*sizeof(SpriteFrame));

	framec = 0;
	for (int i = 0; i < sheetci->spritec; i++)            /* Sprite    */
		for (int d = 0; d < 8; d++)                       /* Direction */
			for (int j = 0; j < sheetci->framecs[i]; j++) /* Frame     */
				memcpy(frames + framec++, &sheetci->frames[i][d][j], sizeof(SpriteFrame));

	sheet->sprite_data = sbo_new(framec*sizeof(SpriteFrame));
	buffer_update(sheet->sprite_data, framec*sizeof(SpriteFrame), frames, 0);

	pipeln_ci.ubos[0] = global_camera_ubo;
	pipeln_ci.ubos[1] = global_light_ubo;
	pipeln_ci.sbos[0] = sheet->sprite_sheet_data;
	pipeln_ci.sbos[1] = sheet->sprite_data;
	pipeln_ci.sbos[2] = global_spot_lights_sbo;
	pipeln_ci.sbos[3] = global_point_lights_sbo;
	pipeln_ci.imgs[0] = sheet->albedo;
	pipeln_ci.imgs[1] = sheet->normal;
	sheet->pipeln = pipeln_new(&pipeln_ci, "Sprites");
	pipeln_update(sheet->pipeln, &pipeln_ci);

	sfree(frames);
	sheetc++;
	return sheet;
}

/* -------------------------------------------------------------------- */

Sprite* sprite_new(SpriteSheet* sheet, String name, Vec2 pos)
{
	intptr i = htable_get(&sheet->groups, name);
	if (i == -1) {
		ERROR("[GFX] Sprite with name \"%s\" could not be found in sheet \"%s\"", name.data, sheet->name);
		return NULL;
	}

	SpriteGroup group = sheet->groups_data[i];
	Sprite sprite =  (Sprite){
		.pos      = pos,
		.gi       = group.gi,
		.framec   = group.framec,
		.timer    = 0,
		.duration = group.duration,
	};

	isize spri = varray_push(&sheet->sprites, &sprite);
	return varray_get(&sheet->sprites, spri);
}



// #include <vulkan/vulkan.h>

// #include "resmgr.h"
// #include "lua.h"
// #include "maths/maths.h"
// #include "vulkan.h"
// #include "buffers.h"
// #include "image.h"
// #include "renderer.h"
// #include "camera.h"
// #include "map.h"
// #include "sprite.h"

// static void sprite_sheet_print(SpriteSheet* sheet);
// static void sprite_sheet_update_pipeln(SpriteSheet* sheet);

// static PipelineCreateInfo pipeln_ci;
// static int sheetc;
// static SpriteSheet sheets[MAX_SPRITE_SHEETS];
// static struct {
// 	int sheet;
// } push_const;

// void sprites_init()
// {
// 	pipeln_ci = (PipelineCreateInfo){
// 		.vshader     = STRING(SHADER_PATH "/sprite.vert"),
// 		.fshader     = STRING(SHADER_PATH "/sprite.frag"),
// 		.ubo_stages  = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
// 		.uboc        = 2,
// 		.sbo_stages  = VK_SHADER_STAGE_VERTEX_BIT,
// 		.sboc        = 4,
// 		.imgc        = 2,
// 		.push_sz     = sizeof(push_const),
// 		.push_stages = VK_SHADER_STAGE_VERTEX_BIT,
// 	};

// 	INFO(TERM_DARK_GREEN "[GFX] Initialized sprites");
// }

// SpriteSheet* sprites_get_sheet(const char* sheet_name)
// {
// 	SpriteSheet* sheet = NULL;
// 	for (int i = 0; i < sheetc; i++) {
// 		sheet = &sheets[i];
// 		if (!strcmp(sheet->name, sheet_name))
// 			return sheet;
// 	}

// 	ERROR("[GFX] Sheet \"%s\" not found", sheet_name);
// 	return NULL;
// }

// int sprites_get_sheet_id(SpriteSheet* sheet)
// {
// 	for (int i = 0; i < sheetc; i++)
// 		if (&sheets[i] == sheet)
// 			return i;

// 	ERROR("[GFX] Invalid sprite sheet: %p", (void*)sheet);
// 	return -1;
// }

// int sprites_get_group(SpriteSheet* restrict sheet, const char* restrict group_name)
// {
// 	assert(sheet && group_name);

// 	SpriteGroup* group;
// 	for (int i = 0; i < sheet->groupc; i++) {
// 		group = &sheet->groups[i];
// 		if (!strcmp(group->name, group_name))
// 			return i;
// 	}

// 	ERROR("[GFX] Sprite group \"%s\" not found in sheet \"%s\"", group_name, sheet->name);
// 	return -1;
// }

// void sprites_record_commands(VkCommandBuffer cmd_buf)
// {
// 	isize        data_sz;
// 	Pipeline*    pl;
// 	SpriteSheet* sheet;
// 	Sprite*      sprite;
// 	for (int i = 0; i < sheetc; i++) {
// 		sheet = &sheets[i];
// 		if (!sheet->sprites.len)
// 			continue;

// 		buffer_update(sheet->sprite_data, sheet->sprites.len*sizeof(Sprite), sheet->sprites.data, 0);
// 		push_const.sheet = i;

// 		pl = sheet->pipeln;
// 		vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pl->pipeln);
// 		vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pl->layout, 0, 1, &pl->dset.set, 0, NULL);
// 		vkCmdPushConstants(cmd_buf, pl->layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push_const), &push_const);
// 		vkCmdDraw(cmd_buf, sheet->sprites.len*6, 1, 0, 0);
// 	}
// }

// void sprites_update()
// {
// 	SpriteSheet* sheet;
// 	SpriteGroup* group;
// 	Sprite*      sprite;
// 	for (int i = 0; i < sheetc; i++) {
// 		sheet = &sheets[i];

// 		for (int j = 0; j < sheet->sprites.len; j++) {
// 			sprite = varray_get(&sheet->sprites, j);
// 			group  = &sheet->groups[sprite->group];
// 			if (!group->states[sprite->state].duration)
// 				continue;

// 			sprite->time += DT_MS;
// 			if (sprite->frame >= group->states[sprite->state].framec - 1)
// 				sprite->time = 0;
// 			sprite->frame = sprite->time / group->states[sprite->state].duration;
// 		}
// 	}
// }

// /* -------------------------------------------------------------------- */

// SpriteSheet* sprite_sheet_new(const char* name, int z_lvl)
// {
// 	char path[PATH_BUFFER_SIZE];
// 	snprintf(path, PATH_BUFFER_SIZE, SPRITE_PATH "/%s.lua", name);
// 	lua_getglobal(lua_state, "load_sprite_sheet");
// 	lua_pushstring(lua_state, path);
// 	if (lua_pcall(lua_state, 1, 1, 0)) {
// 		ERROR("[LUA] Failed in call to \"load_sprite_sheet\": \n\t%s", lua_tostring(lua_state, -1));
// 		return NULL;
// 	}

// 	if (lua_isnoneornil(lua_state, -1)) {
// 		ERROR("[RES] Failed to load \"%s\" (%s)", name, path);
// 		return NULL;
// 	}
// 	SpriteSheet* sheet_data = (struct SpriteSheet*)lua_topointer(lua_state, -1);
// 	sheet_data->z = z_lvl;
// 	lua_pop(lua_state, 1);

// 	return sprite_sheet_load(sheet_data);
// }

// SpriteSheet* sprite_sheet_load(const SpriteSheet* sheet_data)
// {
// 	SpriteSheet* sheet = &sheets[sheetc++];

// 	*sheet = *sheet_data;
// 	sheet->sprites = varray_new(DEFAULT_SPRITES, sizeof(Sprite));
// 	sheet->groups  = smalloc(sheet_data->groupc*sizeof(SpriteGroup));
// 	memcpy(sheet->groups, sheet_data->groups, sheet_data->groupc*sizeof(SpriteGroup));

// 	SpriteGroup* group;
// 	SpriteState* state;
// 	int total_framec = 0;
// 	for (int i = 0; i < sheet_data->groupc; i++) {
// 		group = &sheet->groups[i];
// 		group->statec = sheet_data->groups[i].statec;
// 		group->states = smalloc(sheet_data->groups[i].statec*sizeof(SpriteState));
// 		for (int j = 0; j < sheet_data->groups[i].statec; j++) {
// 			state = &group->states[j];
// 			state->gi       = sheet_data->groups[i].states[j].gi;
// 			state->type     = sheet_data->groups[i].states[j].type;
// 			state->dir      = sheet_data->groups[i].states[j].dir;
// 			state->duration = sheet_data->groups[i].states[j].duration;
// 			state->framec   = sheet_data->groups[i].states[j].framec;
// 			state->frames   = smalloc(sheet_data->groups[i].states[j].framec*sizeof(SpriteFrame));
// 			total_framec += sheet_data->groups[i].states[j].framec;
// 			for (int k = 0; k < sheet_data->groups[i].states[j].framec; k++)
// 				state->frames[k] = sheet_data->groups[i].states[j].frames[k];
// 		}
// 	}

// 	sheet->sprite_data       = sbo_new(DEFAULT_SPRITES * sizeof(Sprite));
// 	sheet->sprite_sheet_data = sbo_new(sizeof(int[4]) + total_framec * sizeof(SpriteFrame));
// 	buffer_update(sheet->sprite_sheet_data, sizeof(int[4]), (int[]){ sheet_data->w, sheet_data->h, sheet_data->z, SPRITE_SCALE }, 0);
// 	isize framec = 0;
// 	isize total_copied = 0;
// 	for (int i = 0; i < sheet->groupc; i++) {
// 		for (int j = 0; j < sheet->groups[i].statec; j++) {
// 			framec = sheet->groups[i].states[j].framec;
// 			buffer_update(sheet->sprite_sheet_data, framec*sizeof(SpriteFrame),
// 			              sheet->groups[i].states[j].frames, sizeof(int[4]) + total_copied*sizeof(SpriteFrame));
// 			total_copied += framec;
// 		}
// 	}

// 	char path[PATH_BUFFER_SIZE];
// 	snprintf(path, PATH_BUFFER_SIZE, SPRITE_PATH "/sheets/%s.png", sheet_data->name);
// 	sheet->albedo = load_image(STRING(path));
// 	snprintf(path, PATH_BUFFER_SIZE, SPRITE_PATH "/sheets/%s-normal.png", sheet_data->name);
// 	sheet->normal = load_image(STRING(path));

// 	pipeln_ci.ubos[0] = global_camera_ubo;
// 	pipeln_ci.ubos[1] = global_light_ubo;
// 	pipeln_ci.sbos[0] = sheet->sprite_sheet_data;
// 	pipeln_ci.sbos[1] = sheet->sprite_data;
// 	pipeln_ci.sbos[2] = global_spot_lights_sbo;
// 	pipeln_ci.sbos[3] = global_point_lights_sbo;
// 	pipeln_ci.imgs[0] = sheet->albedo;
// 	pipeln_ci.imgs[1] = sheet->normal;
// 	sheet->pipeln = pipeln_new(&pipeln_ci, "Sprites");
// 	sheet->pipeln = pipeln_update(sheet->pipeln, &pipeln_ci);

// 	INFO(TERM_GREEN "[RES] Loaded new sprite sheet (%d) \"%s\" with %d sprites", sheetc - 1, sheet->name, sheet->groupc);
// 	return sheet;
// }

// void sprite_sheets_free()
// {
// 	SpriteSheet* sheet;
// 	SpriteGroup* group;
// 	for (int i = 0; i < sheetc; i++) {
// 		sheet = &sheets[i];
// 		for (int j = 0; j < sheet->groupc; j++) {
// 			group = &sheet->groups[j];
// 			for (int k = 0; k < group->statec; k++)
// 				sfree(group->states[k].frames);
// 			sfree(group->states);
// 		}
// 		sfree(sheet->groups);

// 		varray_free(&sheet->sprites);
// 		sbo_free(&sheet->sprite_sheet_data);
// 		sbo_free(&sheet->sprite_data);
// 		pipeln_free(sheet->pipeln);
// 	}
// }

// static void sprite_sheet_update_pipeln(SpriteSheet* sheet)
// {
// 	assert(pipeln_ci.uboc == 2 && pipeln_ci.sboc == 4 && pipeln_ci.imgc == 2);

// 	pipeln_ci.sbos[0] = sheet->sprite_sheet_data;
// 	pipeln_ci.sbos[1] = sheet->sprite_data;
// 	pipeln_ci.sbos[2] = global_spot_lights_sbo;
// 	pipeln_ci.sbos[3] = global_point_lights_sbo;
// 	pipeln_ci.imgs[0] = sheet->albedo;
// 	pipeln_ci.imgs[1] = sheet->normal;
// 	sheet->pipeln = pipeln_update(sheet->pipeln, &pipeln_ci);
// }

// static void sprite_sheet_print(SpriteSheet* sheet)
// {
// 	INFO(TERM_DARK_GREEN "Sprite sheet \"%s\" is %dx%d with %d groups", sheet->name, sheet->w, sheet->h, sheet->groupc);
// 	SpriteGroup* group;
// 	SpriteState* state;
// 	SpriteFrame* frame;
// 	for (int i = 0; i < sheet->groupc; i++) {
// 		group = &sheet->groups[i];
// 		INFO(TERM_DARK_GREEN "\tGroup \"%s\" w/%d states", group->name, group->statec);
// 		for (int j = 0; j < group->statec; j++) {
// 			state = &group->states[j];
// 			INFO(TERM_DARK_GREEN "\tState \"%s\" w/%d frames in direction %u and frame time of %d",
// 			      string_of_animation_state[state->type], state->framec, state->dir, state->duration); // TODO: STRING_OF_DIRECTION
// 			for (int k = 0; k < state->framec; k++) {
// 				frame = &state->frames[k];
// 				fprintf(stderr, "(%d, %d, %d, %d) ", frame->x, frame->y, frame->w, frame->h);
// 			}
// 			fprintf(stderr, "\n");
// 		}
// 	}
// 	INFO(TERM_DARK_GREEN "\n");
// }

// /* -------------------------------------------------------------------- */

// Sprite* sprite_new(SpriteSheet* sheet, int group_id, Vec2 pos)
// {
// 	int sheet_id = sprites_get_sheet_id(sheet);
// 	SpriteGroup* group = &sheet->groups[group_id];
// 	Sprite sprite = {
// 		.pos   = pos,
// 		.gi    = group->states[0].gi,
// 		.group = group_id,
// 		.sheet = sheet_id,
// 	};
// 	isize spri = varray_push(&sheet->sprites, &sprite);

// 	if (sheet->sprite_data.sz < (isize)(sheet->sprites.cap * sizeof(Sprite))) {
// 		resmgr_defer(RES_BUFFER, &sheet->sprite_data);
// 		sheet->sprite_data = sbo_new(sheet->sprites.cap * sizeof(Sprite));
// 		sprite_sheet_update_pipeln(sheet);
// 	}

// 	INFO(TERM_DARK_GREEN "[GFX] Created new sprite (%d %s@%s) at (%.2f, %.2f)",
// 	      sprite.gi, sheet->name, sheet->groups[group_id].name, pos.x, pos.y);
// 	return varray_get(&sheet->sprites, spri);
// }

// Sprite* sprite_new_batch(SpriteSheet* restrict sheet, int group_id, int spritec, Vec2* poss, enum SpriteStateType* restrict states)
// {
// 	int sheet_id = sprites_get_sheet_id(sheet);
// 	SpriteGroup* group = &sheet->groups[group_id];
// 	Sprite* sprites = smalloc(spritec * sizeof(Sprite));
// 	int state;
// 	for (int i = 0; i < spritec; i++) {
// 		state = states? states[i]: 0;
// 		sprites[i] = (Sprite){
// 			.pos   = poss[i],
// 			.gi    = group->states[state].gi,
// 			.sheet = sheet_id,
// 			.group = group_id,
// 			.state = state,
// 		};
// 	}

// 	isize i = varray_push_many(&sheet->sprites, spritec, sprites);

// 	if (sheet->sprite_data.sz < (isize)(sheet->sprites.cap * sizeof(Sprite))) {
// 		resmgr_defer(RES_BUFFER, &sheet->sprite_data);
// 		sheet->sprite_data = sbo_new(sheet->sprites.cap * sizeof(Sprite));
// 		sprite_sheet_update_pipeln(sheet);
// 	}

// 	sfree(sprites);
// 	INFO(TERM_DARK_GREEN "[GFX] Created sprite batch of %d sprites from %s@%s",
// 	      spritec, sheets[sheet_id].name, sheets[sheet_id].groups[group_id].name);
// 	return varray_get(&sheet->sprites, i);
// }

// Sprite* sprite_new_by_gi(SpriteSheet* sheet, int gi, Vec2 pos)
// {
// 	SpriteGroup* group;
// 	for (int i = 0; i < sheet->groupc; i++) {
// 		group = &sheet->groups[i];
// 		if (BETWEEN(gi, group->states[0].gi, group->states[group->statec - 1].gi))
// 			return sprite_new(sheet, i, pos);
// 	}

// 	ERROR("[GFX] Could not find group containing gi %d in sheet %p", gi, (void*)sheet);
// 	return NULL;
// }

// void sprite_set_state(Sprite* sprite, SpriteStateType type, DirectionMask dir)
// {
// 	// TODO: update
// 	SpriteSheet* sheet = &sheets[sprite->sheet];
// 	int gi = 0;
// 	for (int i = 0; i < sheet->groupc; i++) {
// 		for (int j = 0; j < sheet->groups[i].statec; j++) {
// 			if (sheet->groups[i].states[j].type == type && sheet->groups[i].states[j].dir == dir) {
// 				sprite->state = j;
// 				sprite->gi    = gi;
// 				sprite->time  = 0;
// 				return;
// 			}
// 			gi += sheet->groups[i].states[j].framec;
// 		}
// 	}
// }


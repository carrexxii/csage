#include "vulkan/vulkan.h"

#include "lua.h"
#include "util/varray.h"
#include "vulkan.h"
#include "buffers.h"
#include "renderer.h"
#include "camera.h"
#include "sprite.h"

static void sprite_sheet_print(struct SpriteSheet* sheet);

static int sheetc;
static struct SpriteSheet sheets[MAX_SPRITE_SHEETS];

void sprites_record_commands(VkCommandBuffer cmd_buf)
{
	struct SpriteSheet* sheet;
	struct Sprite*      sprite;

	static int q = 0;
	q += 1;
	if (q % 100 == 0)
		for (int i = 0; i < sheetc; i++) {
			sheet = &sheets[i];
			for (int j = 0; j < sheet->sprites.len; j++) {
				sprite = varray_get(&sheet->sprites, j);
				sprite->frame = (sprite->frame + 1) % sheet->groups[sprite->group].states[sprite->state].framec;
			}
		}

	for (int i = 0; i < sheetc; i++) {
		sheet = &sheets[i];
		if (!sheet->sprites.len)
			continue;
		buffer_update(sheet->sprite_data, sheet->sprites.len*sizeof(struct Sprite), sheet->sprites.data, 0);

		vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, sheet->pipeln.pipeln);
		vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, sheet->pipeln.layout, 0, 1, sheet->pipeln.dsets, 0, NULL);
		// DEBUG(1, "Drawing %d sprites", sheet->sprites.len);
		vkCmdPushConstants(cmd_buf, sheet->pipeln.layout, sheet->pipeln.push_stages, 0, sheet->pipeln.push_sz, &i);
		vkCmdDraw(cmd_buf, sheet->sprites.len*6, 1, 0, 0);
	}
}

void sprites_free()
{
	struct SpriteSheet* sheet;
	struct SpriteGroup* group;
	for (int i = 0; i < sheetc; i++) {
		sheet = &sheets[i];
		// for (int j = 0; j < sheet->groupc; j++) {
		// 	group = &sheet->groups[j];
		// 	// for (int k = 0; k < group->statec; k++)
		// 	// 	free(group->states[k].frames);
		// 	free(group);
		// }
		varray_free(&sheet->sprites);
		sbo_free(&sheet->sprite_sheet_data);
		sbo_free(&sheet->sprite_data);
		texture_free(sheet->tex);
		pipeln_free(&sheet->pipeln);
	}
}

/* -------------------------------------------------------------------- */

int sprite_sheet_new(char* name)
{
	char buf[PATH_BUFFER_SIZE];
	snprintf(buf, PATH_BUFFER_SIZE, SPRITE_PATH "/%s.lua", name);
	lua_getglobal(lua_state, "load_sprite_sheet");
	lua_pushstring(lua_state, buf);
	if (lua_pcall(lua_state, 1, 1, 0)) {
		ERROR("[LUA] Failed in call to \"load_sprite_sheet\": \n\t%s", lua_tostring(lua_state, -1));
		return -1;
	}

	if (lua_isnoneornil(lua_state, -1)) {
		ERROR("[RES] Failed to load \"%s\" (%s)", name, buf);
		return -1;
	}
	struct SpriteSheet* sheet_data = lua_topointer(lua_state, -1);
	lua_pop(lua_state, 1);

	struct SpriteSheet* sheet = &sheets[sheetc];
	*sheet = *sheet_data;
	sheet->sprites = varray_new(DEFAULT_SPRITES, sizeof(struct Sprite));
	sheet->groups  = smalloc(sheet_data->groupc*sizeof(struct SpriteGroup));
	memcpy(sheet->groups, sheet_data->groups, sheet_data->groupc*sizeof(struct SpriteGroup));
	int total_framec = 0;
	for (int i = 0; i < sheet_data->groupc; i++) {
		sheet->groups[i].statec = sheet_data->groups[i].statec;
		sheet->groups[i].states = smalloc(sheet_data->groups[i].statec*sizeof(struct SpriteState));
		for (int j = 0; j < sheet_data->groups[i].statec; j++) {
			sheet->groups[i].states[j].type   = sheet_data->groups[i].states[j].type;
			sheet->groups[i].states[j].framec = sheet_data->groups[i].states[j].framec;
			sheet->groups[i].states[j].frames = smalloc(sheet_data->groups[i].states[j].framec*sizeof(struct SpriteFrame));
			total_framec += sheet_data->groups[i].states[j].framec;
			for (int k = 0; k < sheet_data->groups[i].states[j].framec; k++) {
				sheet->groups[i].states[j].frames[k] = sheet_data->groups[i].states[j].frames[k];
				struct SpriteFrame* f = &sheet->groups[i].states[j].frames[k];
			}
		}
	}

	sheet->sprite_data       = sbo_new(DEFAULT_SPRITES * sizeof(struct Sprite));
	sheet->sprite_sheet_data = sbo_new(total_framec * sizeof(struct SpriteFrame));
	isize framec = 0;
	isize total_copied = 0;
	for (int i = 0; i < sheet->groupc; i++) {
		for (int j = 0; j < sheet->groups[i].statec; j++) {
			framec = sheet->groups[i].states[j].framec;
			buffer_update(sheet->sprite_sheet_data, framec*sizeof(struct SpriteFrame),
			              sheet->groups[i].states[j].frames, total_copied*sizeof(struct SpriteFrame));
			total_copied += framec;
		}
	}

	snprintf(buf, PATH_BUFFER_SIZE, SPRITE_PATH "/sheets/%s.png", name);
	sheet->tex = smalloc(sizeof(struct Texture));
	*sheet->tex = texture_new_from_image(buf);

	sheet->pipeln = (struct Pipeline){
		.vshader     = create_shader(SHADER_PATH "/sprite.vert"),
		.fshader     = create_shader(SHADER_PATH "/sprite.frag"),
		.topology    = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.push_stages = VK_SHADER_STAGE_FRAGMENT_BIT,
		.push_sz     = sizeof(int),
		.dset_cap    = 1,
		.uboc        = 2,
		.sboc        = 2,
		.imgc        = 1,
	};
	pipeln_alloc_dsets(&sheet->pipeln);
	pipeln_create_dset(&sheet->pipeln, 2, (UBO[]){ global_camera_ubo, global_light_ubo },
	                                   2, (SBO[]){ sheet->sprite_sheet_data, sheet->sprite_data },
	                                   1, &sheet->tex->image_view);
	pipeln_init(&sheet->pipeln, renderpass);

	DEBUG(3, "[RES] Loaded new sprite sheet \"%s\" with %d sprites", sheet->name, sheet->groupc);
	sprite_sheet_print(sheet);
	return sheetc++;
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
			DEBUG(1, "\tState \"%s\" w/%d frames", STRING_OF_SPRITE_ANIMATION(state->type), state->framec);
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

struct Sprite* sprite_new(char* restrict sheet_name, char* restrict group_name, Vec3 pos)
{
	int64 i;
	struct SpriteSheet* sheet = NULL;
	for (i = 0; i < sheetc; i++) {
		sheet = &sheets[i];
		if (!strcmp(sheet->name, sheet_name))
			break;
	}
	if (i == sheetc || !sheet) {
		ERROR("[GFX] Sheet \"%s\" not found", sheet_name);
		return NULL;
	}

	int gi = 0;
	struct SpriteGroup* group = NULL;
	for (i = 0; i < sheet->groupc; i++) {
		group  = &sheet->groups[i];
		if (!strcmp(group->name, group_name))
			break;
		for (int j = 0; j < group->statec; j++)
			gi += group->states[j].framec;
	}
	if (i == sheet->groupc || !group) {
		ERROR("[GFX] Sprite group \"%s\" not found in sheet \"%s\"", group_name, sheet_name);
		return NULL;
	}

	struct Sprite sprite = {
		.pos   = pos,
		.start = gi,
		.group = i,
		.frame = 1,
	};
	isize spri = varray_push(&sheet->sprites, &sprite);

	// TODO: handle resizing properly
	if (sheet->sprite_data.sz <= (isize)(sheet->sprites.len * sizeof(struct Sprite))) {
		buffer_free(&sheet->sprite_data);
		sheet->sprite_data = sbo_new(sheet->sprites.len * sizeof(struct Sprite));
	}

	DEBUG(5, "[GFX] Created new sprite (%d %s@%s) at (%.2f, %.2f)", sprite.start, sheet_name, group_name, pos.x, pos.y);
	return varray_get(&sheet->sprites, spri);
}

void sprite_destroy(int sprite)
{

}

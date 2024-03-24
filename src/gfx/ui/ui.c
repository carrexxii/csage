#include "vulkan/vulkan.h"

#include "config.h"
#include "input.h"
#include "util/varray.h"
#include "gfx/vulkan.h"
#include "gfx/buffers.h"
#include "types.h"
#include "ui.h"

static void init_pipeln(void);
static inline void update_container(struct Container* container);
static void cb_mouse_left(bool kdown);

static struct Pipeline pipeln;
static VkVertexInputBindingDescription vert_binds[] = {
	{ .binding   = 0,
	  .stride    = sizeof(struct UIVertex), /* xyrgba */
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, },
};
static VkVertexInputAttributeDescription vert_attrs[] = {
	{ .binding  = 0,
	  .location = 0,
	  .format   = VK_FORMAT_R32G32_SFLOAT, /* xy */
	  .offset   = 0, },
	{ .binding  = 0,
	  .location = 1,
	  .format   = VK_FORMAT_R8G8B8A8_UINT, /* rgba */
	  .offset   = sizeof(float[2]), }
};

// struct UIContext ui_context;
// struct UIObject  ui_containers[UI_MAX_TOP_LEVEL_CONTAINERS];
int containerc = 0;
static struct Container containers[UI_MAX_CONTAINERS];
static struct {
	Vec2 mouse_pos;
	struct { bool lmb, rmb; } mouse_pressed;
	struct { bool lmb, rmb; } mouse_released;
	// struct UIObject* clicked_obj;
} context;
// struct UIState {
// 	bool visible;
// 	bool hover;
// 	bool clicked;
// };

static bool pipeln_needs_update;
static struct VArray vert_buf;

void ui_init()
{
	pipeln_needs_update = true;
	vert_buf = varray_new(UI_DEFAULT_VERTICES, sizeof(struct UIVertex));
	// ui_objs = varray_new(UI_DEFAULT_OBJECT_COUNT, sizeof(struct UIObject));

	// DEBUG(2, "[UI] Initialized UI");
}

void ui_register_keys()
{
	input_register(SDL_BUTTON_LEFT, cb_mouse_left);
}

// struct UIObject* ui_alloc_object() {
// 	return varray_get(&ui_objs, ui_objs.len++);
// }

struct Container* ui_new_container(Rect rect, struct UIStyle* style)
{
	if (containerc >= UI_MAX_CONTAINERS) {
		ERROR("[UI] Total container count (%d) exceeded", UI_MAX_CONTAINERS);
		return NULL;
	}

	struct Container* container = &containers[containerc++];
	*container = (struct Container){
		.rect    = rect,
		.objects = varray_new(UI_DEFAULT_OBJECTS, sizeof(struct UIObject)),
		.styles  = {
			.container = style? style: &default_container_style,
			.button    = &default_button_style,
			.label     = &default_label_style,
		},
	};

	return container;
}

void ui_add(struct Container* container, struct UIObject* obj)
{
	assert(container && obj);

	varray_push(&container->objects, obj);
}

void ui_build()
{
	struct Container* container;
	struct UIObject*  obj;
	for (int i = 0; i < containerc; i++) {
		container = &containers[i];
		if (!container->has_update)
			continue;
		varray_push_many(&vert_buf, 6, UI_VERTS(container->rect, container->styles.container->bg));

		for (int j = 0; j < container->objects.len; j++) {
			obj = varray_get(&container->objects, j);
			switch (obj->type) {
			case UI_BUTTON: button_build(obj, &vert_buf, container->styles.button); break;
			default:
				ERROR("[UI] Failed to build object with type %d", obj->type);
			}
		}

		isize verts_sz = vert_buf.len * sizeof(struct UIVertex);
		if (!container->vbo.buf)
			container->vbo = vbo_new(verts_sz, vert_buf.data, true);
		else
			buffer_update(container->vbo, verts_sz, vert_buf.data, 0);
	}

	varray_reset(&vert_buf);
	if (pipeln_needs_update)
		init_pipeln();
}

void ui_update()
{
	float mx = ((float)mouse_x / config.winw - 0.5f)*2.0f;
	float my = ((float)mouse_y / config.winh - 0.5f)*2.0f;
	Vec2 mouse = VEC2((mouse_x / config.winw - 0.5f)*2.0f, (mouse_y / config.winh - 0.5f)*2.0f);

	struct Container* container;
	struct UIObject*  obj;
	for (int i = 0; i < containerc; i++) {
		container = &containers[i];
		if (point_in_rect(mouse, container->rect)) {
			for (int j = 0; j < container->objects.len; j++) {
				obj = varray_get(&container->objects, j);
				if (!point_in_rect(mouse, obj->rect)) {
					obj->state = (struct UIState){ 0 };
				} else {
					if (obj->type == UI_BUTTON && obj->state.clicked && !context.mouse_pressed.lmb)
						button_on_click(obj);

					obj->state.hover   = true;
					obj->state.clicked = context.mouse_pressed.lmb;
				}

			}
		}
		container->has_update = true;
	}
}

void ui_record_commands(VkCommandBuffer cmd_buf)
{
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.pipeln);
	struct Container* container;
	for (int i = 0; i < containerc; i++) {
		container = &containers[i];
		if (container->has_update)
			ui_build();
		vkCmdBindVertexBuffers(cmd_buf, 0, 1, &container->vbo.buf, (VkDeviceSize[]){ 0 });
		vkCmdDraw(cmd_buf, 6*(containers[i].objects.len + 1), 1, 0, i);
	}
}

void ui_free()
{
	struct Container* container;
	struct UIObject*  obj;
	for (int i = 0; i < containerc; i++) {
		container = &containers[i];
		vbo_free(&container->vbo);
		varray_free(&container->objects);
	}

	varray_free(&vert_buf);
	pipeln_free(&pipeln);
}

/* -------------------------------------------------------------------- */

static void init_pipeln()
{
	if (pipeln.pipeln)
		pipeln_free(&pipeln);

	VkImageView img_views[UI_MAX_IMAGES];
	int imgc = 0;
	for (int i = 0; i < containerc; i++)
		for (int j = 0; j < containers[i].texturec; j++)
			img_views[imgc++] = containers[i].textures[j]->image_view;

	pipeln = (struct Pipeline){
		.vshader    = create_shader(SHADER_PATH "/ui.vert"),
		.fshader    = create_shader(SHADER_PATH "/ui.frag"),
		.topology   = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.vert_bindc = 1,
		.vert_binds = vert_binds,
		.vert_attrc = 2,
		.vert_attrs = vert_attrs,
		.dset_cap   = 1,
		.imgc       = imgc,
	};
	pipeln_alloc_dsets(&pipeln);
	pipeln_create_dset(&pipeln, pipeln.uboc, NULL, pipeln.sboc, NULL, pipeln.imgc, img_views);
	pipeln_init(&pipeln);

	pipeln_needs_update = false;
}

static void cb_mouse_left(bool kdown)
{
	context.mouse_pressed.lmb  =  kdown;
	context.mouse_released.lmb = !kdown;
}

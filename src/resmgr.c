#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <vulkan/vulkan.h>

#include "util/string.h"
#include "util/file.h"
#include "util/htable.h"
#include "util/queue.h"
#include "gfx/device.h"
#include "gfx/buffers.h"
#include "gfx/image.h"
#include "gfx/pipeline.h"
#include "gfx/renderer.h"
#include "resmgr.h"

const char* string_of_handle_type[] = {
	[RES_NONE]     = "RES_NONE",
	[RES_FILE]     = "RES_FILE",
	[RES_SHADER]   = "RES_SHADER",
	[RES_IMAGE]    = "RES_IMAGE",
	[RES_BUFFER]   = "RES_BUFFER",
	[RES_PIPELINE] = "RES_PIPELINE",
};

struct Handle {
	enum ResourceType type;
	int   refc;
	void* data;
};

struct DeferHandle {
	int64 frame;
	enum ResourceType type;
	union {
		struct Pipeline* pipeln;
		struct Buffer    buf;
	} data;
};

static VkShaderModule new_shader(String path);
static struct Image   new_image(String path);

static struct HTable* resources;
static struct VArray  handles;
static struct Queue*  deferred;

static Mutex defer_lock;

void resmgr_init()
{
	mtx_init(&defer_lock, mtx_plain);

	resources = htable_new(RESMGR_DEFAULT_HTABLE_SIZE);
	handles   = varray_new(RESMGR_DEFAULT_HTABLE_SIZE/2, sizeof(struct Handle));
	deferred  = queue_new(16, sizeof(struct DeferHandle));
}

void resmgr_printout()
{
	htable_print(resources);
}

void resmgr_defer(enum ResourceType type, void* data)
{
	mtx_lock(&defer_lock);

	struct DeferHandle handle = {
		.type  = type,
		.frame = frame_number,
	};
	switch (type) {
	case RES_BUFFER  : handle.data.buf = *(struct Buffer*)data; break;
	case RES_PIPELINE: handle.data.pipeln = data              ; break;
	default:
		ERROR("[RES] Unmatched resource type for defer: %s", STRING_OF_RESOURCE_TYPE(type));
	}

	enqueue(deferred, &handle);

	mtx_unlock(&defer_lock);
}

void resmgr_clean()
{
	mtx_lock(&defer_lock);

	struct DeferHandle* handle;
	while (!queue_is_empty(deferred)) {
		handle = queue_peek(deferred);
		if (frame_number - handle->frame <= FRAMES_IN_FLIGHT)
			break;

		dequeue(deferred);
		switch (handle->type) {
		case RES_BUFFER  : buffer_free(&handle->data.buf)  ; break;
		case RES_PIPELINE: pipeln_free(handle->data.pipeln); break;
		default:
			ERROR("[RES] Resource of type %s not free'd in defer", STRING_OF_RESOURCE_TYPE(handle->type));
		}
	}

	mtx_unlock(&defer_lock);
}

void resmgr_free()
{
	struct Handle* handle;
	for (int i = 0; i < handles.len; i++) {
		handle = varray_get(&handles, i);
		if (!handle->data)
			continue;

		switch (handle->type) {
		case RES_FILE  : free(handle->data); break;
		case RES_SHADER: vkDestroyShaderModule(logical_gpu, handle->data, NULL); break;
		case RES_IMAGE : image_free(handle->data); break;
		default:
			ERROR("[RES] Unmatched handle type \"%s\" (%p with %d references)",
			      STRING_OF_RESOURCE_TYPE(handle->type), handle->data, handle->refc);
		}
	}

	queue_free(deferred);
}

/* -------------------------------------------------------------------- */

VkShaderModule load_shader(String path)
{
	int64 res_id = htable_get(resources, path);
	if (res_id == -1) {
		VkShaderModule module = new_shader(path);
		struct Handle handle = {
			.type = RES_SHADER,
			.refc = 0,
			.data = module,
		};
		res_id = varray_push(&handles, &handle);
		htable_insert(resources, path, res_id);
	} else {
		DEBUG(3, "[RES] Shader \"%s\" is in cache", path.data);
	}

	struct Handle* res = varray_get(&handles, res_id);
	if (res->type != RES_SHADER)
		ERROR("[RES] Resource requested as shader is listed as \"%s\"", STRING_OF_RESOURCE_TYPE(res->type));

	res->refc++;
	return res->data;
}

struct Image* load_image(String path)
{
	int64 res_id = htable_get(resources, path);
	if (res_id == -1) {
		struct Image* img = smalloc(sizeof(struct Image));
		*img = new_image(path);
		struct Handle handle = {
			.type = RES_IMAGE,
			.refc = 0,
			.data = img,
		};
		res_id = varray_push(&handles, &handle);
		htable_insert(resources, path, res_id);
	} else {
		DEBUG(3, "[RES] Image \"%s\" is in cache", path.data);
	}

	struct Handle* res = varray_get(&handles, res_id);
	if (res->type != RES_IMAGE)
		ERROR("[RES] Resource requested as image is listed as \"%s\":\n\t%p (%d references)",
		      STRING_OF_RESOURCE_TYPE(res->type), res->data, res->refc);

	res->refc++;
	return res->data;
}

/* -------------------------------------------------------------------- */

static VkShaderModule new_shader(String path)
{
	VkShaderModule module;
	FILE* file = file_open(path.data, "rb");
	isize size = file_size(file);
	char* code = file_loadf(file);
	VkShaderModuleCreateInfo moduleci = {
		.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = size,
		.pCode    = (const uint32*)code,
	};

	if (vkCreateShaderModule(logical_gpu, &moduleci, NULL, &module))
		ERROR("[VK] Failed to create shader module \"%s\"", path.data);
	else
		DEBUG(2, "[VK] Created new shader module \"%s\"", path.data);

	free(code);
	return module;
}

static struct Image new_image(String path)
{
	struct Image img = { 0 };

	int w, h, ch;
	uint8* pxs = stbi_load(path.data, &w, &h, &ch, 4);
	if (!pxs) {
		ERROR("[RES] Failed to load image \"%s\"", path.data);
		return img;
	}

	img = image_of_memory(w, h, pxs);

	stbi_image_free(pxs);
	DEBUG(2, "[RES] Created new image \"%s\" (%dx%d)", path.data, w, h);
	return img;
}

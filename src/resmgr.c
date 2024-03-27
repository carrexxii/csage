#include "vulkan/vulkan.h"

#include "util/string.h"
#include "util/file.h"
#include "util/htable.h"
#include "gfx/device.h"
#include "resmgr.h"

#define STRING_OF_HANDLE_TYPE(x) ((x) < HANDLE_MAX? string_of_handle_type[x]: "<Not a handle type>")
enum HandleType {
	HANDLE_NONE,
	HANDLE_FILE,
	HANDLE_SHADER,
	HANDLE_MAX,
};
const char* string_of_handle_type[] = {
	[HANDLE_NONE]   = "HANDLE_NONE",
	[HANDLE_FILE]   = "HANDLE_FILE",
	[HANDLE_SHADER] = "HANDLE_SHADER",
};

struct Handle {
	enum HandleType type;
	int refc;
	void* data;
};

static VkShaderModule shader_new(String path);

struct HTable* resources;
struct VArray  handles;

void resmgr_init()
{
	resources = htable_new(RESMGR_DEFAULT_HTABLE_SIZE);
	handles   = varray_new(RESMGR_DEFAULT_HTABLE_SIZE/2, sizeof(struct Handle));
}

void resmgr_printout()
{
	htable_print(resources);
}

void resmgr_free()
{
	struct Handle* handle;
	for (int i = 0; i < handles.len; i++) {
		handle = varray_get(&handles, i);
		if (!handle->data)
			continue;

		switch (handle->type) {
		case HANDLE_FILE  : free(handle->data); break;
		case HANDLE_SHADER: vkDestroyShaderModule(logical_gpu, handle->data, NULL); break;
		default:
			ERROR("[RES] Unmatched handle type \"%s\" (%p with %d references)",
			      STRING_OF_HANDLE_TYPE(handle->type), handle->data, handle->refc);
		}
	}
}

/* -------------------------------------------------------------------- */

/* Shader modules are never free'd from */
VkShaderModule load_shader(String path)
{
	int64 res_id = htable_get(resources, path);
	if (res_id == -1) {
		VkShaderModule module = shader_new(path);
		struct Handle handle = {
			.type = HANDLE_SHADER,
			.refc = 1,
			.data = module,
		};
		res_id = varray_push(&handles, &handle);
		htable_insert(resources, path, res_id);
	} else {
		DEBUG(3, "[RES] Shader \"%s\" is in cache", path.data);
	}

	struct Handle* res = varray_get(&handles, res_id);
	if (res->type != HANDLE_SHADER)
		ERROR("[RES] Resource requested as shader is listed as \"%s\"", STRING_OF_HANDLE_TYPE(res->type));

	res->refc++;
	return res->data;
}

/* -------------------------------------------------------------------- */

static VkShaderModule shader_new(String path)
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

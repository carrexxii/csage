#include "common.h"
#include "vulkan/vulkan.h"
#include <stdint.h>
#include <vulkan/vulkan_core.h>
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "vulkan.h"
#include "pipeline.h"
#include "util/file.h"
#include "buffers.h"
#include "camera.h"
#include "model.h"

#define MAX_MODELS           16
#define MAX_MATERIALS        8
#define VERTEX_ELEMENT_COUNT 8
#define SIZEOF_VERTEX        sizeof(float[VERTEX_ELEMENT_COUNT])
#define INDEX_TYPE           uint16
#define SIZEOF_INDEX         sizeof(INDEX_TYPE)
#define LINE_BUFFER_SIZE     256

static void print_model(struct Model mdl);

/* -------------------------------------------------------------------- */
static VkVertexInputBindingDescription vert_binds[] = {
	/* xyznnnuv */
	{ .binding   = 0,
	  .stride    = SIZEOF_VERTEX,
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, },
};
static VkVertexInputAttributeDescription vertex_attrs[] = {
	/* xyz */
	{ .binding  = 0,
	  .location = 0,
	  .format   = VK_FORMAT_R32G32B32_SFLOAT,
	  .offset   = 0, },
	/* nnn */
	{ .binding  = 0,
	  .location = 1,
	  .format   = VK_FORMAT_R32G32B32_SFLOAT,
	  .offset   = sizeof(float[3]), },
	/* uv */
	{ .binding  = 0,
	  .location = 2,
	  .format   = VK_FORMAT_R32G32_SFLOAT,
	  .offset   = sizeof(float[6]), },
};
/* -------------------------------------------------------------------- */

static struct Pipeline pipeln;
static mat4 matrices[MAX_MODELS];
static struct Model models[MAX_MODELS];
static intptr modelc = 0;
static UBO ubo_bufs[2];
static SBO sbo_buf;

void models_init(VkRenderPass render_pass)
{
	sbo_buf     = sbo_new(MAX_MODELS*sizeof(mat4));
	ubo_bufs[0] = ubo_new(sizeof(mat4));                          /* Camera matrix */
	ubo_bufs[1] = ubo_new(MAX_MATERIALS*sizeof(struct Material)); /* Material data */
	pipeln = (struct Pipeline){
		.vshader    = create_shader(SHADER_DIR "model.vert"),
		.fshader    = create_shader(SHADER_DIR "model.frag"),
		.vertbindc  = 1,
		.vertbinds  = vert_binds,
		.vertattrc  = ARRAY_LEN(vertex_attrs),
		.vertattrs  = vertex_attrs,
		.uboc       = 2,
		.ubos       = ubo_bufs,
		.sbo        = &sbo_buf,
		.sbosz      = MAX_MODELS*sizeof(mat4),
		.pushstages = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pushsz     = sizeof(int), /* Material index - must be a multiple of 4 */
	};
	pipeln_init(&pipeln, render_pass);
}

// TODO: this doesnt work properly for removing elements
ID model_new(char* path, bool keep_verts)
{
	struct Model* model = &models[modelc];

	cgltf_options options = {
		.type = cgltf_file_type_glb,
	};
	cgltf_data* data;
	if (cgltf_parse_file(&options, path, &data))
		ERROR("[GFX] CGLTF failed to parse file \"%s\"", path);
	else
		DEBUG(5, "Basic model data for \"%s\" loaded with:\n\tMeshes: %lu\n\tMaterials: %lu\n\tTextures: %lu\n\tBuffers: %lu",
		      path, data->meshes_count, data->materials_count, data->textures_count, data->buffers_count);

	if (cgltf_load_buffers(&options, data, path))
		ERROR("[GFX] Error loading buffer data");
	else
		DEBUG(5, "Loaded buffer data for \"%s\"", path);

	/* *** Material Data *** */
	model->materialc = data->materials_count;
	model->materials = scalloc(model->materialc, sizeof(struct Material));
	if (model->materialc > MAX_MATERIALS)
		ERROR("[GFX] Model has %d materials. MAX_MATERIALS is set to %d", model->materialc, MAX_MATERIALS);

	cgltf_material*     material;
	cgltf_image*        img;
	cgltf_texture_view* texture;
	for (int m = 0; m < model->materialc; m++) {
		material = &data->materials[m];
		if (material->has_pbr_metallic_roughness) {
			memcpy(model->materials[m].albedo, material->pbr_metallic_roughness.base_color_factor, sizeof(float[4]));
			model->materials[m].metallic  = material->pbr_metallic_roughness.metallic_factor;
			model->materials[m].roughness = material->pbr_metallic_roughness.roughness_factor;
			if (material->pbr_metallic_roughness.metallic_roughness_texture.texture ||
				material->pbr_metallic_roughness.base_color_texture.texture)
				DEBUG(5, "[GFX] Metallic texture unaccounted for");
		}
		if (material->normal_texture.texture) {
				DEBUG(5, "[GFX] Normal texture unaccounted for");
		}
		if (material->occlusion_texture.texture) {
				DEBUG(5, "[GFX] Occlusion texture unaccounted for");
		}
		if (material->emissive_texture.texture) {
				DEBUG(5, "[GFX] Emissive texture unaccounted for");
		}
	}

	/* *** Mesh Data *** */
	model->meshc  = data->meshes_count;
	model->meshes = scalloc(data->meshes_count, sizeof(struct Mesh));

	cgltf_mesh*      mesh;
	cgltf_primitive* prim;
	cgltf_accessor*  attr;
	float*  vert;
	uint16* ind_16;
	uint32* ind_32;
	for (uint m = 0; m < data->meshes_count; m++) {
		mesh = &data->meshes[m];
		for (int p = 0; p < (int)mesh->primitives_count; p++){
			prim = &mesh->primitives[p];
			assert(prim->type == cgltf_primitive_type_triangles);
			for (int a = 0; a < (int)prim->attributes_count; a++) {
				attr = prim->attributes[a].data;
				if (attr->component_type != cgltf_component_type_r_32f && attr->type != cgltf_type_vec3)
					continue;
				model->meshes[m].vertc = attr->count;
				if (!model->meshes[m].verts)
					model->meshes[m].verts = smalloc(attr->count*SIZEOF_VERTEX);
				vert = (float*)attr->buffer_view->buffer->data + attr->buffer_view->offset/sizeof(float) + attr->offset/sizeof(float);
				int i = 0;
				int i_start = 0;
				switch(prim->attributes[a].type) {
					case cgltf_attribute_type_position: i_start = 0; break;
					case cgltf_attribute_type_normal  : i_start = 3; break;
					case cgltf_attribute_type_texcoord: i_start = 6; break;
					default:
						continue;
				}
				for (int v = 0; v < (int)attr->count; v++) {
					model->meshes[m].verts[VERTEX_ELEMENT_COUNT*v + i_start + 0] = vert[i + 0];
					model->meshes[m].verts[VERTEX_ELEMENT_COUNT*v + i_start + 1] = vert[i + 1];
					if (i_start != 6) /* 6 = UV coordinates */
						model->meshes[m].verts[VERTEX_ELEMENT_COUNT*v + i_start + 2] = vert[i + 2];
					i += (int)(attr->stride/sizeof(float));
				}
			}
			if (prim->indices) {
				attr = prim->indices;
				model->meshes[m].indc = attr->count;
				int i = 0;
				switch (attr->component_type) {
					case cgltf_component_type_r_16u:
						model->meshes[m].inds = smalloc(attr->count*sizeof(INDEX_TYPE));
						ind_16 = (INDEX_TYPE*)attr->buffer_view->buffer->data + attr->buffer_view->offset/sizeof(INDEX_TYPE) + attr->offset/sizeof(INDEX_TYPE);
						for (int v = 0; v < (int)attr->count; v++) {
							model->meshes[m].inds[v] = ind_16[i];
							i += (int)(attr->stride/sizeof(INDEX_TYPE));
						}
						break;
					case cgltf_component_type_r_32u:
						model->meshes[m].inds = smalloc(attr->count*sizeof(INDEX_TYPE));
						ind_32 = (uint32*)attr->buffer_view->buffer->data + attr->buffer_view->offset/sizeof(uint32) + attr->offset/sizeof(uint32);
						for (int v = 0; v < (int)attr->count; v++) {
							model->meshes[m].inds[v] = (INDEX_TYPE)ind_32[i];
							i += (int)(attr->stride/sizeof(uint32));
						}
						break;
					default:
						ERROR("[GFX] Unsupported index size value: %u", attr->component_type);
				}

			}

			/* Assign materials to their meshes */
			// TODO: Add a default material for those that dont have one
			for (int i = 0; i < (int)data->materials_count; i++)
				if (&data->materials[i] == data->meshes[i].primitives[p].material)
					model->meshes[i].materiali = i;
		}
	}

	int indc = 0;
	for (int i = 0; i < model->meshc; i++) {
		model->meshes[i].vbo = vbo_new(model->meshes[i].vertc*SIZEOF_VERTEX, model->meshes[i].verts);
		model->meshes[i].ibo = ibo_new(model->meshes[i].indc*SIZEOF_INDEX, model->meshes[i].inds);
		indc += model->meshes[i].indc;
		if (!keep_verts) {
			free(model->meshes[i].verts);
			free(model->meshes[i].inds);
			model->meshes[i].verts = NULL;
			model->meshes[i].inds  = NULL;
		}
	}

	cgltf_free(data);
	DEBUG(3, "[GFX] Loaded file \"%s\" with %d meshes (%d vertices/%d triangles)", path, model->meshc, indc, indc/3);
	return modelc++;
}

mat4* model_get_matrix(ID model_id)
{
	return &matrices[model_id];
}

void models_record_commands(VkCommandBuffer cmd_buf)
{
	mat4 vp;
	camera_get_vp(vp);
	buffer_update(ubo_bufs[0], sizeof(mat4), vp);
	
	void* mem;
	vkMapMemory(gpu, sbo_buf.mem, 0, modelc*sizeof(mat4), 0, &mem);
	memcpy(mem, matrices, modelc*sizeof(mat4));
	vkUnmapMemory(gpu, sbo_buf.mem);

	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.pipeln);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.layout, 0, 1, &pipeln.dset, 0, NULL);
	struct Model* model;
	struct Mesh*  mesh;
	for (int i = 0; i < modelc; i++) {
		model = &models[i];
		buffer_update(ubo_bufs[1], model->materialc*sizeof(struct Material), model->materials);
		for (int m = 0; m < models[i].meshc; m++) {
			mesh = &model->meshes[m];
			// DEBUG(1, "[%d] Drawing %d vertices", i, models[i].meshes[m].vertc);
			vkCmdBindVertexBuffers(cmd_buf, 0, 1, &mesh->vbo.buf, (VkDeviceSize[]){ 0 });
			vkCmdBindIndexBuffer(cmd_buf, mesh->ibo.buf, 0, VK_INDEX_TYPE_UINT16);
			vkCmdPushConstants(cmd_buf, pipeln.layout, pipeln.pushstages, 0, pipeln.pushsz, &mesh->materiali);
			vkCmdDrawIndexed(cmd_buf, mesh->indc, 1, 0, 0, 0);
		}
	}
}

void model_free(ID model_id)
{
	struct Model* model = &models[model_id];
	for (int i = 0; i < model->meshc; i++) {
		if (model->meshes[i].verts)
			sfree(model->meshes[i].verts);
		if (model->meshes[i].inds)
			sfree(model->meshes[i].inds);
		vbo_free(&model->meshes[i].vbo);
		ibo_free(&model->meshes[i].ibo);
	}
	model->meshc = 0;
}

void models_free()
{
	for (int i = 0; i < MAX_MODELS; i++)
		model_free(i);
	pipeln_free(&pipeln);
}

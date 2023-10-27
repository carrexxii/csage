#include "vulkan/vulkan.h"
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "vulkan.h"
#include "pipeline.h"
#include "util/file.h"
#include "buffers.h"
#include "camera.h"
#include "model.h"

#define LINE_BUFFER_SIZE 256

static void print_model(struct Model mdl);

/* -------------------------------------------------------------------- */
static VkVertexInputBindingDescription vert_binds[] = {
	/* [3:xyz][3:nnn][2:uv][4:bone ids][4:bone weights] */
	{ .binding   = 0,
	  .stride    = sizeof(float[3]),
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, },
	{ .binding   = 1,
	  .stride    = sizeof(float[3]),
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, },
	{ .binding   = 2,
	  .stride    = sizeof(float[2]),
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, },
	{ .binding   = 3,
	  .stride    = sizeof(uint8[4]),
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, },
	{ .binding   = 4,
	  .stride    = sizeof(float[4]),
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, },
};
static VkVertexInputAttributeDescription vertex_attrs[] = {
	/* xyz */
	{ .binding  = 0,
	  .location = 0,
	  .format   = VK_FORMAT_R32G32B32_SFLOAT,
	  .offset   = 0, },
	/* nnn */
	{ .binding  = 1,
	  .location = 1,
	  .format   = VK_FORMAT_R32G32B32_SFLOAT,
	  .offset   = sizeof(float[3]), },
	/* uv */
	{ .binding  = 2,
	  .location = 2,
	  .format   = VK_FORMAT_R32G32_SFLOAT,
	  .offset   = sizeof(float[6]), },
	/* bone IDs */
	{ .binding  = 3,
	  .location = 3,
	  .format   = VK_FORMAT_R8G8B8A8_UINT,
	  .offset   = sizeof(float[8]), },
	/* bone weights */
	{ .binding  = 4,
	  .location = 4,
	  .format   = VK_FORMAT_R32G32B32A32_SFLOAT,
	  .offset   = sizeof(float[8]) + sizeof(uint8[4]), },
};
/* -------------------------------------------------------------------- */

void (*update_model_transforms)(SBO);
mat4s* model_transforms;

static struct Pipeline pipeln;
static struct Pipeline pipeln_static;
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
		.vertbindc  = ARRAY_LEN(vert_binds),
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

	pipeln_static = (struct Pipeline){
		.vshader    = create_shader(SHADER_DIR "model_static.vert"),
		.fshader    = create_shader(SHADER_DIR "model.frag"),
		.vertbindc  = ARRAY_LEN(vert_binds) - 2,
		.vertbinds  = vert_binds,
		.vertattrc  = ARRAY_LEN(vertex_attrs) - 2,
		.vertattrs  = vertex_attrs,
		.uboc       = 2,
		.ubos       = ubo_bufs,
		.sbo        = &sbo_buf,
		.sbosz      = MAX_MODELS*sizeof(mat4),
		.pushstages = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pushsz     = sizeof(int), /* Material index - must be a multiple of 4 */
	};
	pipeln_init(&pipeln_static, render_pass);
}

// TODO: this doesnt work properly for removing elements
struct Model* model_new(char* path, bool keep_verts)
{
	struct Model* model = &models[modelc++];

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
	model->materialc = data->materials_count? data->materials_count: 1;
	model->materials = scalloc(model->materialc, sizeof(struct Material));
	if (model->materialc > MAX_MATERIALS)
		ERROR("[GFX] Model has %d materials. MAX_MATERIALS is set to %d", model->materialc, MAX_MATERIALS);

	cgltf_material*     material;
	cgltf_image*        img;
	cgltf_texture_view* texture;
	for (int m = 0; m < (int)data->materials_count; m++) {
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
	// TODO: do this in general
	/* Default material if none exist */
	if (!data->materials_count)
		models->materials[0] = (struct Material){
			.albedo = { 0.3, 0.3, 0.3, 0.3 },
		};

	/* *** Mesh Data *** */
	model->meshc  = data->meshes_count;
	model->meshes = scalloc(data->meshes_count, sizeof(struct Mesh));

	/* Find the largest mesh and allocate for it */
	int vert_max = 0;
	int indc_max = 0;
	for (uint m = 0; m < data->meshes_count; m++) {
		for (int p = 0; p < (int)data->meshes[m].primitives_count; p++) {
			if (data->meshes[m].primitives[p].indices)
				indc_max = MAX(indc_max, (int)data->meshes[m].primitives[p].indices->count);
			for (int a = 0; a < (int)data->meshes[m].primitives[p].attributes_count; a++)
				vert_max = MAX(vert_max, (int)data->meshes[m].primitives[p].attributes[a].data->count);
		}
	}
	uint16* inds = smalloc(indc_max*sizeof(uint16));
	float* verts = smalloc(vert_max*(sizeof(float[12]) + sizeof(uint8[4])));
	float* normals       = verts   + 3*vert_max;
	float* uvs           = normals + 3*vert_max;
	uint8* joint_ids     = (uint8*)(uvs + 2*vert_max);
	float* joint_weights = (float*)(joint_ids + 4*vert_max);

	cgltf_mesh*      mesh;
	cgltf_primitive* prim;
	cgltf_accessor*  attr;
	float*  vert;
	uint16* ind_16;
	uint32* ind_32;
	for (uint m = 0; m < data->meshes_count; m++) {
		mesh = &data->meshes[m];
		for (int p = 0; p < (int)mesh->primitives_count; p++) {
			prim = &mesh->primitives[p];
			assert(prim->type == cgltf_primitive_type_triangles);
			for (int a = 0; a < (int)prim->attributes_count; a++) {
				attr = prim->attributes[a].data;
				// if (attr->component_type != cgltf_component_type_r_32f && attr->type != cgltf_type_vec3)
					// continue;
				// model->meshes[m].vertc = attr->count;
				vert = (float*)attr->buffer_view->buffer->data + attr->buffer_view->offset/sizeof(float) + attr->offset/sizeof(float);
				int i = 0;
				// DEBUG_VALUE(mesh->name);
				for (int v = 0; v < (int)attr->count; v++) {
					switch(prim->attributes[a].type) {
						case cgltf_attribute_type_position:
							verts[3*v + 0] = vert[i + 0];
							verts[3*v + 1] = vert[i + 1];
							verts[3*v + 2] = vert[i + 2];
							// DEBUG(1, "vert: %.2f, %.2f, %.2f", vert[i], vert[i + 1], vert[i + 2]);
							break;
						case cgltf_attribute_type_normal:
							normals[3*v + 0] = vert[i + 0];
							normals[3*v + 1] = vert[i + 1];
							normals[3*v + 2] = vert[i + 2];
							// DEBUG(1, "normal: %.2f, %.2f, %.2f", vert[i], vert[i + 1], vert[i + 2]);
							break;
						case cgltf_attribute_type_texcoord:
							uvs[2*v + 0] = vert[i + 0];
							uvs[2*v + 1] = vert[i + 1];
							// DEBUG(1, "uv: %.2f, %.2f", vert[i], vert[i + 1]);
							break;
						case cgltf_attribute_type_joints:
							joint_ids[3*v + 0] = vert[i + 0];
							joint_ids[3*v + 1] = vert[i + 1];
							joint_ids[3*v + 2] = vert[i + 2];
							joint_ids[3*v + 3] = vert[i + 2];
							break;
						case cgltf_attribute_type_weights:
							joint_weights[3*v + 0] = vert[i + 0];
							joint_weights[3*v + 1] = vert[i + 1];
							joint_weights[3*v + 2] = vert[i + 2];
							joint_weights[3*v + 3] = vert[i + 2];
							break;
						default:
							continue;
					}
					i += (int)(attr->stride/sizeof(float));
				}
				switch(prim->attributes[a].type) {
					case cgltf_attribute_type_position:
						models->meshes[m].vbos[0] = vbo_new(attr->count*sizeof(float[3]), verts);
						break;
					case cgltf_attribute_type_normal:
						models->meshes[m].vbos[1] = vbo_new(attr->count*sizeof(float[3]), normals);
						break;
					case cgltf_attribute_type_texcoord:
						models->meshes[m].vbos[2] = vbo_new(attr->count*sizeof(float[2]), uvs);
						break;
					case cgltf_attribute_type_joints:
						models->meshes[m].vbos[3] = vbo_new(attr->count*sizeof(uint8[4]), verts);
						break;
					case cgltf_attribute_type_weights:
						models->meshes[m].vbos[4] = vbo_new(attr->count*sizeof(float[4]), verts);
						break;
					default:
						continue;
				}
			}
			if (prim->indices) {
				attr = prim->indices;
				model->meshes[m].indc = attr->count;
				int i = 0;
				switch (attr->component_type) {
					case cgltf_component_type_r_16u:
						ind_16 = (uint16*)attr->buffer_view->buffer->data +
						         attr->buffer_view->offset/sizeof(uint16) +
						         attr->offset/sizeof(uint16);
						for (int v = 0; v < (int)attr->count; v++) {
							inds[v] = (uint16)ind_16[i];
							i += (int)(attr->stride/sizeof(uint16));
						}
						break;
					case cgltf_component_type_r_32u:
						ind_32 = (uint32*)attr->buffer_view->buffer->data +
						         attr->buffer_view->offset/sizeof(uint32) +
						         attr->offset/sizeof(uint32);
						for (int v = 0; v < (int)attr->count; v++) {
							inds[v] = (uint16)ind_32[i];
							i += (int)(attr->stride/sizeof(uint32));
						}
						break;
					default:
						ERROR("[GFX] Unsupported index size value: %u", attr->component_type);
				}

				model->meshes[m].ibo = ibo_new(model->meshes[m].indc*sizeof(uint16), inds);
			}

			/* Assign materials to their meshes */
			// TODO: Add a default material for those that dont have one
			for (int i = 0; i < (int)data->materials_count; i++)
				if (&data->materials[i] == data->meshes[i].primitives[p].material)
					model->meshes[i].materiali = i;
		}
	}

	/* *** Animation Data *** */
	cgltf_skin* skin;
	cgltf_node* node;
	struct Joint* joint;
	if (data->skins_count > 1)
		ERROR("[GFX] Only one of the %lu skins will be loaded for \"%s\"", data->skins_count, path);
	else if (data->skins_count) {
		skin = data->skins;
		model->skin = smalloc(skin->joints_count*sizeof(struct Joint) + sizeof(struct Skin));
		for (int j = 0; j < (int)skin->joints_count; j++) {
			joint = &model->skin->joints[j];
			node  = skin->joints[j];

			strncpy(joint->name, node->name, MAX_NAME_LENGTH);
			joint->parent = -1;
			for (int j2 = 0; j2 < (int)skin->joints_count; j2++) {
				if (skin->joints[j2] == node->parent && j2 != j) {
					joint->parent = j2;
					break;
				}
			}

			joint->transform.translation.x = node->translation[0];
			joint->transform.translation.y = node->translation[1];
			joint->transform.translation.z = node->translation[2];

			joint->transform.scale.x = node->scale[0];
			joint->transform.scale.y = node->scale[1];
			joint->transform.scale.z = node->scale[2];

			joint->transform.rotation.x = node->rotation[0];
			joint->transform.rotation.y = node->rotation[1];
			joint->transform.rotation.z = node->rotation[2];
			joint->transform.rotation.w = node->rotation[3];

			if (joint->parent != -1 && joint->parent < j) {
				glm_quat_mul(model->skin->joints[joint->parent].transform.rotation.raw,
				             joint->transform.rotation.raw, joint->transform.rotation.raw);
				glm_quat_rotatev(model->skin->joints[joint->parent].transform.rotation.raw,
				                 joint->transform.translation.raw, joint->transform.translation.raw);
				glm_vec3_add(model->skin->joints[joint->parent].transform.translation.raw,
				             joint->transform.translation.raw, joint->transform.translation.raw);
				glm_vec3_mulv(model->skin->joints[joint->parent].transform.scale.raw,
				              joint->transform.scale.raw, joint->transform.scale.raw);
			} else if (joint->parent != -1) {
				ERROR("[GFX] Assuming joints are toplogically sorted, but joint %d has parent %d", j, joint->parent);
			}

			DEBUG(1, "Joint name: %32s; parent: %d", model->skin->joints[j].name, model->skin->joints[j].parent);
		}
	}

	// for (int i = 0; i < vert_max; i++) {
	// 	DEBUG(1, "vert: %.2f, %.2f, %.2f", verts[3*i], verts[3*i + 1], verts[3*i + 2]);
	// 	DEBUG(1, "normal: %.2f, %.2f, %.2f", normals[3*i], normals[3*i + 1], normals[3*i + 2]);
	// 	DEBUG(1, "uv: %.2f, %.2f", uvs[2*i], uvs[2*i + 1]);
	// 	// DEBUG(1, "joint_ids: %hhu, %hhu, %hhu, %hhu", joint_ids[4*i], joint_ids[4*i + 1], joint_ids[4*i + 2], joint_ids[4*i + 3]);
	// 	// DEBUG(1, "joint_weights: %.2f, %.2f, %.2f, %.2f", joint_weights[4*i], joint_weights[4*i + 1], joint_weights[4*i + 2], joint_weights[4*i + 3]);
	// 	DEBUG(1, "\n");
	// }

	int total_indc = 0;
	for (int i = 0; i < model->meshc; i++) {
		// for (int j = 0; j < model->meshes[i].indc; j++)
		// 	DEBUG(1, "%hu, %hu, %hu", inds[3*j], inds[3*j+1], inds[3*j+2]);
		total_indc += model->meshes[i].indc;
	}

	sfree(verts);
	sfree(inds);
	cgltf_free(data);
	DEBUG(3, "[GFX] Loaded file \"%s\" with %d meshes (%d vertices/%d triangles) and %d animations",
	      path, model->meshc, total_indc, total_indc/3, model->animationc);
	// exit(0);
	return model;
}

void models_record_commands(VkCommandBuffer cmd_buf)
{
	struct Model* model;
	struct Mesh*  mesh;

	mat4 vp;
	camera_get_vp(vp);
	buffer_update(ubo_bufs[0], sizeof(mat4), vp);
	
	update_model_transforms(sbo_buf);

	/* Dynamic models */
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.pipeln);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.layout, 0, 1, &pipeln.dset, 0, NULL);
	for (int i = 0; i < modelc; i++) {
		model = &models[i];
		if (!model->skin)
			continue;
		buffer_update(ubo_bufs[1], model->materialc*sizeof(struct Material), model->materials);
		for (int m = 0; m < models[i].meshc; m++) {
			mesh = &model->meshes[m];
			// DEBUG(1, "[%d] Drawing %d vertices", i, models[i].meshes[m].vertc);
			vkCmdBindVertexBuffers(cmd_buf, 0, ARRAY_LEN(vertex_attrs), (VkBuffer[]){ mesh->vbos[0].buf,
			                       mesh->vbos[1].buf, mesh->vbos[2].buf, mesh->vbos[3].buf, mesh->vbos[4].buf },
			                       (VkDeviceSize[]){ 0 });
			vkCmdBindIndexBuffer(cmd_buf, mesh->ibo.buf, 0, VK_INDEX_TYPE_UINT16);
			vkCmdPushConstants(cmd_buf, pipeln.layout, pipeln.pushstages, 0, pipeln.pushsz, &mesh->materiali);
			vkCmdDrawIndexed(cmd_buf, mesh->indc, 1, 0, 0, 0);
		}
	}

	/* Static models */
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln_static.pipeln);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.layout, 0, 1, &pipeln.dset, 0, NULL);
	for (int i = 0; i < modelc; i++) {
		model = &models[i];
		if (model->skin)
			continue;
		buffer_update(ubo_bufs[1], model->materialc*sizeof(struct Material), model->materials);
		for (int m = 0; m < models[i].meshc; m++) {
			mesh = &model->meshes[m];
			// DEBUG(1, "[%d] Drawing %d vertices", i, models[i].meshes[m].indc);
			vkCmdBindVertexBuffers(cmd_buf, 0, ARRAY_LEN(vertex_attrs) - 2, (VkBuffer[]){ mesh->vbos[0].buf,
			                       mesh->vbos[1].buf, mesh->vbos[2].buf }, (VkDeviceSize[]){ 0, sizeof(VkBuffer), sizeof(VkBuffer[2]) });
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
		for (int j = 0; j < (int)ARRAY_LEN(model->meshes[i].vbos); j++)
			vbo_free(&model->meshes[i].vbos[j]);
		ibo_free(&model->meshes[i].ibo);
	}
	model->meshc = 0;
}

void models_free()
{
	for (int i = 0; i < MAX_MODELS; i++)
		model_free(i);
	pipeln_free(&pipeln);
	pipeln_free(&pipeln_static);
}

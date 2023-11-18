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

static struct PushConstants {
	int   materiali;
	float timer;
} push_constants;

static void load_materials(struct Model* model, cgltf_data* data);
static void load_meshes(struct Model* model, cgltf_data* data);
static void load_skin(struct Model* model, cgltf_data* data);
static void load_animations(struct Model* model, cgltf_data* data);

/* -------------------------------------------------------------------- */
static VkVertexInputBindingDescription vertex_binds[] = {
	/* [3:xyz][3:nnn][2:uv][4:joint ids][4:joint weights] */
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
	  .stride    = sizeof(int8[4]),
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, },
	{ .binding   = 4,
	  .stride    = sizeof(float[4]),
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, },
};
static VkVertexInputAttributeDescription vertex_attrs[] = {
	/* position */
	{ .binding  = 0,
	  .location = 0,
	  .format   = VK_FORMAT_R32G32B32_SFLOAT,
	  .offset   = 0, },
	/* normal */
	{ .binding  = 1,
	  .location = 1,
	  .format   = VK_FORMAT_R32G32B32_SFLOAT,
	  .offset   = 0, },
	/* uvs */
	{ .binding  = 2,
	  .location = 2,
	  .format   = VK_FORMAT_R32G32_SFLOAT,
	  .offset   = 0, },
	/* bone IDs */
	{ .binding  = 3,
	  .location = 3,
	  .format   = VK_FORMAT_R8G8B8A8_SINT,
	  .offset   = 0, },
	/* bone weights */
	{ .binding  = 4,
	  .location = 4,
	  .format   = VK_FORMAT_R32G32B32A32_SFLOAT,
	  .offset   = 0, },
};
/* -------------------------------------------------------------------- */

void (*update_model_transforms)(SBO);
mat4s* model_transforms;

static struct Pipeline pipeln;
static struct Pipeline pipeln_static;
static struct Model models[MAX_MODELS];
static intptr modelc = 0;
static UBO ubo_bufs[3];
static SBO sbo_buf;

void models_init(VkRenderPass render_pass)
{
	sbo_buf     = sbo_new(MAX_MODELS*sizeof(mat4));
	ubo_bufs[0] = ubo_new(sizeof(mat4));                           /* Camera matrix */
	ubo_bufs[1] = ubo_new(MAX_MATERIALS*sizeof(struct Material));  /* Material data */
	ubo_bufs[2] = ubo_new(MAX_JOINTS*sizeof(struct Transform));    /* Joint data    */

	pipeln = (struct Pipeline){
		.vshader    = create_shader(SHADER_DIR "model.vert"),
		.fshader    = create_shader(SHADER_DIR "model.frag"),
		.vertbindc  = ARRAY_SIZE(vertex_binds),
		.vertbinds  = vertex_binds,
		.vertattrc  = ARRAY_SIZE(vertex_attrs),
		.vertattrs  = vertex_attrs,
		.uboc       = ARRAY_SIZE(ubo_bufs),
		.ubos       = ubo_bufs,
		.sbo        = &sbo_buf,
		.sbosz      = MAX_MODELS*sizeof(mat4),
		.pushstages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		.pushsz     = sizeof(struct PushConstants),
	};
	pipeln_init(&pipeln, render_pass);

	pipeln_static = (struct Pipeline){
		.vshader    = create_shader(SHADER_DIR "model_static.vert"),
		.fshader    = create_shader(SHADER_DIR "model.frag"),
		.vertbindc  = ARRAY_SIZE(vertex_binds) - 2,
		.vertbinds  = vertex_binds,
		.vertattrc  = ARRAY_SIZE(vertex_attrs) - 2,
		.vertattrs  = vertex_attrs,
		.uboc       = 2,
		.ubos       = ubo_bufs,
		.sbo        = &sbo_buf,
		.sbosz      = MAX_MODELS*sizeof(mat4),
		.pushstages = VK_SHADER_STAGE_FRAGMENT_BIT,
		.pushsz     = sizeof(struct PushConstants),
	};
	pipeln_init(&pipeln_static, render_pass);
}

inline static int float_compare(const void* restrict f1, const void* restrict f2) {
	return *(float*)f1 > *(float*)f2? 1: *(float*)f2 > *(float*)f1? -1: 0;
}

// TODO: this doesnt work properly for removing elements
struct Model* model_new(char* path)
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

	load_materials(model, data);
	load_meshes(model, data);
	load_skin(model, data);
	load_animations(model, data);

#if DEBUG_LEVEL > 0
	int total_indc = 0;
	for (int i = 0; i < model->meshc; i++)
		total_indc += model->meshes[i].indc;

	int total_framec = 0;
	for (int i = 0; i < model->animationc; i++)
		total_framec += model->animations[i].framec;
#endif

	model->current_animation = 0;
	model->timer = 0.0;

	cgltf_free(data);
	DEBUG(3, "[GFX] Loaded file \"%s\" with %d meshes (%d vertices/%d triangles) and %d animations (%d joints) with %d total frames",
	      path, model->meshc, total_indc, total_indc/3, model->animationc, model->skin->jointc, total_framec);
	return model;
}

void models_update()
{
	struct Model*     model;
	struct Animation* animation;
	for (int m = 0; m < modelc; m++) {
		model     = &models[m];
		animation = &model->animations[model->current_animation];
		model->timer += dt;
		if (model->timer >= animation->times[animation->current_frame])
			animation->current_frame++;

		if (animation->current_frame >= animation->framec) {
			animation->current_frame = 0;
			model->timer = 0;
		}
	}
}

void models_record_commands(VkCommandBuffer cmd_buf)
{
	struct Model*     model;
	struct Mesh*      mesh;
	struct Animation* animation;

	mat4 vp;
	camera_get_vp(vp);
	buffer_update(ubo_bufs[0], sizeof(mat4), vp);
	
	update_model_transforms(sbo_buf);

	/* Animated models */
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.pipeln);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.layout, 0, 1, &pipeln.dset, 0, NULL);
	for (int i = 0; i < modelc; i++) {
		model     = &models[i];
		animation = &model->animations[model->current_animation];
		if (!model->skin)
			continue;
		buffer_update(ubo_bufs[1], model->materialc*sizeof(struct Material), model->materials);
		buffer_update(ubo_bufs[2], animation->framec*sizeof(struct Transform),
		              animation->frames[animation->current_frame].transforms);
		for (int m = 0; m < models[i].meshc; m++) {
			mesh = &model->meshes[m];
			push_constants.materiali = mesh->materiali;
			push_constants.timer     = model->timer/3.0;
			// DEBUG(1, "[%d Animated] Drawing %d vertices", i, models[i].meshes[m].vertc);

			vkCmdBindVertexBuffers(cmd_buf, 0, ARRAY_SIZE(vertex_attrs), (VkBuffer[]){ mesh->vbos[0].buf,
			                       mesh->vbos[1].buf, mesh->vbos[2].buf, mesh->vbos[3].buf, mesh->vbos[4].buf },
			                       (VkDeviceSize[]){ 0, 0, 0, 0, 0 });
			vkCmdBindIndexBuffer(cmd_buf, mesh->ibo.buf, 0, VK_INDEX_TYPE_UINT16);
			vkCmdPushConstants(cmd_buf, pipeln.layout, pipeln.pushstages, 0, pipeln.pushsz, &push_constants);
			vkCmdDrawIndexed(cmd_buf, mesh->indc, 1, 0, 0, 0);
		}
	}

	/* Static models */
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln_static.pipeln);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln_static.layout, 0, 1, &pipeln_static.dset, 0, NULL);
	for (int i = 0; i < modelc; i++) {
		model = &models[i];
		if (model->skin)
			continue;
		buffer_update(ubo_bufs[1], model->materialc*sizeof(struct Material), model->materials);
		for (int m = 0; m < models[i].meshc; m++) {
			mesh = &model->meshes[m];
			push_constants.materiali = mesh->materiali;
			// DEBUG(1, "[%d Static] Drawing %d vertices", i, models[i].meshes[m].indc);

			vkCmdBindVertexBuffers(cmd_buf, 0, ARRAY_SIZE(vertex_attrs) - 2, (VkBuffer[]){ mesh->vbos[0].buf,
			                       mesh->vbos[1].buf, mesh->vbos[2].buf }, (VkDeviceSize[]){ 0, sizeof(VkBuffer), sizeof(VkBuffer[2]) });
			vkCmdBindIndexBuffer(cmd_buf, mesh->ibo.buf, 0, VK_INDEX_TYPE_UINT16);
			vkCmdPushConstants(cmd_buf, pipeln_static.layout, pipeln_static.pushstages, 0, pipeln_static.pushsz, &push_constants);
			vkCmdDrawIndexed(cmd_buf, mesh->indc, 1, 0, 0, 0);
		}
	}
}

void model_free(ID model_id)
{
	struct Model* model = &models[model_id];
	for (int i = 0; i < model->meshc; i++) {
		for (int j = 0; j < (int)ARRAY_SIZE(model->meshes[i].vbos); j++)
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

static void load_materials(struct Model* model, cgltf_data* data)
{
	model->materialc = data->materials_count? data->materials_count: 1;
	model->materials = scalloc(model->materialc, sizeof(struct Material));
	if (model->materialc > MAX_MATERIALS)
		ERROR("[GFX] Model has %d materials. MAX_MATERIALS is set to %d", model->materialc, MAX_MATERIALS);

	cgltf_material*     material;
	// cgltf_image*        img;
	// cgltf_texture_view* texture;
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
}

static void load_meshes(struct Model* model, cgltf_data* data)
{
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
	float* verts = smalloc(vert_max*(sizeof(float[12]) + sizeof(int8[4])));
	float* normals       = verts   + 3*vert_max;
	float* uvs           = normals + 3*vert_max;
	int8*  joint_ids     = (int8*)(uvs + 2*vert_max);
	float* joint_weights = (float*)(joint_ids + 4*vert_max);

	cgltf_mesh*      mesh;
	cgltf_primitive* prim;
	cgltf_accessor*  attr;
	float*  vert;
	int8*   vert8;
	uint16* ind_16;
	uint32* ind_32;
	for (uint m = 0; m < data->meshes_count; m++) {
		mesh = &data->meshes[m];
		for (int p = 0; p < (int)mesh->primitives_count; p++) {
			prim = &mesh->primitives[p];
			assert(prim->type == cgltf_primitive_type_triangles);
			for (int a = 0; a < (int)prim->attributes_count; a++) {
				attr = prim->attributes[a].data;
				vert  = (float*)attr->buffer_view->buffer->data + attr->buffer_view->offset/sizeof(float) + attr->offset/sizeof(float);
				vert8 = (int8*)attr->buffer_view->buffer->data + attr->buffer_view->offset/sizeof(int8) + attr->offset/sizeof(int8);
				int i = 0;
				for (int v = 0; v < (int)attr->count; v++) {
					switch(prim->attributes[a].type) {
						case cgltf_attribute_type_position:
							verts[3*v + 0] = vert[i + 0];
							verts[3*v + 1] = vert[i + 1];
							verts[3*v + 2] = vert[i + 2];
							i += (int)(attr->stride/sizeof(float));
							break;
						case cgltf_attribute_type_normal:
							normals[3*v + 0] = vert[i + 0];
							normals[3*v + 1] = vert[i + 1];
							normals[3*v + 2] = vert[i + 2];
							i += (int)(attr->stride/sizeof(float));
							break;
						case cgltf_attribute_type_texcoord:
							uvs[2*v + 0] = vert[i + 0];
							uvs[2*v + 1] = vert[i + 1];
							break;
						case cgltf_attribute_type_joints:
							joint_ids[4*v + 0] = vert8[i + 0];
							joint_ids[4*v + 1] = vert8[i + 1];
							joint_ids[4*v + 2] = vert8[i + 2];
							joint_ids[4*v + 3] = vert8[i + 3];
							i += (int)(attr->stride/sizeof(int8));
							DEBUG_VALUE(joint_ids[4*v + 0]);
							DEBUG_VALUE(joint_ids[4*v + 1]);
							DEBUG_VALUE(joint_ids[4*v + 2]);
							DEBUG_VALUE(joint_ids[4*v + 3]);
							DEBUG(1, " ");
							break;
						case cgltf_attribute_type_weights:
							joint_weights[4*v + 0] = vert[i + 0];
							joint_weights[4*v + 1] = vert[i + 1];
							joint_weights[4*v + 2] = vert[i + 2];
							joint_weights[4*v + 3] = vert[i + 3];
							i += (int)(attr->stride/sizeof(float));
							DEBUG_VALUE(joint_weights[4*v + 0]);
							DEBUG_VALUE(joint_weights[4*v + 1]);
							DEBUG_VALUE(joint_weights[4*v + 2]);
							DEBUG_VALUE(joint_weights[4*v + 3]);
							DEBUG(1, " ");
							break;
						default:
							ERROR("[GFX] Invalid attribute type: %d", prim->attributes[a].type);
							continue;
					}
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
						models->meshes[m].vbos[3] = vbo_new(attr->count*sizeof(int8[4]), verts);
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

	sfree(inds);
	sfree(verts);
}

static void load_skin(struct Model* model, cgltf_data* data)
{
	cgltf_skin* skin;
	cgltf_node* node;
	struct Joint* joint;
	if (data->skins_count > 1)
		ERROR("[GFX] Warning: only one of the %lu skins will be loaded", data->skins_count);
	else if (data->skins_count) {
		skin = data->skins;
		model->skin = smalloc(skin->joints_count*sizeof(struct Joint) + sizeof(struct Skin));
		model->skin->jointc = (int)skin->joints_count;
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

			// DEBUG(1, "Joint name: %32s; parent: %d", model->skin->joints[j].name, model->skin->joints[j].parent);
		}
	}

}

static void load_animations(struct Model* model, cgltf_data* data)
{
	DEBUG(1, "\n-------------------------------------------------------------------------------");
	model->animationc = data->animations_count;
	model->animations = smalloc(model->animationc*sizeof(struct Animation));
	DEBUG(1, "Animations: %d", model->animationc);

	// struct {
	// 	cgltf_animation_path_type type;
	// 	cgltf_animation_channel*  channel;
	// }* transforms;
	cgltf_animation*         animation;
	cgltf_animation_channel* channel;
	cgltf_animation_sampler* sampler;
	// cgltf_accessor*          input;
	// cgltf_accessor*          output;
	cgltf_skin* skin = &data->skins[0];
	for (int a = 0; a < (int)data->animations_count; a++) {
		animation = &data->animations[a];
		assert(animation->samplers_count == animation->channels_count);
		strncpy(model->animations[a].name, animation->name, MAX_NAME_LENGTH);
		DEBUG(1, " *** %s *** channels: %lu; samplers: %lu", model->animations[a].name, animation->channels_count, animation->samplers_count);

		// TODO: calculate the number of joints modifies
		/* First calculate the number of frames to be allocated for */
		int framec = 0;
		float time;
		float times[MAX_ANIMATION_FRAMES] = { 0 };
		for (int c = 0; c < (int)animation->channels_count; c++) {
			channel = &animation->channels[c];
			sampler = channel->sampler;

			/* Count how many frames for the animation */
			for (int t = 0; t < (int)sampler->input->count; t++) {
				if (!cgltf_accessor_read_float(sampler->input, t, &time, 1))
					ERROR("[GFX] Error reading from accessor %d", t);
				for (int f = 0; f < framec; f++) {
					if (fabs(time - times[f]) <= FLT_EPSILON)
						goto skip_duplicate_time;
				}

				times[framec++] = time;
				skip_duplicate_time:
			}
			if (framec > MAX_ANIMATION_FRAMES)
				goto skip_animation;
		}

		DEBUG(1, "Animation %d of %d has %d frames", a + 1, model->animationc, framec);

		model->animations[a].framec = framec;
		model->animations[a].frames = smalloc(framec*sizeof(struct AnimationFrame) + framec*sizeof(float));
		model->animations[a].times  = (float*)(model->animations[a].frames + framec);
		for (int f = 0; f < framec; f++)
			model->animations[a].frames[f].transforms = smalloc(MAX_JOINTS*sizeof(struct Transform));

		// TODO: own sorting function
		memcpy(model->animations[a].times, times, framec*sizeof(float));
		qsort(model->animations[a].times, model->animations[a].framec, sizeof(float), float_compare);
		fprintf(stderr, "Frame times: ");
		for (int t = 0; t < model->animations[a].framec; t++)
			fprintf(stderr, "%f, ", model->animations[a].times[t]);
		fprintf(stderr, "\n");

		/* Load the actual tranforms */
		struct AnimationFrame* frame;
		float* target;
		intptr size;
		for (int c = 0; c < (int)animation->channels_count; c++) {
			channel = &animation->channels[c];
			sampler = channel->sampler;

			/* Find which joint this belongs to */
			int joint_index = -1;
			for (int j = 0; j < (int)skin->joints_count; j++) {
				if (skin->joints[j] == channel->target_node) {
					joint_index = j;
					break;
				}
			}
			if (joint_index == -1)
				ERROR("[GFX] Animation channel %d is not in the skin", c);

			/* Load each transform */
			for (int t = 0; t < (int)sampler->input->count; t++) {
				/* Find which frame this is for using the time */
				if (!cgltf_accessor_read_float(sampler->input, t, &time, 1))
					ERROR("[GFX] Error reading from accessor");
				frame = NULL;
				for (int f = 0; f < model->animations[a].framec; f++) {
					if (fabs(model->animations[a].times[f] - time) <= FLT_EPSILON) {
						frame = &model->animations[a].frames[f];
						break;
					}
				}
				if (!frame) {
					ERROR("[GFX] Skipping accessor %d for time %f", t, time);
					continue;
				}
				// DEBUG(1, "Loading accessor with taget_path = %d for time %f", channel->target_path, time);

				if (sampler->output->type == cgltf_type_vec3 || sampler->output->type == cgltf_type_vec4) {
					switch (channel->target_path) {
						case cgltf_animation_path_type_translation:
							target = frame->transforms[joint_index].translation.raw;
							size   = 3;
							break;
						case cgltf_animation_path_type_rotation:
							target = frame->transforms[joint_index].rotation.raw;
							size   = 4;
							break;
						case cgltf_animation_path_type_scale:
							target = frame->transforms[joint_index].scale.raw;
							size   = 3;
							break;
						default:
							ERROR("[GFX] Invalid target path for animation %d", channel->target_path);
							target = NULL;
							size   = 0;
					}
					// DEBUG(1, "joint_index: %d; size: %ld", joint_index, size);
					if (!cgltf_accessor_read_float(sampler->output, t, target, size))
						ERROR("[GFX] Error reading from accessor %d", t);
				} else {
					ERROR("[GFX] Accessor has invalid type: %u", sampler->output->type);
				}
				// DEBUG(1, " ");
			}
		}
	skip_animation:
	}
}

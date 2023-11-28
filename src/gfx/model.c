#include "vulkan/vulkan.h"
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#include "stb_image.h"

#include "util/arena.h"
#include "util/file.h"
#include "vulkan.h"
#include "buffers.h"
#include "pipeline.h"
#include "renderer.h"
#include "camera.h"
#include "model.h"

#define LINE_BUFFER_SIZE 256

static struct PushConstants {
	int   materiali;
	float timer;
} push_constants;

static void load_materials(struct Model* model, cgltf_data* data);
static struct Texture load_material_image(cgltf_image* img);
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
struct Material default_material;

static struct Pipeline pipeln;
static struct Pipeline pipeln_static;
static struct Model models[MAX_MODELS];
static intptr modelc = 0;
static UBO ubo_bufs[4];
static SBO sbo_buf;

void models_init(VkRenderPass renderpass)
{
	default_material = (struct Material){
		.albedo    = { 0.3f, 0.3f, 0.3f, 1.0f },
		.metallic  = 1.0f,
		.roughness = 1.0f,
		.tex       = texture_new_from_image(TEXTURE_PATH "default.png"),
	};

	sbo_buf     = sbo_new(MAX_MODELS*sizeof(mat4));
	ubo_bufs[0] = ubo_new(sizeof(mat4));                          /* Camera matrix */
	ubo_bufs[1] = ubo_new(MAX_MATERIALS*sizeof(struct Material)); /* Material data */
	ubo_bufs[2] = ubo_new(sizeof(struct GlobalLighting));         /* Light data    */
	ubo_bufs[3] = ubo_new(MAX_JOINTS*sizeof(struct Transform));   /* Joint data    */

	pipeln = (struct Pipeline){
		.vshader     = create_shader(SHADER_DIR "model.vert"),
		.fshader     = create_shader(SHADER_DIR "model.frag"),
		.vert_bindc  = 5,
		.vert_binds  = vertex_binds,
		.vert_attrc  = 5,
		.vert_attrs  = vertex_attrs,
		.push_stages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		.push_sz     = sizeof(struct PushConstants),
		.sboc        = 1,
		.sbo_szs     = (isize[]){ MAX_MODELS*sizeof(mat4) },
		.sbos        = (SBO[]){ sbo_buf },
		.uboc        = 4,
		.ubos        = ubo_bufs,
		.imgc        = 1,
		.dset_cap    = 2,
	};
	pipeln_static = (struct Pipeline){
		.vshader     = create_shader(SHADER_DIR "model_static.vert"),
		.fshader     = create_shader(SHADER_DIR "model.frag"),
		.vert_bindc  = 3,
		.vert_binds  = vertex_binds,
		.vert_attrc  = 3,
		.vert_attrs  = vertex_attrs,
		.push_stages = VK_SHADER_STAGE_FRAGMENT_BIT,
		.push_sz     = sizeof(struct PushConstants),
		.sboc        = 1,
		.sbo_szs     = (isize[]){ MAX_MODELS*sizeof(mat4) },
		.sbos        = (SBO[]){ sbo_buf },
		.uboc        = 3,
		.ubos        = ubo_bufs,
		.imgc        = 1,
		.dset_cap    = 2,
	};
	pipeln_alloc_dsets(&pipeln);
	pipeln_alloc_dsets(&pipeln_static);
	VkImageView img_view;
	for (int i = 0; i < modelc; i++) {
		for (int j = 0; j < models[i].materialc; j++) {
			if (models[i].materials[j].tex.image)
				img_view = models[i].materials[j].tex.image_view;
			else
				img_view = models[i].materials[j].tex.image_view;
			if (!img_view)
				img_view = default_material.tex.image_view;
			models[i].materials[j].dset = pipeln_create_image_dset(&pipeln_static, 1, &img_view);
		}
	}
	pipeln_init(&pipeln, renderpass);
	pipeln_init(&pipeln_static, renderpass);
}

inline static int float_compare(const void* restrict f1, const void* restrict f2) {
	return *(float*)f1 > *(float*)f2? 1: *(float*)f2 > *(float*)f1? -1: 0;
}

// TODO: this doesnt work properly for removing elements
struct Model* model_new(char* path)
{
	struct Model* model = &models[modelc++];
	*model = (struct Model){ 0 };

	cgltf_options options = {
		.type = cgltf_file_type_glb,
	};
	cgltf_data* data;
	int err;
	if ((err = cgltf_parse_file(&options, path, &data))) {
		ERROR("[GFX] CGLTF failed to parse file \"%s\": %d", path, err);
	} else {
		DEBUG(4, "[GFX] Model data for \"%s\" (%zuB / %.2fkB) loaded with:", path, data->json_size, data->json_size/1024.0);
		DEBUG(4, "\tMeshes    -> %lu", data->meshes_count);
		DEBUG(4, "\tMaterials -> %lu", data->materials_count);
		DEBUG(4, "\tTextures  -> %lu", data->textures_count);
		DEBUG(4, "\tImages    -> %lu", data->images_count);
		DEBUG(4, "\tBuffers   -> %lu", data->buffers_count);
		DEBUG(4, "\tSkins     -> %lu", data->skins_count);
	}

	if ((err = cgltf_validate(data)))
		ERROR("[RES] GLTF file \"%s\" fails `cgltf_validate`: %d", path, err);

	if ((err = cgltf_load_buffers(&options, data, path)))
		ERROR("[GFX] Error loading buffer data: %d", err);
	else
		DEBUG(4, "Loaded buffer data for \"%s\" (%zuB / %.2fkB)", path, data->bin_size, data->bin_size/1024.0);

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

	cgltf_free(data);
	DEBUG(3, "[GFX] Loaded file \"%s\" with %d meshes (%d vertices/%d triangles) and %d animations (%d joints) with %d total frames",
	      path, model->meshc, total_indc, total_indc/3, model->animationc, model->skin? model->skin->jointc: 0, total_framec);
	exit(0);
	return model;
}

void models_update()
{
	// struct Model*     model;
	// struct Animation* animation;
	// for (int m = 0; m < modelc; m++) {
	// 	model     = &models[m];
	// 	animation = &model->animations[model->current_animation];
	// 	model->timer += DT;
	// 	if (model->timer >= animation->times[animation->current_frame])
	// 		animation->current_frame++;

	// 	if (animation->current_frame >= animation->framec) {
	// 		animation->current_frame = 0;
	// 		model->timer = 0;
	// 	}
	// }
}

void models_record_commands(VkCommandBuffer cmd_buf)
{
	struct Model*     model;
	struct Mesh*      mesh;
	// struct Animation* animation;

	mat4 vp;
	camera_get_vp(vp);
	// TODO: Named buffers, not arrays
	buffer_update(ubo_bufs[0], sizeof(mat4), vp);
	
	update_model_transforms(sbo_buf);

	/* Animated models */
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.pipeln);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.layout, 0, 1, pipeln.dset, 0, NULL);
	for (int i = 0; i < modelc; i++) {
		model     = &models[i];
		// animation = &model->animations[model->current_animation];
		if (!model->skin)
			continue;
		buffer_update(ubo_bufs[1], model->materialc*sizeof(struct Material), model->materials);
		buffer_update(ubo_bufs[2], sizeof(struct GlobalLighting), &global_light);
		// buffer_update(ubo_bufs[2], animation->framec*sizeof(struct Transform),
		//               animation->frames[animation->current_frame].transforms);
		for (int m = 0; m < models[i].meshc; m++) {
			mesh = &model->meshes[m];
			push_constants.materiali = mesh->materiali;
			push_constants.timer     = model->timer/3.0;
			vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln_static.layout, 1,
			                        1, &model->materials[mesh->materiali].dset, 0, NULL);
			// DEBUG(1, "[%d Animated] Drawing %d vertices", i, models[i].meshes[m].vertc);

			// TODO: these vbos need to be named
			vkCmdBindVertexBuffers(cmd_buf, 0, 5, (VkBuffer[]){ mesh->vbos[0].buf,
			                       mesh->vbos[1].buf, mesh->vbos[3].buf, mesh->vbos[4].buf, mesh->vbos[2].buf },
			                       (VkDeviceSize[]){ 0, 0, 0, 0, 0 });
			vkCmdBindIndexBuffer(cmd_buf, mesh->ibo.buf, 0, VK_INDEX_TYPE_UINT16);
			vkCmdPushConstants(cmd_buf, pipeln.layout, pipeln.push_stages, 0, pipeln.push_sz, &push_constants);
			vkCmdDrawIndexed(cmd_buf, mesh->indc, 1, 0, 0, 0);
		}
	}

	/* Static models */
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln_static.pipeln);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln_static.layout, 0, 1, pipeln_static.dset, 0, NULL);
	for (int i = 0; i < modelc; i++) {
		model = &models[i];
		if (model->skin)
			continue;
		buffer_update(ubo_bufs[1], model->materialc*sizeof(struct Material), model->materials);
		buffer_update(ubo_bufs[2], sizeof(struct GlobalLighting), &global_light);

		for (int m = 0; m < models[i].meshc; m++) {
			mesh = &model->meshes[m];
			push_constants.materiali = mesh->materiali;
			vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln_static.layout, 1,
			                        1, &model->materials[mesh->materiali].dset, 0, NULL);
			// DEBUG(1, "[%d Static] Drawing %d vertices", i, models[i].meshes[m].indc);

			vkCmdBindVertexBuffers(cmd_buf, 0, 3, (VkBuffer[]){ mesh->vbos[0].buf,
			                       mesh->vbos[1].buf, mesh->vbos[2].buf }, (VkDeviceSize[]){ 0, 0, 0 });
			vkCmdBindIndexBuffer(cmd_buf, mesh->ibo.buf, 0, VK_INDEX_TYPE_UINT16);
			vkCmdPushConstants(cmd_buf, pipeln_static.layout, pipeln_static.push_stages, 0, pipeln_static.push_sz, &push_constants);
			vkCmdDrawIndexed(cmd_buf, mesh->indc, 1, 0, 0, 0);
		}
	}
}

void model_free(ID model_id)
{
	struct Model* model = &models[model_id];
	for (int i = 0; i < model->meshc; i++) {
		/* Static Meshes do not use the last two vbos */
		for (int j = 0; j < MODEL_MESH_VBO_COUNT - (model->animationc > 0? 0: 2); j++)
			vbo_free(&model->meshes[i].vbos[j]);
		ibo_free(&model->meshes[i].ibo);
	}

	texture_free(&default_material.tex);
	for (int i = 0; i < model->materialc; i++)
		if (model->materials[i].tex.image)
			texture_free(&model->materials[i].tex);
	model->meshc = 0;
}

void models_free()
{
	for (int i = 0; i < modelc; i++)
		model_free(i);

	sbo_free(&sbo_buf);
	for (int i = 0; i < (int)ARRAY_SIZE(ubo_bufs); i++)
		ubo_free(&ubo_bufs[i]);

	pipeln_free(&pipeln);
	pipeln_free(&pipeln_static);
}

/* -------------------------------------------------------------------- */

static void load_materials(struct Model* model, cgltf_data* data)
{
	model->materialc = data->materials_count? data->materials_count: 1;
	model->materials = scalloc(model->materialc, sizeof(struct Material));
	if (model->materialc > MAX_MATERIALS)
		ERROR("[RES] Model has %d materials. MAX_MATERIALS is set to %d", model->materialc, MAX_MATERIALS);

	cgltf_material* material;
	for (int m = 0; m < (int)data->materials_count; m++) {
		material = &data->materials[m];
		memcpy(model->materials[m].albedo, material->pbr_metallic_roughness.base_color_factor, sizeof(float[4]));

		model->materials[m].metallic  = material->pbr_metallic_roughness.metallic_factor;
		model->materials[m].roughness = material->pbr_metallic_roughness.roughness_factor;
		if (!material->has_pbr_metallic_roughness) {
			ERROR("[RES] Only metallic-roughness materials are supported");
			continue;
		}

		if (material->pbr_metallic_roughness.base_color_texture.texture)
			model->materials[m].tex = load_material_image(material->pbr_metallic_roughness.base_color_texture.texture->image);
		if (material->pbr_metallic_roughness.metallic_roughness_texture.texture)
			ERROR("[GFX] Ignoring metallic roughness texture");
		if (material->normal_texture.texture)
			ERROR("[GFX] Ignoring normal texture");
		if (material->occlusion_texture.texture)
			ERROR("[GFX] Ignoring occlusion texture");
		if (material->emissive_texture.texture)
			ERROR("[RES] Ignoring emissive texture");

		DEBUG(5, "[RES] Loaded material %d with: %s%s%s%s%s", m,
		      material->pbr_metallic_roughness.base_color_texture.texture        ? "base colour texture, "       : "",
		      material->pbr_metallic_roughness.metallic_roughness_texture.texture? "metallic roughness texture, ": "",
		      material->normal_texture.texture   ? "normal texture, "   : "",
		      material->occlusion_texture.texture? "occlusion texture, ": "",
		      material->emissive_texture.texture ? "emissive texture"   : "");
		DEBUG(5, "\tAlbedo    -> %.2f, %.2f, %.2f, %.2f", UNPACK4(model->materials[m].albedo));
		DEBUG(5, "\tMetallic  -> %.2f", model->materials[m].metallic);
		DEBUG(5, "\tRoughness -> %.2f", model->materials[m].roughness);
	}

	/* Default material if none exist */
	if (!data->materials_count) {
		models->materialc    = 1;
		models->materials[0] = default_material;
	}
}

static struct Texture load_material_image(cgltf_image* img)
{
	if (img->uri) {
		ERROR("[RES] Image loading from URIs not implemented");
		return (struct Texture){ 0 };
	} else if (img->buffer_view->buffer->data) {
		if (!strncmp(img->mime_type, "image/png", sizeof("image/png"))) {
			int w, h, ch;
			byte* pxs = stbi_load_from_memory((byte*)img->buffer_view->buffer->data + img->buffer_view->offset,
			                                  img->buffer_view->size, &w, &h, &ch, 4);
			return texture_new(pxs, w, h);
		} else {
			ERROR("[RES] MIME type not supported for image: %s", img->mime_type);
		}
	}
	
	ERROR("[RES] Image does not have either URI or buffer data");
	return (struct Texture){ 0 };
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

	/* Add extra 2* for arena alignment */
	struct Arena* mesh_mem = arena_new(2*indc_max*sizeof(uint16) + vert_max*(sizeof(float[12]) + sizeof(int8[4])), 0);
	uint16* inds         = arena_alloc(mesh_mem, indc_max*sizeof(uint16));
	float* verts         = arena_alloc(mesh_mem, vert_max*sizeof(float[3]));
	float* normals       = arena_alloc(mesh_mem, vert_max*sizeof(float[3]));
	float* uvs           = arena_alloc(mesh_mem, vert_max*sizeof(float[2]));
	int8*  joint_ids     = arena_alloc(mesh_mem, vert_max*sizeof(uint8[4]));
	float* joint_weights = arena_alloc(mesh_mem, vert_max*sizeof(float[4]));

	cgltf_mesh*      mesh;
	cgltf_primitive* prim;
	cgltf_accessor*  attr;
	float*  vert;
	uint8*  uvert8;
	uint16* uvert16;
	uint16* ind_16;
	uint32* ind_32;
	for (uint m = 0; m < data->meshes_count; m++) {
		mesh = &data->meshes[m];
		for (int p = 0; p < (int)mesh->primitives_count; p++) {
			prim = &mesh->primitives[p];
			if (prim->type != cgltf_primitive_type_triangles)
				ERROR("[RES] Mesh has non-triangle primitives (%d)", prim->type);

			for (int a = 0; a < (int)prim->attributes_count; a++) {
				attr    = prim->attributes[a].data;
				vert    = (float*)attr->buffer_view->buffer->data + attr->buffer_view->offset/sizeof(float) + attr->offset/sizeof(float);
				uvert8  = (uint8*)attr->buffer_view->buffer->data + attr->buffer_view->offset/sizeof(uint8) + attr->offset/sizeof(uint8);
				uvert16 = (uint16*)attr->buffer_view->buffer->data + attr->buffer_view->offset/sizeof(uint16) + attr->offset/sizeof(uint16);
				int i = 0;
				for (int v = 0; v < (int)attr->count; v++) {
					switch(prim->attributes[a].type) {
					case cgltf_attribute_type_position:
						if (attr->component_type != cgltf_component_type_r_32f || attr->type != cgltf_type_vec3)
							ERROR("[RES] Position attribute has invalid type (expect vec3 (%d vs %d) float (%d vs %d)):",
							      cgltf_type_vec3, attr->type, cgltf_component_type_r_32f, attr->component_type);
						verts[3*v + 0] = vert[i + 0];
						verts[3*v + 1] = vert[i + 1];
						verts[3*v + 2] = vert[i + 2];
						i += (int)(attr->stride/sizeof(float));
						break;
					case cgltf_attribute_type_normal:
						if (attr->component_type != cgltf_component_type_r_32f || attr->type != cgltf_type_vec3)
							ERROR("[RES] Normal attribute has invalid type (expect vec3 (%d vs %d) float (%d vs %d)):",
							      cgltf_type_vec3, attr->type, cgltf_component_type_r_32f, attr->component_type);
						normals[3*v + 0] = vert[i + 0];
						normals[3*v + 1] = vert[i + 1];
						normals[3*v + 2] = vert[i + 2];
						i += (int)(attr->stride/sizeof(float));
						break;
					case cgltf_attribute_type_texcoord:
						if (attr->component_type != cgltf_component_type_r_32f || attr->type != cgltf_type_vec2)
							ERROR("[RES] Texcoord attribute has invalid type (expect vec2 (%d vs %d) float (%d vs %d)):",
							      cgltf_type_vec2, attr->type, cgltf_component_type_r_32f, attr->component_type);
						uvs[2*v + 0] = vert[i + 0];
						uvs[2*v + 1] = vert[i + 1];
						i += (int)(attr->stride/sizeof(float));
						break;
					case cgltf_attribute_type_joints:
						if (attr->type != cgltf_type_vec4)
							ERROR("[RES] Joint attribute has unsupported type (expect vec4 (%d) -> got (%d)",
							      cgltf_type_vec4, attr->type);
						if (attr->component_type == cgltf_component_type_r_8u) {
							joint_ids[4*v + 0] = uvert8[i + 0];
							joint_ids[4*v + 1] = uvert8[i + 1];
							joint_ids[4*v + 2] = uvert8[i + 2];
							joint_ids[4*v + 3] = uvert8[i + 3];
							i += (int)(attr->stride/sizeof(uint8));
						} else if (attr->component_type == cgltf_component_type_r_16u) {
							joint_ids[4*v + 0] = uvert16[i + 0];
							joint_ids[4*v + 1] = uvert16[i + 1];
							joint_ids[4*v + 2] = uvert16[i + 2];
							joint_ids[4*v + 3] = uvert16[i + 3];
							i += (int)(attr->stride/sizeof(uint16));
						} else {
							ERROR("[RES] Joint attribute has unsupported component type (expect u8 (%d) or u16 (%d) -> got %d",
							      cgltf_component_type_r_8u, cgltf_component_type_r_16u, attr->component_type);
						}
						break;
					case cgltf_attribute_type_weights:
						if (attr->component_type != cgltf_component_type_r_32f || attr->type != cgltf_type_vec4)
							ERROR("[RES] Weight attribute has invalid type (expect vec4 (%d vs %d) float (%d vs %d)):",
							      cgltf_type_vec4, attr->type, cgltf_component_type_r_32f, attr->component_type);
						joint_weights[4*v + 0] = vert[i + 0];
						joint_weights[4*v + 1] = vert[i + 1];
						joint_weights[4*v + 2] = vert[i + 2];
						joint_weights[4*v + 3] = vert[i + 3];
						// DEBUG(1, "[%d] %.2f, %.2f, %.2f, %.2f", v, joint_weights[4*v + 0], joint_weights[4*v + 1], joint_weights[4*v + 2], joint_weights[4*v + 3]);
						i += (int)(attr->stride/sizeof(float));
						break;
					case cgltf_attribute_type_tangent:
						static bool tangent_err = false;
						if (!tangent_err) {
							ERROR("[GFX] Tangent loading from GLTF files not implemented");
							tangent_err = true;
						}
						break;
					case cgltf_attribute_type_color:
						static bool colour_err = false;
						if (!colour_err) {
							ERROR("[GFX] Colour loading from GLTF files not implemented");
							colour_err = true;
						}
						break;
					default:
						ERROR("[GFX] Invalid attribute type: %d", prim->attributes[a].type);
						continue;
					}
				}

				/*** Apply the transform to the mesh ***/
				cgltf_node* node;
				mat4 trans;
				float* vts; /* vts = vertex type that the transform will be applied to */
				if (prim->attributes[a].type == cgltf_attribute_type_position)
					vts = verts;
				else if (prim->attributes[a].type == cgltf_attribute_type_normal)
					vts = normals;
				else
					goto skip_transform;

				for (int n = 0; n < (int)data->nodes_count; n++) {
					node = &data->nodes[n];
					cgltf_node_transform_world(node, (float*)trans);
					// DEBUG(1, "rot: %d, trans: %d, scale: %d", node->has_rotation, node->has_translation, node->has_scale);
					// DEBUG(1, "rotation: %.2f, %.2f, %.2f, %.2f", node->rotation[0], node->rotation[1], node->rotation[2], node->rotation[3]);
					// DEBUG(1, "translation: %.2f, %.2f, %.2f", node->translation[0], node->translation[1], node->translation[2]);
					// DEBUG(1, "scale: %.2f, %.2f, %.2f", node->scale[0], node->scale[1], node->scale[2]);
					// DEBUG(1, "Transform matrix for %p (current mesh: %p):", node->mesh, mesh);
					// for (int j = 0; j < 16; j++)
					// 	printf("%6.2f%s", trans[j/4][j%4], (j + 1) % 4 == 0? "\n": ", ");

					if (node->mesh == mesh) {
						vec3 v;
						for (int j = 0; j < vert_max; j++) {
							v[0] = vts[3*j + 0];
							v[1] = vts[3*j + 1];
							v[2] = vts[3*j + 2];
							glm_mat4_mulv3(trans, v, 1.0f, &vts[3*j]);
						}
					}
				}

			skip_transform:
				switch(prim->attributes[a].type) {
				case cgltf_attribute_type_position:
					models->meshes[m].vbos[0] = vbo_new(attr->count*sizeof(float[3]), verts, false);
					break;
				case cgltf_attribute_type_normal:
					models->meshes[m].vbos[1] = vbo_new(attr->count*sizeof(float[3]), normals, false);
					break;
				case cgltf_attribute_type_texcoord:
					models->meshes[m].vbos[2] = vbo_new(attr->count*sizeof(float[2]), uvs, false);
					break;
				case cgltf_attribute_type_joints:
					models->meshes[m].vbos[3] = vbo_new(attr->count*sizeof(int8[4]), verts, false);
					break;
				case cgltf_attribute_type_weights:
					models->meshes[m].vbos[4] = vbo_new(attr->count*sizeof(float[4]), verts, false);
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
			} else {
				ERROR("[RES] GLTF file does not provide indices (non-indexed drawing not supported)");
				exit(70);
			}

			/* Assign materials to their meshes */
			for (int i = 0; i < (int)data->materials_count; i++)
				if (&data->materials[i] == data->meshes[i].primitives[p].material)
					model->meshes[i].materiali = i;
		}

		if (!models->meshes[m].vbos[2].buf)
			models->meshes[m].vbos[2] = vbo_new(1, uvs, false);
	}

	arena_free(mesh_mem);
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

			strncpy(joint->name, node->name, JOINT_MAX_NAME_LEN);
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

			if (node->scale[0] - node->scale[1] > FLT_EPSILON ||
			    node->scale[1] - node->scale[2] > FLT_EPSILON)
			    ERROR("[RES] Non-linear scale (%.2f, %2.f, %.2f)", node->scale[0], node->scale[1], node->scale[2]);
			joint->transform.scale = node->scale[0];

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
				joint->transform.scale *= model->skin->joints[joint->parent].transform.scale;
			} else if (joint->parent != -1) {
				ERROR("[GFX] Assuming joints are toplogically sorted, but joint %d has parent %d", j, joint->parent);
			}

			DEBUG(1, "[%d] Joint name: \"%.32s\"; parent: %d", j, model->skin->joints[j].name, model->skin->joints[j].parent);
			DEBUG(1, "\ttranslation: %.2f, %.2f, %.2f", joint->transform.translation.x, joint->transform.translation.y, joint->transform.translation.z);
			DEBUG(1, "\tscale: %.2f", joint->transform.scale);
			DEBUG(1, "\trotation: %.2f, %.2f, %.2f, %.2f", joint->transform.rotation.x, joint->transform.rotation.y, joint->transform.rotation.z, joint->transform.rotation.w);
		}
	}
}

static void load_animations(struct Model* model, cgltf_data* data)
{
	DEBUG(1, "\n-------------------------------------------------------------------------------");
	model->animationc = data->animations_count;
	model->animations = scalloc(data->animations_count, sizeof(struct Animation));

	cgltf_animation*         animation;
	cgltf_animation_channel* channel;
	cgltf_animation_sampler* sampler;
	for (int a = 0; a < (int)data->animations_count; a++) {
		animation = &data->animations[a];
		if (animation->name)
			strncpy(model->animations[a].name, animation->name, JOINT_MAX_NAME_LEN);
		DEBUG(1, "[%d] Animation \"%s\"", a, model->animations[a].name);

		int framec = 0;
		for (int c = 0; c < (int)animation->channels_count; c++)
			framec = MAX(framec, (int)animation->channels[c].sampler->input->count);
		model->animations[a].framec = framec;
		model->animations[a].frames = smalloc(framec*sizeof(struct KeyFrame));

		// TODO: Optimize to not always allocated for every joint
		for (int f = 0; f < framec; f++) {
			model->animations[a].frames[f].transforms = scalloc(model->skin->jointc, sizeof(struct Transform));
			model->animations[a].frames[f].jointc = model->skin->jointc;
		}

		cgltf_accessor* input;
		cgltf_accessor* output;
		float* verts;
		for (int c = 0; c < (int)animation->channels_count; c++) {
			channel = &animation->channels[c];
			sampler = channel->sampler;
			input   = sampler->input;
			output  = sampler->output;
			if (input->type != cgltf_type_scalar)
				ERROR("[RES] Input accessor type was expected to be scalar (%d), got: %d",
				      cgltf_type_scalar, input->type);
			if (output->type != cgltf_type_vec3 && output->type != cgltf_type_vec4)
				ERROR("[RES] Output accessor type was expected to be vec3 (%d) or vec4 (%d), got: %d",
				      cgltf_type_vec3, cgltf_type_vec4, output->type);
			if (input->component_type != cgltf_component_type_r_32f || output->component_type != cgltf_component_type_r_32f)
				ERROR("[RES] Input/Output accessor component type was expected to be float (%d), got: %d/%d",
				      cgltf_component_type_r_32f, input->component_type, output->component_type);

			/* Times for the keyframes */
			verts = (float*)input->buffer_view->buffer->data + input->buffer_view->offset/sizeof(float) + input->offset/sizeof(float);
			for (int f = 0, i = 0; f < (int)input->count; f++) {
				model->animations[a].frames[f].time = verts[i];
				i += (int)(input->stride/sizeof(float));
			}

			/* Find the joint this channel is for */
			int joint = -1;
			for (int i = 0; i < (int)data->skins[0].joints_count; i++) {
				if (data->skins[0].joints[i] == channel->target_node) {
					joint = i;
					break;
				}
			}
			if (joint == -1) {
				ERROR("[RES] Channel does not seem to be linked to any joint");
				continue;
			}

			/* Transforms (one of scale/transform/rotate at a time) for the keyframes */
			verts = (float*)output->buffer_view->buffer->data + output->buffer_view->offset/sizeof(float) + output->offset/sizeof(float);
			for (int f = 0, i = 0; f < (int)output->count; f++) {
				switch (channel->target_path) {
				case cgltf_animation_path_type_rotation:
					memcpy(&model->animations[a].frames[f].transforms[joint].rotation, verts, sizeof(float[4]));
					break;
				case cgltf_animation_path_type_translation:
					memcpy(&model->animations[a].frames[f].transforms[joint].translation, verts, sizeof(float[3]));
					break;
				case cgltf_animation_path_type_scale:
					model->animations[a].frames[f].transforms[joint].scale = verts[0];
					break;
				default:
					ERROR("[RES] Unsupported target path: %d", channel->target_path);
				}
				i += (int)(output->stride/sizeof(float));
			}
		}
	}

	DEBUG(1, " --- Model Animation --- ");
	struct Animation* anim = &model->animations[0];
	struct KeyFrame*  frame;
	struct Transform* trans;
	for (int f = 0; f < anim->framec; f++) {
		frame = &anim->frames[f];
		DEBUG(1, "Frame %d (%d joints):", f, frame->jointc);
		for (int j = 0; j < frame->jointc; j++) {
			trans = &frame->transforms[j];
			DEBUG(1, "\t[%d:] Rot (%.2f, %.2f, %.2f, %.2f); Trans (%.2f, %.2f, %.2f); Scale (%.2f)",
			      j, trans->rotation.x, trans->rotation.y, trans->rotation.z, trans->rotation.w,
			      trans->translation.x, trans->translation.y, trans->translation.z, trans->scale);
		}
	}
}

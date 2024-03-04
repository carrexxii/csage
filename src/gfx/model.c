#define CGLTF_IMPLEMENTATION
#include "cgltf/cgltf.h"
#include "vulkan/vulkan.h"
#include "stb/stb_image.h"

#include "maths/maths.h"
#include "util/arena.h"
#include "util/file.h"
#include "vulkan.h"
#include "buffers.h"
#include "pipeline.h"
#include "renderer.h"
#include "camera.h"
#include "model.h"

static struct PushConstants {
	int   mtli;
	float timer;
} push_consts;

static void load_materials(struct Model* mdl, cgltf_data* data);
static struct Texture load_material_image(cgltf_image* img);
static void load_meshes(struct Model* mdl, cgltf_data* data);
static void load_skin(struct Model* mdl, cgltf_data* data);
static void load_animations(struct Model* mdl, cgltf_data* data);

/* -------------------------------------------------------------------- */

static VkVertexInputBindingDescription vert_binds[] = {
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
	  .stride    = sizeof(uint8[4]),
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, },
	{ .binding   = 4,
	  .stride    = sizeof(float[4]),
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, },
};
static VkVertexInputAttributeDescription vert_attrs[] = {
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
	  .format   = VK_FORMAT_R8G8B8A8_UINT,
	  .offset   = 0, },
	/* bone weights */
	{ .binding  = 4,
	  .location = 4,
	  .format   = VK_FORMAT_R32G32B32A32_SFLOAT,
	  .offset   = 0, },
};

/* -------------------------------------------------------------------- */

void (*update_mdl_tforms)(SBO);
Mat4x4* mdl_tforms;
struct Material default_mtl;

static struct Pipeline pipeln;
static struct Pipeline pipeln_static;
static struct Model mdls[MAX_MODELS];
static intptr mdlc = 0;
static UBO ubo_cam;
static UBO ubo_mtls;
static UBO ubo_lights;
static UBO ubo_joints;
static SBO sbo_buf;

void models_init(VkRenderPass renderpass)
{
	default_mtl = (struct Material){
		.albedo    = { 0.3f, 0.3f, 0.3f, 1.0f },
		.metallic  = 1.0f,
		.roughness = 1.0f,
		.tex       = texture_new_from_image(TEXTURE_PATH "default.png"),
	};

	sbo_buf    = sbo_new(MAX_MODELS*sizeof(Mat4x4));
	ubo_cam    = ubo_new(sizeof(Mat4x4[2]));
	ubo_mtls   = ubo_new(MAX_MATERIALS*sizeof(struct Material));
	ubo_lights = ubo_new(sizeof(struct GlobalLighting));
	ubo_joints = ubo_new(2*MODEL_MAX_JOINTS*sizeof(struct Transform));

	pipeln = (struct Pipeline){
		.vshader     = create_shader(SHADER_DIR "model.vert"),
		.fshader     = create_shader(SHADER_DIR "model.frag"),
		.topology    = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.vert_bindc  = 5,
		.vert_binds  = vert_binds,
		.vert_attrc  = 5,
		.vert_attrs  = vert_attrs,
		.push_stages = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		.push_sz     = sizeof(struct PushConstants),
		.dset_cap    = 4,
		.uboc        = 4,
		.sboc        = 2,
		.imgc        = 1,
	};
	pipeln_static = (struct Pipeline){
		.vshader     = create_shader(SHADER_DIR "model_static.vert"),
		.fshader     = create_shader(SHADER_DIR "model.frag"),
		.topology    = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.vert_bindc  = 3,
		.vert_binds  = vert_binds,
		.vert_attrc  = 3,
		.vert_attrs  = vert_attrs,
		.push_stages = VK_SHADER_STAGE_FRAGMENT_BIT,
		.push_sz     = sizeof(struct PushConstants),
		.dset_cap    = 4,
		.uboc        = 3,
		.sboc        = 1,
		.imgc        = 2,
	};
	// pipeln_alloc_dsets(&pipeln);
	pipeln_alloc_dsets(&pipeln_static);
	VkImageView img_view;
	for (int i = 0; i < mdlc; i++) {
		for (int j = 0; j < mdls[i].mtlc; j++) {
			img_view = mdls[i].mtls[j].tex.image_view;
			if (!img_view)
				img_view = default_mtl.tex.image_view;

			UBO ubos[] = { ubo_cam, ubo_mtls, ubo_lights, ubo_joints };
			struct Material* mtl = &mdls[i].mtls[j];
			if (mdls[i].skin)
				mtl->dset = pipeln_create_dset(&pipeln,
				                               4, ubos,
				                               2, (SBO[]){ sbo_buf, mdls[i].skin->sbo },
				                               1, &img_view);
			else
				mtl->dset = pipeln_create_dset(&pipeln_static,
				                               3, ubos,
				                               1, (SBO[]){ sbo_buf },
				                               1, &img_view);
		}
	}
	// pipeln_init(&pipeln, renderpass);
	pipeln_init(&pipeln_static, renderpass);
}

// TODO: this doesnt work properly for removing elements
struct Model* model_new(char* path)
{
	struct Model* mdl = &mdls[mdlc++];
	*mdl = (struct Model){ 0 };

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

	load_materials(mdl, data);
	load_meshes(mdl, data);
	load_skin(mdl, data);
	load_animations(mdl, data);

#if DEBUG_LEVEL > 0
	int total_indc = 0;
	for (int i = 0; i < mdl->meshc; i++)
		total_indc += mdl->meshes[i].indc;

	int total_framec = 0;
	for (int i = 0; i < mdl->animc; i++)
		total_framec += mdl->anims[i].frmc;
#endif

	cgltf_free(data);
	DEBUG(3, "[GFX] Loaded file \"%s\" with %d meshes (%d vertices/%d triangles), %d materials and %d animations (%d joints) with %d total frames",
	      path, mdl->meshc, total_indc, total_indc/3, mdl->mtlc, mdl->animc, mdl->skin? mdl->skin->jointc: 0, total_framec);
	// exit(0);
	return mdl;
}

void models_update()
{
	// struct Model*     mdl;
	// struct Animation* anim;
	// for (int m = 0; m < mdlc; m++) {
	// 	mdl  = &mdls[m];
	// 	anim = &mdl->anims[mdl->curr_anim];
	// 	mdl->timer += DT;
	// 	if (mdl->timer >= anim->frms[anim->curr_frm].time)
	// 		anim->curr_frm++;

	// 	if (anim->curr_frm >= anim->frmc) {
	// 		anim->curr_frm = 0;
	// 		mdl->timer = 0;
	// 	}

	// 	buffer_update(mdl->skin->sbo, mdl->skin->sbo.sz, mdl->skin, 0);
	// }
}

void models_record_commands(VkCommandBuffer cmd_buf, struct Camera* cam)
{
	struct Model*     mdl;
	struct Mesh*      mesh;
	struct Animation* anim;
	struct KeyFrame*  curr_frm;
	struct KeyFrame*  next_frm;

	buffer_update(ubo_cam, sizeof(Mat4x4[2]), cam->mats, 0);

	update_mdl_tforms(sbo_buf);

	/*** Animated models ***/
	// vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.pipeln);
	// vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.layout, 0, 1, pipeln.dsets, 0, NULL);
	// for (int i = 0; i < mdlc; i++) {
	// 	mdl      = &mdls[i];
	// 	anim     = &mdl->anims[mdl->curr_anim];
	// 	curr_frm = &anim->frms[anim->curr_frm];
	// 	next_frm = &anim->frms[(anim->curr_frm + 1)%anim->frmc];
	// 	if (!mdl->skin->jointc)
	// 		continue;
	// 	buffer_update(ubo_mtls, mdl->mtlc*sizeof(struct Material), mdl->mtls, 0);
	// 	buffer_update(ubo_lights, sizeof(struct GlobalLighting), &global_light, 0);
	// 	buffer_update(ubo_joints, mdl->skin->jointc*sizeof(struct Transform), curr_frm->tforms, 0);
	// 	// buffer_update(ubo_joints, mdl->skin->jointc*sizeof(struct Transform), next_frm->tforms, MODEL_MAX_JOINTS*sizeof(struct Transform));
	// 	for (int m = 0; m < mdls[i].meshc; m++) {
	// 		mesh = &mdl->meshes[m];
	// 		push_consts.mtli  = mesh->mtli;
	// 		push_consts.timer = mdl->timer;
	// 		vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.layout, 0, 1, &mdl->mtls[mesh->mtli].dset, 0, NULL);
	// 		// vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.layout, 0, 1, pipeln.dsets, 0, NULL);
	// 		// DEBUG(1, "[%d Animated] Drawing %d vertices", i, mdls[i].meshes[m].vertc);

	// 		// TODO: these vbos need to be named
	// 		vkCmdBindVertexBuffers(cmd_buf, 0, 5, (VkBuffer[]){ mesh->vbos[0].buf,
	// 		                       mesh->vbos[1].buf, mesh->vbos[2].buf, mesh->vbos[3].buf, mesh->vbos[4].buf },
	// 		                       (VkDeviceSize[]){ 0, 0, 0, 0, 0 });
	// 		vkCmdBindIndexBuffer(cmd_buf, mesh->ibo.buf, 0, VK_INDEX_TYPE_UINT16);
	// 		vkCmdPushConstants(cmd_buf, pipeln.layout, pipeln.push_stages, 0, pipeln.push_sz, &push_consts);
	// 		vkCmdDrawIndexed(cmd_buf, mesh->indc, 1, 0, 0, 0);
	// 	}
	// }

	/*** Static Models ***/
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln_static.pipeln);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln_static.layout, 0, 1, pipeln_static.dsets, 0, NULL);
	for (int i = 0; i < mdlc; i++) {
		mdl = &mdls[i];
		if (mdl->skin)
			continue;
		buffer_update(ubo_mtls, mdl->mtlc*sizeof(struct Material), mdl->mtls, 0);
		buffer_update(ubo_lights, sizeof(struct GlobalLighting), &global_light, 0);

		for (int m = 0; m < mdls[i].meshc; m++) {
			mesh = &mdl->meshes[m];
			push_consts.mtli = mesh->mtli;
			vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln_static.layout, 0,
			                        1, &mdl->mtls[mesh->mtli].dset, 0, NULL);

			// DEBUG(1, "[%d Static] Drawing %d vertices", i, mdls[i].meshes[m].indc);
			vkCmdBindVertexBuffers(cmd_buf, 0, 3, (VkBuffer[]){ mesh->vbos[0].buf,
			                       mesh->vbos[1].buf, mesh->vbos[2].buf }, (VkDeviceSize[]){ 0, 0, 0 });
			vkCmdBindIndexBuffer(cmd_buf, mesh->ibo.buf, 0, VK_INDEX_TYPE_UINT16);
			vkCmdPushConstants(cmd_buf, pipeln_static.layout, pipeln_static.push_stages, 0, pipeln_static.push_sz, &push_consts);
			vkCmdDrawIndexed(cmd_buf, mesh->indc, 1, 0, 0, 0);
		}
	}
}

void model_free(ID mdl_id)
{
	struct Model* mdl = &mdls[mdl_id];
	for (int i = 0; i < mdl->meshc; i++) {
		/* Static Meshes do not use the last two vbos */
		for (int j = 0; j < MODEL_MESH_VBO_COUNT - (mdl->animc > 0? 0: 2); j++)
			vbo_free(&mdl->meshes[i].vbos[j]);
		ibo_free(&mdl->meshes[i].ibo);
	}

	texture_free(&default_mtl.tex);
	for (int i = 0; i < mdl->mtlc; i++)
		if (mdl->mtls[i].tex.image)
			texture_free(&mdl->mtls[i].tex);
	for (int i = 0; i < mdlc; i++)
		if (mdls[i].skin)
			sbo_free(&mdls[i].skin->sbo);
	mdl->meshc = 0;
}

void models_free()
{
	for (int i = 0; i < mdlc; i++)
		model_free(i);

	sbo_free(&sbo_buf);
	ubo_free(&ubo_cam);
	ubo_free(&ubo_mtls);
	ubo_free(&ubo_lights);
	ubo_free(&ubo_joints);

	pipeln_free(&pipeln);
	pipeln_free(&pipeln_static);
}

/* -------------------------------------------------------------------- */

static void load_materials(struct Model* model, cgltf_data* data)
{
	model->mtlc = data->materials_count? data->materials_count: 1;
	model->mtls = scalloc(model->mtlc, sizeof(struct Material));
	if (model->mtlc > MAX_MATERIALS)
		ERROR("[RES] Model has %d materials. MAX_MATERIALS is set to %d", model->mtlc, MAX_MATERIALS);

	cgltf_material* material;
	for (int m = 0; m < (int)data->materials_count; m++) {
		material = &data->materials[m];
		memcpy(model->mtls[m].albedo, material->pbr_metallic_roughness.base_color_factor, sizeof(float[4]));

		model->mtls[m].metallic  = material->pbr_metallic_roughness.metallic_factor;
		model->mtls[m].roughness = material->pbr_metallic_roughness.roughness_factor;
		if (!material->has_pbr_metallic_roughness) {
			ERROR("[RES] Only metallic-roughness materials are supported");
			continue;
		}

		if (material->pbr_metallic_roughness.base_color_texture.texture)
			model->mtls[m].tex = load_material_image(material->pbr_metallic_roughness.base_color_texture.texture->image);
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
		DEBUG(5, "\tAlbedo    -> %.2f, %.2f, %.2f, %.2f", UNPACK4(model->mtls[m].albedo));
		DEBUG(5, "\tMetallic  -> %.2f", model->mtls[m].metallic);
		DEBUG(5, "\tRoughness -> %.2f", model->mtls[m].roughness);
	}

	/* Default material if none exist */
	if (!data->materials_count) {
		mdls->mtlc    = 1;
		mdls->mtls[0] = default_mtl;
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
			struct Texture tex = texture_new(pxs, w, h);
			free(pxs);
			return tex;
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
	struct Arena* mesh_mem = arena_new(2*indc_max*sizeof(uint16) + vert_max*(sizeof(float[12]) + sizeof(uint8[4])), 0);
	uint16* inds         = arena_alloc(mesh_mem, indc_max*sizeof(uint16));
	float* verts         = arena_alloc(mesh_mem, vert_max*sizeof(float[3]));
	float* normals       = arena_alloc(mesh_mem, vert_max*sizeof(float[3]));
	float* uvs           = arena_alloc(mesh_mem, vert_max*sizeof(float[2]));
	uint8* joint_ids     = arena_alloc(mesh_mem, vert_max*sizeof(uint8[4]));
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
					static bool tangent_err = false;
					static bool colour_err  = false;
					switch(prim->attributes[a].type) {
					case cgltf_attribute_type_position:
						if (attr->component_type != cgltf_component_type_r_32f || attr->type != cgltf_type_vec3)
							ERROR("[RES] Position attribute has invalid type (expect vec3 (%d vs %d) float (%d vs %d)):",
							      cgltf_type_vec3, attr->type, cgltf_component_type_r_32f, attr->component_type);
						verts[3*v + 0] = vert[i + 0];
						verts[3*v + 1] = vert[i + 1];
						verts[3*v + 2] = vert[i + 2];
						i += (int)(attr->stride / sizeof(float));
						break;
					case cgltf_attribute_type_normal:
						if (attr->component_type != cgltf_component_type_r_32f || attr->type != cgltf_type_vec3)
							ERROR("[RES] Normal attribute has invalid type (expect vec3 (%d vs %d) float (%d vs %d)):",
							      cgltf_type_vec3, attr->type, cgltf_component_type_r_32f, attr->component_type);
						normals[3*v + 0] = vert[i + 0];
						normals[3*v + 1] = vert[i + 1];
						normals[3*v + 2] = vert[i + 2];
						i += (int)(attr->stride / sizeof(float));
						break;
					case cgltf_attribute_type_texcoord:
						if (attr->component_type != cgltf_component_type_r_32f || attr->type != cgltf_type_vec2)
							ERROR("[RES] Texcoord attribute has invalid type (expect vec2 (%d vs %d) float (%d vs %d)):",
							      cgltf_type_vec2, attr->type, cgltf_component_type_r_32f, attr->component_type);
						uvs[2*v + 0] = vert[i + 0];
						uvs[2*v + 1] = vert[i + 1];
						i += (int)(attr->stride / sizeof(float));
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
							i += (int)(attr->stride / sizeof(uint8));
						} else if (attr->component_type == cgltf_component_type_r_16u) {
							joint_ids[4*v + 0] = uvert16[i + 0];
							joint_ids[4*v + 1] = uvert16[i + 1];
							joint_ids[4*v + 2] = uvert16[i + 2];
							joint_ids[4*v + 3] = uvert16[i + 3];
							i += (int)(attr->stride / sizeof(uint16));
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
						i += (int)(attr->stride / sizeof(float));
						break;
					case cgltf_attribute_type_tangent:
						if (!tangent_err) {
							ERROR("[GFX] Tangent loading from GLTF files not implemented");
							tangent_err = true;
						}
						break;
					case cgltf_attribute_type_color:
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
				// cgltf_node* node;
				// Mat4x4 trans;
				// float* vts; /* vts = vertex type that the transform will be applied to */
				// if (prim->attributes[a].type == cgltf_attribute_type_position)
				// 	vts = verts;
				// else if (prim->attributes[a].type == cgltf_attribute_type_normal)
				// 	vts = normals;
				// else
				// 	goto skip_transform;

				// for (int n = 0; n < (int)data->nodes_count; n++) {
				// 	node = &data->nodes[n];
				// 	cgltf_node_transform_world(node, trans.arr);
				// 	// DEBUG(1, "rot: %d, trans: %d, scale: %d", node->has_rotation, node->has_translation, node->has_scale);
				// 	// DEBUG(1, "rotation: %.2f, %.2f, %.2f, %.2f", node->rotation[0], node->rotation[1], node->rotation[2], node->rotation[3]);
				// 	// DEBUG(1, "translation: %.2f, %.2f, %.2f", node->translation[0], node->translation[1], node->translation[2]);
				// 	// DEBUG(1, "scale: %.2f, %.2f, %.2f", node->scale[0], node->scale[1], node->scale[2]);
				// 	// DEBUG(1, "Transform matrix for %p (current mesh: %p):", node->mesh, mesh);
				// 	// for (int j = 0; j < 16; j++)
				// 	// 	printf("%6.2f%s", trans[j/4][j%4], (j + 1) % 4 == 0? "\n": ", ");

				// 	if (node->mesh == mesh) {
				// 		// Vec3 v;
				// 		// for (int j = 0; j < vert_max; j++) {
				// 			// v.x = vts[3*j + 0];
				// 			// v.y = vts[3*j + 1];
				// 			// v.z = vts[3*j + 2];
				// 			// glm_mat4_mulv3(trans, v, 1.0f, &vts[3*j]);
				// 		// }
				// 	}
				// }

			// skip_transform:
				switch(prim->attributes[a].type) {
				case cgltf_attribute_type_position:
					mdls->meshes[m].vbos[0] = vbo_new(attr->count*sizeof(float[3]), verts, false);
					break;
				case cgltf_attribute_type_normal:
					mdls->meshes[m].vbos[1] = vbo_new(attr->count*sizeof(float[3]), normals, false);
					break;
				case cgltf_attribute_type_texcoord:
					mdls->meshes[m].vbos[2] = vbo_new(attr->count*sizeof(float[2]), uvs, false);
					break;
				case cgltf_attribute_type_joints:
					mdls->meshes[m].vbos[3] = vbo_new(attr->count*sizeof(uint8[4]), joint_ids, false);
					break;
				case cgltf_attribute_type_weights:
					mdls->meshes[m].vbos[4] = vbo_new(attr->count*sizeof(float[4]), joint_weights, false);
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
			for (int i = 0; i < (int)data->materials_count; i++) {
				if (&data->materials[i] == data->meshes[m].primitives[p].material) // !
					model->meshes[i].mtli = i;
			}
		}

		/* Make sure there's a dummy vbo for uvs if there aren't any */
		float duvs[] = { 0.0f, 0.0f, };
		if (!mdls->meshes[m].vbos[2].buf)
			mdls->meshes[m].vbos[2] = vbo_new(1, duvs, false);
	}

	arena_free(mesh_mem);
}

static void load_skin(struct Model* model, cgltf_data* data)
{
	if (data->skins_count > 1)
		ERROR("[RES] Warning: only one of the %lu skins will be loaded", data->skins_count);
	else
		DEBUG(5, "\tModel has %lu skins", data->skins_count);

	cgltf_skin* skin;
	cgltf_node* node;
	struct Joint* joint;
	if (data->skins_count) {
		skin = data->skins;
		model->skin = smalloc(skin->joints_count*sizeof(struct Joint) + sizeof(struct Skin));
		model->skin->jointc = (int)skin->joints_count;

		float* inv_binds = smalloc(skin->inverse_bind_matrices->count*skin->inverse_bind_matrices->stride);
		cgltf_accessor_unpack_floats(skin->inverse_bind_matrices, inv_binds, skin->inverse_bind_matrices->count*skin->inverse_bind_matrices->stride);

		for (int j = 0; j < (int)skin->joints_count; j++) {
			joint = &model->skin->joints[j];
			node  = skin->joints[j];
			if (node->mesh)
				ERROR("[RES] Ignoring node mesh data");

			memcpy(joint->inv_bind, &inv_binds[16*j], sizeof(float[16]));

			joint->parent = -1;
			for (int j2 = 0; j2 < (int)skin->joints_count; j2++) {
				if (skin->joints[j2] == node->parent && j2 != j) {
					joint->parent = j2;
					break;
				}
			}

			cgltf_node_transform_local(node, joint->tform);
		}

		model->skin->sbo = sbo_new(model->skin->jointc*sizeof(struct Joint) + sizeof(struct Skin));
		// buffer_update(model->skin->sbo, model->skin->sbo.sz, inv_binds, 0);
		free(inv_binds);
	}
}

static void load_animations(struct Model* mdl, cgltf_data* data)
{
	DEBUG(5, "\tModel has %lu animations", data->animations_count);
	mdl->animc = data->animations_count;
	mdl->anims = scalloc(data->animations_count, sizeof(struct Animation));

	cgltf_animation*         anim;
	cgltf_animation_channel* channel;
	cgltf_animation_sampler* sampler;
	for (int a = 0; a < (int)data->animations_count; a++) {
		anim = &data->animations[a];

		int framec = 0;
		for (int c = 0; c < (int)anim->channels_count; c++)
			framec = MAX(framec, (int)anim->channels[c].sampler->input->count);
		mdl->anims[a].frmc     = framec;
		mdl->anims[a].frms     = scalloc(framec, sizeof(struct KeyFrame));
		mdl->anims[a].curr_frm = 0;

		// TODO: Find a method that doesn't allocate for unchanged joints?
		for (int f = 0; f < framec; f++)
			mdl->anims[a].frms[f].tforms = scalloc(MODEL_MAX_JOINTS, sizeof(struct Transform));

		cgltf_accessor* input;
		cgltf_accessor* output;
		float* verts;
		for (int c = 0; c < (int)anim->channels_count; c++) {
			channel = &anim->channels[c];
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
				mdl->anims[a].frms[f].time = verts[i];
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

			verts = (float*)output->buffer_view->buffer->data + output->buffer_view->offset/sizeof(float) + output->offset/sizeof(float);
			for (int f = 0, i = 0; f < (int)output->count; f++) {
				switch (channel->target_path) {
				case cgltf_animation_path_type_rotation:
					memcpy(&mdl->anims[a].frms[f].tforms[joint].rot.arr, &verts[i], sizeof(float[4]));
					break;
				case cgltf_animation_path_type_translation:
					memcpy(&mdl->anims[a].frms[f].tforms[joint].trans.arr, &verts[i], sizeof(float[3]));
					break;
				case cgltf_animation_path_type_scale:
					mdl->anims[a].frms[f].tforms[joint].scale = verts[i];
					break;
				default:
					ERROR("[RES] Unsupported target path: %d", channel->target_path);
				}
				i += (int)(output->stride/sizeof(float));
			}
		}
	}

	// DEBUG(1, " --- Model Animation --- ");
	// struct Animation* danim;
	// struct KeyFrame*  frame;
	// struct Transform* trans;
	// for (int a = 0; a < mdl->animc; a++) {
	// 	danim = &mdl->anims[a];
	// 	for (int f = 0; f < danim->frmc; f++) {
	// 		frame = &danim->frms[f];
	// 		fprintf(stderr, "Frame %d: \n", f);
	// 		for (int j = 0; j < mdl->skin->jointc; j++) {
	// 			trans = &frame->tforms[j];
	// 			DEBUG(1, "\t[%d::%.2f]\tRot (%5.2f, %5.2f, %5.2f, %5.2f);    \tTrans (%5.2f, %5.2f, %5.2f); Scale (%5.2f)",
	// 			      j, frame->time, trans->rot.x, trans->rot.y, trans->rot.z, trans->rot.w,
	// 			      trans->trans.x, trans->trans.y, trans->trans.z, trans->scale);
	// 		}
	// 	}
	// }
}

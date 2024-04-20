#define CGLTF_IMPLEMENTATION
#include "cgltf/cgltf.h"
#include <vulkan/vulkan.h>
#include "stb/stb_image.h"

#include "resmgr.h"
#include "maths/maths.h"
#include "vulkan.h"
#include "buffers.h"
#include "pipeline.h"
#include "renderer.h"
#include "camera.h"
#include "model.h"

static PipelineCreateInfo anim_pipeln_ci;
static PipelineCreateInfo static_pipeln_ci;
static struct {
	int   mtli;
	float timer;
} push_consts;

static isize load_materials(cgltf_data* data);
static Image load_material_image(cgltf_image* img);
static void  load_meshes(Model* mdl, cgltf_data* data);
static void  load_skin(Model* mdl, cgltf_data* data);
static void  load_animations(Model* mdl, cgltf_data* data);

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

	/* Only for animated models */
	{ .binding   = 3,
	  .stride    = sizeof(uint8[4]),
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, },
	{ .binding   = 4,
	  .stride    = sizeof(uint8[4]),
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

	/* Only for animated models */
	/* bone IDs */
	{ .binding  = 3,
	  .location = 3,
	  .format   = VK_FORMAT_R8G8B8A8_UINT,
	  .offset   = 0, },
	/* bone weights */
	{ .binding  = 4,
	  .location = 4,
	  .format   = VK_FORMAT_R8G8B8A8_UINT,
	  .offset   = 0, },
};

/* -------------------------------------------------------------------- */

static Pipeline* anim_pipeln;
static Pipeline* static_pipeln;
static SBO mtl_sbo;
static SBO anim_mdl_sbo;
static SBO static_mdl_sbo;
static VArray models;
static VArray materials;
// static VArray   tforms;
static Material default_mtl;

void models_init()
{
	anim_pipeln_ci = (PipelineCreateInfo){
		.vshader     = STRING(SHADER_PATH "/model.vert"),
		.fshader     = STRING(SHADER_PATH "/model.frag"),
		.ubo_stages  = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		.uboc        = 2,
		.sbo_stages  = VK_SHADER_STAGE_VERTEX_BIT,
		.sboc        = 4,
		.imgc        = 2,
		.push_sz     = sizeof(push_consts),
		.push_stages = VK_SHADER_STAGE_VERTEX_BIT,
	};
	static_pipeln_ci = (PipelineCreateInfo){
		.vshader     = STRING(SHADER_PATH "/model_static.vert"),
		.fshader     = STRING(SHADER_PATH "/model.frag"),
		.ubo_stages  = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		.uboc        = 2,
		.sbo_stages  = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		.sboc        = 2,
		.imgc        = 2,
		.push_sz     = sizeof(push_consts),
		.push_stages = VK_SHADER_STAGE_VERTEX_BIT,
	};

	anim_pipeln   = pipeln_new(&anim_pipeln_ci  , "Animated Models");
	static_pipeln = pipeln_new(&static_pipeln_ci, "Static Models");

	materials = varray_new(DEFAULT_MATERIAL_COUNT, sizeof(Material));
	models    = varray_new(DEFAULT_MODEL_COUNT, sizeof(Model));
	// tforms = varray_new(DEFAULT_MODEL_COUNT, sizeof(Mat4x4));

	default_mtl = (Material){
		.albedo    = { 0.3f, 0.3f, 0.3f, 1.0f },
		.metallic  = 1.0f,
		.roughness = 1.0f,
		.img       = *load_image(STRING(TEXTURE_PATH "/default.png")), // FIXME
	};
	varray_push(&materials, &default_mtl);

	mtl_sbo = sbo_new(DEFAULT_MATERIAL_COUNT * sizeof(Material));
}

void model_update_pipelines()
{
	isize imgc = 0;
	Image* imgs[materials.len];
	Material* mtl;
	for (int i = 0; i < materials.len; i++) {
		mtl = varray_get(&materials, i);
		if (mtl->img.img)
			imgs[imgc++] = &mtl->img;
	}

	static_pipeln_ci.ubos[0] = global_camera_ubo;
	static_pipeln_ci.ubos[1] = global_light_ubo;
	static_pipeln_ci.sbos[0] = static_mdl_sbo;
	static_pipeln_ci.sbos[1] = mtl_sbo;

	static_pipeln_ci.imgc = imgc;
	memcpy(static_pipeln_ci.imgs, imgs, imgc*sizeof(Image*));

	static_pipeln = pipeln_update(static_pipeln, &static_pipeln_ci);
}

isize model_new(const char* name)
{
	Model mdl = { 0 };

	char path[PATH_BUFFER_SIZE];
	snprintf(path, PATH_BUFFER_SIZE, MODEL_PATH "/%s", name);

	cgltf_options options = {
		.type = cgltf_file_type_glb,
	};
	cgltf_data* data;
	int err;
	if ((err = cgltf_parse_file(&options, path, &data))) {
		ERROR("[GFX] CGLTF failed to parse file \"%s\": %d", path, err);
	} else {
		INFO(TERM_DARK_GREEN "[GFX] Model data for \"%s\" (%zuB / %.2fkB / %.2fMB) loaded with:",
		     path, data->json_size, data->json_size/1024.0, data->json_size/1024.0/1024.0);
		INFO("\tMeshes    -> %lu", data->meshes_count);
		INFO("\tMaterials -> %lu", data->materials_count);
		INFO("\tTextures  -> %lu", data->textures_count);
		INFO("\tImages    -> %lu", data->images_count);
		INFO("\tBuffers   -> %lu", data->buffers_count);
		INFO("\tSkins     -> %lu", data->skins_count);
	}

	if ((err = cgltf_validate(data)))
		ERROR("[RES] GLTF file \"%s\" fails `cgltf_validate`: %d", path, err);

	if ((err = cgltf_load_buffers(&options, data, path)))
		ERROR("[GFX] Error loading buffer data: %d", err);
	else
		INFO(TERM_DARK_GREEN "Loaded buffer data for \"%s\" (%zuB / %.2fkB / %.2fMB)",
		     path, data->bin_size, data->bin_size/1024.0, data->bin_size/1024.0/1024.0);

	isize mtli  = load_materials(data);
	load_meshes(&mdl, data);
	// load_skin(&mdl, data);
	// isize animc = load_animations(&mdl, data);

// #if DEBUG
// 	int total_indc = 0;
// 	int total_mtlc = 0;
// 	for (int i = 0; i < mdl.meshc; i++) {
// 		total_indc += mdl.meshes[i].indc;
// 		total_mtlc += !!mdl.meshes[i].mtli;
// 	}

// 	int total_framec = 0;
// 	for (int i = 0; i < mdl.animc; i++)
// 		total_framec += mdl.anims[i].frmc;
// #endif

	cgltf_free(data);
	// INFO(TERM_DARK_GREEN "[GFX] Loaded file \"%s\" with %d meshes (%d vertices/%d triangles), %d materials and %d animations (%d joints) with %d total frames",
	     // name, mdl.meshc, total_indc, total_indc/3, total_mtlc, mdl.animc, mdl.skin? mdl.skin->jointc: 0, total_framec);
	// exit(0);
	isize mdli = varray_push(&models, &mdl);
	return mdli;
}

void models_update()
{
	// Model*     mdl;
	// Animation* anim;
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

void models_record_commands(VkCommandBuffer cmd_buf)
{
	Pipeline* pl = static_pipeln;
	Model* mdl;
	Mesh*  mesh;

	/*** Static Models ***/
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pl->pipeln);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pl->layout, 0, 1, &pl->dset.set, 0, NULL);
	for (int i = 0; i < models.len; i++) {
		mdl = varray_get(&models, i);
		if (mdl->skin)
			continue;

		for (int m = 0; m < mdl->meshc; m++) {
			mesh = &mdl->meshes[m];
			push_consts.mtli = mesh->mtli;

			vkCmdBindVertexBuffers(cmd_buf, 0, 3, (VkBuffer[]){ mesh->vbos[0].buf,
			                       mesh->vbos[1].buf, mesh->vbos[2].buf }, (VkDeviceSize[]){ 0, 0, 0 });
			vkCmdBindIndexBuffer(cmd_buf, mesh->ibo.buf, 0, VK_INDEX_TYPE_UINT16);
			vkCmdPushConstants(cmd_buf, static_pipeln->layout, static_pipeln_ci.push_stages, 0, static_pipeln_ci.push_sz, &push_consts);
			vkCmdDrawIndexed(cmd_buf, mesh->indc, 1, 0, 0, 0);
		}
	}
}

// void model_free(ID mdl_id)
// {
// 	Model* mdl = &mdls[mdl_id];
// 	for (int i = 0; i < mdl->meshc; i++) {
// 		/* Static Meshes do not use the last two vbos */
// 		for (int j = 0; j < MODEL_MESH_VBO_COUNT - (mdl->animc > 0? 0: 2); j++)
// 			vbo_free(&mdl->meshes[i].vbos[j]);
// 		ibo_free(&mdl->meshes[i].ibo);
// 	}

// 	for (int i = 0; i < mdlc; i++)
// 		if (mdls[i].skin)
// 			sbo_free(&mdls[i].skin->sbo);
// 	mdl->meshc = 0;
// }

// void models_free()
// {
// 	for (int i = 0; i < mdlc; i++)
// 		model_free(i);

// 	sbo_free(&sbo_buf);
// 	ubo_free(&ubo_cam);
// 	ubo_free(&ubo_mtls);
// 	ubo_free(&ubo_lights);
// 	ubo_free(&ubo_joints);

// 	pipeln_free(&pipeln);
// 	pipeln_free(&static_pipeln);
// }

/* -------------------------------------------------------------------- */

static isize load_materials(cgltf_data* data)
{
	Material mtl;
	cgltf_material* mtl_data;
	isize fst = materials.len;
	for (int i = 0; i < (int)data->materials_count; i++) {
		mtl_data = &data->materials[i];

		mtl.albedo    = VEC4_A(mtl_data->pbr_metallic_roughness.base_color_factor);
		mtl.metallic  = mtl_data->pbr_metallic_roughness.metallic_factor;
		mtl.roughness = mtl_data->pbr_metallic_roughness.roughness_factor;

		if (mtl_data->pbr_metallic_roughness.base_color_texture.texture)
			mtl.img = load_material_image(mtl_data->pbr_metallic_roughness.base_color_texture.texture->image);
		if (mtl_data->pbr_metallic_roughness.metallic_roughness_texture.texture)
			WARN("[GFX] Ignoring metallic roughness texture");
		if (mtl_data->normal_texture.texture)
			WARN("[GFX] Ignoring normal texture");
		if (mtl_data->occlusion_texture.texture)
			WARN("[GFX] Ignoring occlusion texture");
		if (mtl_data->emissive_texture.texture)
			WARN("[RES] Ignoring emissive texture");

		varray_push(&materials, &mtl);
		INFO(TERM_DARK_GREEN "[RES] Loaded material %ld with: %s%s%s%s%s", materials.len - 1,
		     mtl_data->pbr_metallic_roughness.base_color_texture.texture        ? "base colour texture, "       : "",
		     mtl_data->pbr_metallic_roughness.metallic_roughness_texture.texture? "metallic roughness texture, ": "",
		     mtl_data->normal_texture.texture   ? "normal texture, "   : "",
		     mtl_data->occlusion_texture.texture? "occlusion texture, ": "",
		     mtl_data->emissive_texture.texture ? "emissive texture"   : "");
		INFO(TERM_DARK_GREEN "\tAlbedo    -> %.2f, %.2f, %.2f, %.2f", UNPACK4(mtl.albedo.arr));
		INFO(TERM_DARK_GREEN "\tMetallic  -> %.2f", mtl.metallic);
		INFO(TERM_DARK_GREEN "\tRoughness -> %.2f", mtl.roughness);
	}

	return fst;
}

// TODO: Add in-memory loading to resmgr
static Image load_material_image(cgltf_image* img_data)
{
	if (img_data->uri) {
		ERROR("[RES] Image loading from URIs not yet implemented");
		return (Image){ 0 };
	} else if (img_data->buffer_view->buffer->data) {
		if (!strncmp(img_data->mime_type, "image/png", sizeof("image/png"))) {
			int w, h, ch;
			byte* pxs = stbi_load_from_memory((byte*)img_data->buffer_view->buffer->data + img_data->buffer_view->offset,
			                                  img_data->buffer_view->size, &w, &h, &ch, 4);
			Image img = image_of_memory(w, h, pxs);
			free(pxs);
			return img;
		} else {
			ERROR("[RES] MIME type not supported for image: %s", img_data->mime_type);
		}
	}

	ERROR("[RES] Image does not have either URI or buffer data");
	return (Image){ 0 };
}

static void load_meshes(Model* mdl, cgltf_data* data)
{
	mdl->meshc  = data->meshes_count;
	mdl->meshes = scalloc(data->meshes_count, sizeof(Mesh));

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
	Arena mesh_mem = arena_new(2*indc_max*sizeof(uint16) + vert_max*(sizeof(float[8]) + sizeof(uint8[8])), 0);
	uint16* inds          = arena_alloc(&mesh_mem, indc_max*sizeof(uint16));
	float*  verts         = arena_alloc(&mesh_mem, vert_max*sizeof(Vec3));
	float*  normals       = arena_alloc(&mesh_mem, vert_max*sizeof(Vec3));
	float*  uvs           = arena_alloc(&mesh_mem, vert_max*sizeof(Vec2));
	uint8*  joint_ids     = arena_alloc(&mesh_mem, vert_max*sizeof(uint8[4]));
	float*  joint_weights = arena_alloc(&mesh_mem, vert_max*sizeof(uint8[4]));

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

				switch(prim->attributes[a].type) {
				case cgltf_attribute_type_position: mdl->meshes[m].vbos[0] = vbo_new(attr->count*sizeof(Vec3)    , verts        , false); break;
				case cgltf_attribute_type_normal  : mdl->meshes[m].vbos[1] = vbo_new(attr->count*sizeof(Vec3)    , normals      , false); break;
				case cgltf_attribute_type_texcoord: mdl->meshes[m].vbos[2] = vbo_new(attr->count*sizeof(Vec2)    , uvs          , false); break;
				case cgltf_attribute_type_joints  : mdl->meshes[m].vbos[3] = vbo_new(attr->count*sizeof(uint8[4]), joint_ids    , false); break;
				case cgltf_attribute_type_weights : mdl->meshes[m].vbos[4] = vbo_new(attr->count*sizeof(uint8[4]), joint_weights, false); break;
				default:
					continue;
				}
			}

			if (prim->indices) {
				attr = prim->indices;
				mdl->meshes[m].indc = attr->count;
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

				mdl->meshes[m].ibo = ibo_new(mdl->meshes[m].indc*sizeof(uint16), inds);
			} else {
				ERROR("[RES] GLTF file does not provide indices (non-indexed drawing not supported)");
			}

			/* Assign materials to their meshes */
			for (int i = 0; i < (int)data->materials_count; i++) {
				if (&data->materials[i] == data->meshes[m].primitives[p].material) // !
					mdl->meshes[i].mtli = i;
			}
		}

		/* Make sure there's a dummy vbo for uvs if there aren't any */
		float duvs[] = { 0.0f, 0.0f, };
		if (!mdl->meshes[m].vbos[2].buf)
			mdl->meshes[m].vbos[2] = vbo_new(1, duvs, false);
	}

	arena_free(&mesh_mem);
}

// static void load_skin(Model* model, cgltf_data* data)
// {
// 	if (data->skins_count > 1)
// 		ERROR("[RES] Warning: only one of the %lu skins will be loaded", data->skins_count);
// 	else
// 		INFO(TERM_DARK_GREEN "\tModel has %lu skins", data->skins_count);

// 	cgltf_skin* skin;
// 	cgltf_node* node;
// 	Joint* joint;
// 	if (data->skins_count) {
// 		skin = data->skins;
// 		model->skin = smalloc(skin->joints_count*sizeof(Joint) + sizeof(Skin));
// 		model->skin->jointc = (int)skin->joints_count;

// 		float* inv_binds = smalloc(skin->inverse_bind_matrices->count*skin->inverse_bind_matrices->stride);
// 		cgltf_accessor_unpack_floats(skin->inverse_bind_matrices, inv_binds, skin->inverse_bind_matrices->count*skin->inverse_bind_matrices->stride);

// 		for (int j = 0; j < (int)skin->joints_count; j++) {
// 			joint = &model->skin->joints[j];
// 			node  = skin->joints[j];
// 			if (node->mesh)
// 				ERROR("[RES] Ignoring node mesh data");

// 			memcpy(joint->inv_bind, &inv_binds[16*j], sizeof(float[16]));

// 			joint->parent = -1;
// 			for (int j2 = 0; j2 < (int)skin->joints_count; j2++) {
// 				if (skin->joints[j2] == node->parent && j2 != j) {
// 					joint->parent = j2;
// 					break;
// 				}
// 			}

// 			cgltf_node_transform_local(node, joint->tform);
// 		}

// 		model->skin->sbo = sbo_new(model->skin->jointc*sizeof(Joint) + sizeof(Skin));
// 		// buffer_update(model->skin->sbo, model->skin->sbo.sz, inv_binds, 0);
// 		free(inv_binds);
// 	}
// }

// static void load_animations(Model* mdl, cgltf_data* data)
// {
// 	INFO(TERM_DARK_GREEN "\tModel has %lu animations", data->animations_count);
// 	mdl->animc = data->animations_count;
// 	mdl->anims = scalloc(data->animations_count, sizeof(ModelAnimation));

// 	cgltf_animation*         anim;
// 	cgltf_animation_channel* channel;
// 	cgltf_animation_sampler* sampler;
// 	for (int a = 0; a < (int)data->animations_count; a++) {
// 		anim = &data->animations[a];

// 		int framec = 0;
// 		for (int c = 0; c < (int)anim->channels_count; c++)
// 			framec = MAX(framec, (int)anim->channels[c].sampler->input->count);
// 		mdl->anims[a].frmc     = framec;
// 		mdl->anims[a].frms     = scalloc(framec, sizeof(KeyFrame));
// 		mdl->anims[a].curr_frm = 0;

// 		// TODO: Find a method that doesn't allocate for unchanged joints?
// 		for (int f = 0; f < framec; f++)
// 			mdl->anims[a].frms[f].tforms = scalloc(MODEL_MAX_JOINTS, sizeof(Transform));

// 		cgltf_accessor* input;
// 		cgltf_accessor* output;
// 		float* verts;
// 		for (int c = 0; c < (int)anim->channels_count; c++) {
// 			channel = &anim->channels[c];
// 			sampler = channel->sampler;
// 			input   = sampler->input;
// 			output  = sampler->output;
// 			if (input->type != cgltf_type_scalar)
// 				ERROR("[RES] Input accessor type was expected to be scalar (%d), got: %d",
// 				      cgltf_type_scalar, input->type);
// 			if (output->type != cgltf_type_vec3 && output->type != cgltf_type_vec4)
// 				ERROR("[RES] Output accessor type was expected to be vec3 (%d) or vec4 (%d), got: %d",
// 				      cgltf_type_vec3, cgltf_type_vec4, output->type);
// 			if (input->component_type != cgltf_component_type_r_32f || output->component_type != cgltf_component_type_r_32f)
// 				ERROR("[RES] Input/Output accessor component type was expected to be float (%d), got: %d/%d",
// 				      cgltf_component_type_r_32f, input->component_type, output->component_type);

// 			/* Times for the keyframes */
// 			verts = (float*)input->buffer_view->buffer->data + input->buffer_view->offset/sizeof(float) + input->offset/sizeof(float);
// 			for (int f = 0, i = 0; f < (int)input->count; f++) {
// 				mdl->anims[a].frms[f].time = verts[i];
// 				i += (int)(input->stride/sizeof(float));
// 			}

// 			/* Find the joint this channel is for */
// 			int joint = -1;
// 			for (int i = 0; i < (int)data->skins[0].joints_count; i++) {
// 				if (data->skins[0].joints[i] == channel->target_node) {
// 					joint = i;
// 					break;
// 				}
// 			}
// 			if (joint == -1) {
// 				ERROR("[RES] Channel does not seem to be linked to any joint");
// 				continue;
// 			}

// 			verts = (float*)output->buffer_view->buffer->data + output->buffer_view->offset/sizeof(float) + output->offset/sizeof(float);
// 			for (int f = 0, i = 0; f < (int)output->count; f++) {
// 				switch (channel->target_path) {
// 				case cgltf_animation_path_type_rotation:
// 					memcpy(&mdl->anims[a].frms[f].tforms[joint].rot.arr, &verts[i], sizeof(float[4]));
// 					break;
// 				case cgltf_animation_path_type_translation:
// 					memcpy(&mdl->anims[a].frms[f].tforms[joint].trans.arr, &verts[i], sizeof(float[3]));
// 					break;
// 				case cgltf_animation_path_type_scale:
// 					mdl->anims[a].frms[f].tforms[joint].scale = verts[i];
// 					break;
// 				default:
// 					ERROR("[RES] Unsupported target path: %d", channel->target_path);
// 				}
// 				i += (int)(output->stride/sizeof(float));
// 			}
// 		}
// 	}

// 	// INFO(TERM_DARK_GREEN " --- Model Animation --- ");
// 	// Animation* danim;
// 	// KeyFrame*  frame;
// 	// Transform* trans;
// 	// for (int a = 0; a < mdl->animc; a++) {
// 	// 	danim = &mdl->anims[a];
// 	// 	for (int f = 0; f < danim->frmc; f++) {
// 	// 		frame = &danim->frms[f];
// 	// 		fprintf(stderr, "Frame %d: \n", f);
// 	// 		for (int j = 0; j < mdl->skin->jointc; j++) {
// 	// 			trans = &frame->tforms[j];
// 	// 			INFO(TERM_DARK_GREEN "\t[%d::%.2f]\tRot (%5.2f, %5.2f, %5.2f, %5.2f);    \tTrans (%5.2f, %5.2f, %5.2f); Scale (%5.2f)",
// 	// 			      j, frame->time, trans->rot.x, trans->rot.y, trans->rot.z, trans->rot.w,
// 	// 			      trans->trans.x, trans->trans.y, trans->trans.z, trans->scale);
// 	// 		}
// 	// 	}
// 	// }
// }


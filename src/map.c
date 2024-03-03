#include "vulkan/vulkan.h"
#define JSMN_PARENT_LINKS
#include "jsmn/jsmn.h"

#include "util/file.h"
#include "util/string.h"
#include "util/varray.h"
#include "maths/maths.h"
#include "gfx/vulkan.h"
#include "gfx/buffers.h"
#include "gfx/pipeline.h"
#include "gfx/texture.h"
#include "gfx/model.h" // TODO: move materials to separate file
#include "camera.h"
#include "map.h"

#define VERTEX_ELEMENTS    5
#define SIZEOF_VERTEX      sizeof(uint8[VERTEX_ELEMENTS])
#define BUFFER_SIZE        4096
#define MAX_CHUNKS         256
#define VERTEX_WIDTH       (MAP_CHUNK_WIDTH  + 1)
#define VERTEX_HEIGHT      (MAP_CHUNK_HEIGHT + 1)
#define VERTEX_DEPTH       (MAP_CHUNK_DEPTH  + 1)
#define VERTICES_PER_CHUNK (VERTEX_WIDTH*VERTEX_HEIGHT*VERTEX_DEPTH)
#define VERTEX_STRIDE      (12*VERTEX_ELEMENTS)

#define STRING_OF_JSMN_TYPE(x) (           \
	x == JSMN_UNDEFINED? "JSMN_UNDEFINED": \
	x == JSMN_OBJECT   ? "JSMN_OBJECT"   : \
	x == JSMN_ARRAY    ? "JSMN_ARRAY"    : \
	x == JSMN_STRING   ? "JSMN_STRING"   : \
	x == JSMN_PRIMITIVE? "JSMN_PRIMITIVE": \
	"<Not a JSMN type>")

static void parse_map(struct Map* map, char* json, jsmntok_t* tokens, isize tokenc);
static int parse_layer(struct MapLayer* layer, int i, char* json, jsmntok_t* tokens);
static int parse_chunk(struct MapChunk* chunk, int i, char* json, jsmntok_t* tokens);
static int parse_data(uint8* data, int i, char* json, jsmntok_t* tokens);
static int parse_tileset(struct Tileset* tset, int i, char* json, jsmntok_t* tokens);
static void build_mesh(struct Map* map);
static void mesh_tile(uint16* inds, Vec3i v);

static VkRenderPass renderpass;
static VkVertexInputBindingDescription vert_binds[] = {
	{ .binding   = 0,
	  .stride    = SIZEOF_VERTEX, /* xyzuv */
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, }
};
static VkVertexInputAttributeDescription vert_attrs[] = {
	{ .binding  = 0,
	  .location = 0,
	  .format   = VK_FORMAT_R8G8B8_UINT, /* xyz */
	  .offset   = 0, },
	{ .binding  = 0,
	  .location = 1,
	  .format   = VK_FORMAT_R8G8_UINT, /* uv */
	  .offset   = sizeof(uint8[3]), },
};

static UBO cam_ubo;
static VBO lattice_vbo;
static char buf[BUFFER_SIZE];

void maps_init(VkRenderPass rpass)
{
	renderpass = rpass;

	cam_ubo = ubo_new(sizeof(Mat4x4[2]));

	/* Generate the vertex lattice -> 3 versions, 1 for each normal */
	intptr vertc = 12*MAP_CHUNK_WIDTH*MAP_CHUNK_HEIGHT*MAP_CHUNK_DEPTH;
	uint8* verts = scalloc(vertc, sizeof(uint8[VERTEX_ELEMENTS]));
	int i = 0;
	for (int z = 0; z < MAP_CHUNK_DEPTH; z++) {
		for (int y = 0; y < MAP_CHUNK_HEIGHT; y++) {
			for (int x = 0; x < MAP_CHUNK_WIDTH; x++) {
				/* For uvs: 0 = 0.0f; 1 = 0.25f; 2 = 0.5f; 3 = 0.75f; 4 = 1.0f */
				#define V(vi) verts[VERTEX_STRIDE*i + vi]
				V(0 ) = x    ; V(1 ) = y    ; V(2 ) = z    ; V(3 ) = 2; V(4 ) = 4;
				V(5 ) = x + 1; V(6 ) = y    ; V(7 ) = z    ; V(8 ) = 0; V(9 ) = 3;
				V(10) = x + 1; V(11) = y + 1; V(12) = z    ; V(13) = 0; V(14) = 1;
				V(15) = x    ; V(16) = y + 1; V(17) = z    ; V(18) = 2; V(19) = 2;

				V(20) = x    ; V(21) = y + 1; V(22) = z    ; V(23) = 2; V(24) = 2;
				V(25) = x + 1; V(26) = y + 1; V(27) = z    ; V(28) = 0; V(29) = 1;
				V(30) = x + 1; V(31) = y + 1; V(32) = z + 1; V(33) = 2; V(34) = 0;
				V(35) = x    ; V(36) = y + 1; V(37) = z + 1; V(38) = 4; V(39) = 1;

				V(40) = x + 1; V(41) = y + 1; V(42) = z    ; V(43) = 2; V(44) = 2;
				V(45) = x + 1; V(46) = y    ; V(47) = z    ; V(48) = 2; V(49) = 4;
				V(50) = x + 1; V(51) = y    ; V(52) = z + 1; V(53) = 4; V(54) = 3;
				V(55) = x + 1; V(56) = y + 1; V(57) = z + 1; V(58) = 4; V(59) = 1;
				#undef V
				i++;
			}
		}
	}
	lattice_vbo = vbo_new(vertc*sizeof(uint8[VERTEX_ELEMENTS]), verts, false);
	free(verts);

	DEBUG(3, "[MAP] Map system initialized");
}

void map_new(struct Map* map, const char* name)
{
	char path[256];
	sprintf(path, MAP_PATH "%s.tmj", name);
	FILE* file = file_open(path, "r");

	isize len  = file_size(file);
	char* json = file_loadf(file);

	jsmn_parser parser;
	jsmn_init(&parser);
	isize tokenc = jsmn_parse(&parser, json, len, NULL, -1);

	jsmntok_t* tokens = NULL; /* Can do this on the stack with a static pool */
	jsmn_init(&parser);
	switch (tokenc) {
	case JSMN_ERROR_INVAL:
		ERROR("[MAP] Invalid JSON map file (JSMN_ERROR_INVAL) for \"%s\"", path);
		goto cleanup;
	case JSMN_ERROR_PART:
		ERROR("[MAP] JSON map file appears to be incomplete (JSMN_ERROR_PART) for \"%s\"", path);
		goto cleanup;
	case JSMN_ERROR_NOMEM: assert(false);
	default:
		tokens = scalloc(tokenc, sizeof(jsmntok_t));
		jsmn_parse(&parser, json, len, tokens, tokenc);
	}

	if (tokenc < 1 || tokens[0].type != JSMN_OBJECT) {
		ERROR("[MAP] Expected top-level JSON object, got: %s (%d)",
		      STRING_OF_JSMN_TYPE(tokens[0].type), tokens[0].type);
		goto cleanup;
	}

	parse_map(map, json, tokens, tokenc);
	build_mesh(map);

	map->ubo = ubo_new(sizeof(struct MapData));

	map->pipeln = (struct Pipeline){
		.vshader     = create_shader(SHADER_DIR "map.vert"),
		.fshader     = create_shader(SHADER_DIR "map.frag"),
		.topology    = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.vert_bindc  = 1,
		.vert_binds  = vert_binds,
		.vert_attrc  = 2,
		.vert_attrs  = vert_attrs,
		.push_stages = VK_SHADER_STAGE_VERTEX_BIT,
		.push_sz     = sizeof(Vec3i),
		.dset_cap    = 1,
		.uboc        = 2,
		.sboc        = 0,
		.imgc        = 1,
	};
	pipeln_alloc_dsets(&map->pipeln);
	pipeln_create_dset(&map->pipeln, 2, (UBO[]){ cam_ubo, map->ubo }, 0, NULL, 1, &map->tileset.texture.image_view);
	pipeln_init(&map->pipeln, renderpass);

cleanup:
	if (json)
		free(json);
	if (tokens) {
		free(tokens);
		DEBUG(2, "[MAP] Loaded \"%s\" (%hux%hu) (%ld tokens)", path, map->w, map->h, tokenc);
	}
}

void map_record_commands(VkCommandBuffer cmd_buf, struct Camera* cam, struct Map* map)
{
	buffer_update(cam_ubo, sizeof(Mat4x4[2]), (Mat4x4[]){ cam->mats->proj, cam->mats->view }, 0);

	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, map->pipeln.pipeln);
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, map->pipeln.layout, 0, 1, map->pipeln.dsets, 0, NULL);

	struct MapLayer* layer;
	struct MapChunk* chunk;
	vkCmdBindVertexBuffers(cmd_buf, 0, 1, &lattice_vbo.buf, (VkDeviceSize[]){ 0 });
	for (int i = 0; i < map->layerc; i++) {
		layer = &map->layers[i];
		for (int j = 0; j < layer->chunkc; j++) {
			chunk = &layer->chunks[j];
			memcpy(map->data.block_data, chunk->data, sizeof(map->data.block_data));
			buffer_update(map->ubo, sizeof(struct MapData), &map->data, 0);

			// DEBUG(1, "[%d] Drawing %d tiles (%d)", j, chunk->tilec, chunk->tilec*18);
			vkCmdBindIndexBuffer(cmd_buf, layer->chunks[j].ibo.buf, 0, VK_INDEX_TYPE_UINT16);
			vkCmdPushConstants(cmd_buf, map->pipeln.layout, map->pipeln.push_stages, 0, map->pipeln.push_sz, &chunk->pos);
			vkCmdDrawIndexed(cmd_buf, chunk->tilec*18, 1, 0, 0, 0);
		}
	}
}

void map_free(struct Map* map)
{
	for (int i = 0; i < map->layerc; i++) {
		for (int j = 0; j < map->layers[i].chunkc; j++)
			ibo_free(&map->layers[i].chunks[j].ibo);
		free(map->layers[i].chunks);
	}
	free(map->layers);
	ubo_free(&map->ubo);
	texture_free(&map->tileset.texture);
	pipeln_free(&map->pipeln);
	*map = (struct Map){ 0 };
}

void maps_free()
{
	ubo_free(&cam_ubo);
	vbo_free(&lattice_vbo);
}

/* -------------------------------------------------------------------- */

#define NEXT() do {                                                            \
		token = tokens[i++];                                                    \
		if (token.end - token.start < BUFFER_SIZE)                               \
			snprintf(buf, token.end - token.start + 1, "%s", json + token.start); \
	} while (0)

static void parse_map(struct Map* map, char* json, jsmntok_t* tokens, isize tokenc)
{
	jsmntok_t token = tokens[0];
	isize len = token.size;
	int i = 0;
	NEXT();
	NEXT();
	for (int j = 0; j < len; j++) {
		switch (token.type) {
		case JSMN_PRIMITIVE:
		case JSMN_STRING:
			if (!strcmp(buf, "orientation")) {
				NEXT();
				if (strcmp(buf, "isometric"))
					ERROR("[MAP] Map has type \"%s\", should be \"isometric\"", buf);
			} else if (!strcmp(buf, "tilewidth")) {
				NEXT();
				int tw = atoi(buf);
				if (tw != 128)
					ERROR("[MAP] Tile width should be 128px, got: %d", tw);
			} else if (!strcmp(buf, "tileheight")) {
				NEXT();
				int th = atoi(buf);
				if (th != 64)
					ERROR("[MAP] Tile height should be 64px, got: %d", th);
			} else if (!strcmp(buf, "compressionlevel")) {
				DEBUG(1, "[MAP] TODO: compressionlevel (%s)", buf);
				NEXT();
			} else if (!strcmp(buf, "tilesets")) {
				NEXT();
				int tsetc = token.size;
				if (tsetc > 1)
					ERROR("[MAP] Only one tileset supported, got: %d tilesets", tsetc);
				NEXT();
				i = parse_tileset(&map->tileset, i, json, tokens);
			} else if (!strcmp(buf, "layers")) {
				int layerc = tokens[i].size;
				map->layers = smalloc(layerc * sizeof(struct MapLayer));
				NEXT();
				NEXT();
				for (int l = 0; l < layerc; l++)
					i = parse_layer(&map->layers[map->layerc++], i, json, tokens);
				i--;
			}
			else if (!strcmp(buf, "width"))      { NEXT(); map->w  = atoi(buf); }
			else if (!strcmp(buf, "height"))     { NEXT(); map->h  = atoi(buf); }
			else if (!strcmp(buf, "tilewidth"))  { NEXT(); map->tw = atoi(buf); }
			else if (!strcmp(buf, "tileheight")) { NEXT(); map->th = atoi(buf); }
			else if (!strcmp(buf, "infinite"))     { NEXT(); }
			else if (!strcmp(buf, "nextlayerid"))  { NEXT(); }
			else if (!strcmp(buf, "nextobjectid")) { NEXT(); }
			else if (!strcmp(buf, "renderorder"))  { NEXT(); }
			else if (!strcmp(buf, "tiledversion")) { NEXT(); }
			else if (!strcmp(buf, "type"))         { NEXT(); }
			else if (!strcmp(buf, "version"))      { NEXT(); }
			else {
				ERROR("[MAP] Unmatched value: \"%s\"", buf);
				NEXT();
			}
			if (i < tokenc)
				NEXT();
			break;
		case JSMN_ARRAY:
			ERROR("[MAP] Unmatched array: \"%s\"", buf);
			i += token.size + 1;
			break;
		default:
			ERROR("[MAP] Unmatched token \"%s\" (%s)", buf, STRING_OF_JSMN_TYPE(token.type));
			i += token.size + 1;
		}
	}
}

static int parse_layer(struct MapLayer* layer, int i, char* json, jsmntok_t* tokens)
{
	jsmntok_t token = tokens[i - 1];
	struct VArray chunks = varray_new(16, sizeof(struct MapChunk));

	int len = token.size;
	NEXT();
	for (int j = 0; j < len; j++) {
		if (!strcmp(buf, "chunks")) {
			struct MapChunk chunk;
			NEXT();
			int chunkc = token.size;
			NEXT();
			for (int c = 0; c < chunkc; c++) {
				chunk = (struct MapChunk){ 0 };
				i = parse_chunk(&chunk, i, json, tokens);
				varray_push(&chunks, &chunk);
			}
		}
		else if (!strcmp(buf, "name"))    { NEXT(); strncpy(layer->name, buf, sizeof(layer->name)); NEXT(); }
		else if (!strcmp(buf, "id"))      { NEXT(); NEXT(); }
		else if (!strcmp(buf, "type"))    { NEXT(); NEXT(); }
		else if (!strcmp(buf, "opacity")) { NEXT(); NEXT(); }
		else if (!strcmp(buf, "visible")) { NEXT(); NEXT(); }
		else if (!strcmp(buf, "width"))   { NEXT(); layer->w = atoi(buf); NEXT(); }
		else if (!strcmp(buf, "height"))  { NEXT(); layer->h = atoi(buf); NEXT(); }
		else if (!strcmp(buf, "x"))       { NEXT(); layer->x = atoi(buf); NEXT(); }
		else if (!strcmp(buf, "y"))       { NEXT(); layer->y = atoi(buf); NEXT(); }
		else if (!strcmp(buf, "startx"))  { NEXT(); NEXT(); }
		else if (!strcmp(buf, "starty"))  { NEXT(); NEXT(); }
		else {
			ERROR("[MAP] Unmatched layer token : \"%s\"", buf);
			NEXT();
			ERROR("[MAP] \"%s\"", buf);
			NEXT();
		}
	}

	layer->chunkc = chunks.len;
	layer->chunks = smalloc(chunks.len*sizeof(struct MapChunk));
	memcpy(layer->chunks, chunks.data, chunks.len*sizeof(struct MapChunk));
	varray_free(&chunks);
	return i;
}

static int parse_chunk(struct MapChunk* chunk, int i, char* json, jsmntok_t* tokens)
{
	jsmntok_t token = tokens[i - 1];

	int len = token.size;
	NEXT();
	for (int j = 0; j < len; j++) {
		if (!strcmp(buf, "data")) {
			if (token.size > UINT8_MAX + 1)
				ERROR("[MAP] Map chunks need to be <= 256 tiles");
			i = parse_data(chunk->data, i, json, tokens);
		}
		else if (!strcmp(buf, "width"))  { NEXT(); NEXT(); }
		else if (!strcmp(buf, "height")) { NEXT(); NEXT(); }
		else if (!strcmp(buf, "x"))      { NEXT(); chunk->pos.x = atoi(buf); NEXT(); }
		else if (!strcmp(buf, "y"))      { NEXT(); chunk->pos.y = atoi(buf); NEXT(); }
		else {
			ERROR("[MAP] Unmatched chunk token: \"%s\"", buf);
			NEXT();
		}
	}

	return i;
}

static int parse_data(uint8* data, int i, char* json, jsmntok_t* tokens)
{
	jsmntok_t token;

	NEXT();
	int len = token.size;
	for (int j = 0; j < len; j++) {
		NEXT();
		data[j] = atoi(buf);
	}

	NEXT();
	return i;
}

#define TSET_NEXT() do {                                                                        \
		tset_token = tset_tokens[k++];                                                           \
		snprintf(buf, tset_token.end - tset_token.start + 1, "%s", tset_json + tset_token.start); \
	} while (0)
static int parse_tileset(struct Tileset* tset, int i, char* json, jsmntok_t* tokens)
{
	jsmntok_t token = tokens[i - 1];
	int len = token.size;

	NEXT();
	char path[256];
	jsmntok_t tset_tokens[256];
	for (int j = 0; j < len; j++) {
		if (!strcmp(buf, "source")) {
			NEXT();
			sprintf(path, MAP_PATH "%s", buf);
			FILE* file = file_open(path, "r");

			isize tset_len  = file_size(file);
			char* tset_json = file_loadf(file);

			jsmn_parser tset_parser;
			jsmn_init(&tset_parser);
			isize tset_tokenc = jsmn_parse(&tset_parser, tset_json, tset_len, tset_tokens, 256);
			if (tset_tokenc == JSMN_ERROR_INVAL) {
				ERROR("[MAP] Invalid JSON tileset file (JSMN_ERROR_INVAL) for \"%s\"", path);
			} else if (tset_tokenc == JSMN_ERROR_PART) {
				ERROR("[MAP] JSON tileset file appears to be incomplete (JSMN_ERROR_PART) for \"%s\"", path);
			} else if (tset_tokenc == JSMN_ERROR_NOMEM) {
				assert(false);
			} else {
				if (tset_tokenc < 1 || tset_tokens[0].type != JSMN_OBJECT) {
					ERROR("[MAP] Expected top-level JSON object, got: %s (%d)",
						STRING_OF_JSMN_TYPE(tset_tokens[0].type), tset_tokens[0].type);
					return i;
				}
				jsmntok_t tset_token;
				int k = 0;
				TSET_NEXT();
				while (k < tset_tokenc) {
					TSET_NEXT();
					if (!strcmp(buf, "image")) {
						TSET_NEXT();
						string_remove((String){ .data = buf, .len = strlen(buf) }, '\\');
						sprintf(path, MAP_PATH "%s", buf);
						strcpy(tset->image, path);
						tset->texture = texture_new_from_image(path);
					}
					else if (!strcmp(buf, "name"))       { TSET_NEXT(); strcpy(tset->name, buf);   }
					else if (!strcmp(buf, "margin"))     { TSET_NEXT(); tset->margin  = atoi(buf); }
					else if (!strcmp(buf, "spacing"))    { TSET_NEXT(); tset->spacing = atoi(buf); }
					else if (!strcmp(buf, "columns"))    { TSET_NEXT(); tset->columns = atoi(buf); }
					else if (!strcmp(buf, "tilewidth"))  { TSET_NEXT(); tset->tw      = atoi(buf); }
					else if (!strcmp(buf, "tileheight")) { TSET_NEXT(); tset->th      = atoi(buf); }
					else if (!strcmp(buf, "firstgid"))     { TSET_NEXT(); }
					else if (!strcmp(buf, "imagewidth"))   { TSET_NEXT(); }
					else if (!strcmp(buf, "imageheight"))  { TSET_NEXT(); }
					else if (!strcmp(buf, "tilecount"))    { TSET_NEXT(); }
					else if (!strcmp(buf, "tiledversion")) { TSET_NEXT(); }
					else if (!strcmp(buf, "type"))         { TSET_NEXT(); }
					else if (!strcmp(buf, "version"))      { TSET_NEXT(); }
					else {
						ERROR("[MAP] Unmatched tileset value: %s", buf);
						TSET_NEXT();
					}
				}
			}

			free(tset_json);
		} else if (!strcmp(buf, "firstgid")) {
			NEXT();
			NEXT();
		}
	}

	return i;
}

static void build_mesh(struct Map* map)
{
	uint16* inds = smalloc(MAP_CHUNK_WIDTH*MAP_CHUNK_HEIGHT*2*sizeof(uint16[18]));
	struct MapLayer* layer;
	struct MapChunk* chunk;
	for (int i = 0; i < map->layerc; i++) {
		layer = &map->layers[i];
		for (int j = 0; j < layer->chunkc; j++) {
			chunk = &layer->chunks[j];
			for (int k = 0; k < MAP_CHUNK_SIZE; k++) {
				if (chunk->data[k]) {
					mesh_tile(&inds[18*chunk->tilec++], VEC3I(k % MAP_CHUNK_WIDTH,
					                                          k / MAP_CHUNK_WIDTH,
					                                          chunk->pos.z));
				}
			}
			chunk->ibo = ibo_new(18*chunk->tilec*sizeof(inds[0]), inds);
		}
	}

	free(inds);
}

static void mesh_tile(uint16* inds, Vec3i v)
{
	#define V(vi) (vert_start + vi - 1)
	int vert_start = ((v.z)*MAP_CHUNK_WIDTH*MAP_CHUNK_HEIGHT + (v.y)*MAP_CHUNK_WIDTH + v.x);
	/* Top: 0-3 */
	*inds++ = V(0); *inds++ = V(1); *inds++ = V(2);
	*inds++ = V(0); *inds++ = V(3); *inds++ = V(2);

	/* Left: 4-7 */
	*inds++ = V(4); *inds++ = V(5); *inds++ = V(6);
	*inds++ = V(6); *inds++ = V(7); *inds++ = V(4);

	/* Right: 8-11 */
	*inds++ = V(8 ); *inds++ = V(9 ); *inds++ = V(10);
	*inds++ = V(10); *inds++ = V(11); *inds++ = V(8 );
	#undef V
}

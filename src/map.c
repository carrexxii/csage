#include "vulkan/vulkan.h"
#define JSMN_PARENT_LINKS
#include "jsmn/jsmn.h"

#include "util/file.h"
#include "util/string.h"
#include "util/varray.h"
#include "util/arena.h"
#include "maths/maths.h"
#include "gfx/vulkan.h"
#include "gfx/buffers.h"
#include "gfx/pipeline.h"
#include "gfx/texture.h"
#include "gfx/renderer.h"
#include "gfx/model.h" // TODO: move materials to separate file
#include "camera.h"
#include "map.h"

#define VERTEX_ELEMENTS    6
#define SIZEOF_VERTEX      sizeof(byte[VERTEX_ELEMENTS])
#define BUFFER_SIZE        4096
#define MAX_CHUNKS         256
#define VERTEX_WIDTH       (MAP_CHUNK_WIDTH  + 1)
#define VERTEX_HEIGHT      (MAP_CHUNK_HEIGHT + 1)
#define VERTEX_DEPTH       2
#define VERTICES_PER_CHUNK (VERTEX_WIDTH*VERTEX_HEIGHT*VERTEX_DEPTH)
#define VERTEX_STRIDE      (12*VERTEX_ELEMENTS)

#define STRING_OF_JSMN_TYPE(x) (           \
	x == JSMN_UNDEFINED? "JSMN_UNDEFINED": \
	x == JSMN_OBJECT   ? "JSMN_OBJECT"   : \
	x == JSMN_ARRAY    ? "JSMN_ARRAY"    : \
	x == JSMN_STRING   ? "JSMN_STRING"   : \
	x == JSMN_PRIMITIVE? "JSMN_PRIMITIVE": \
	"<Not a JSMN type>")

#define PROPERTY_TYPE_OF_STRING(x) (         \
	!strcmp((x), "int")   ? PROPERTY_INT   : \
	!strcmp((x), "float") ? PROPERTY_FLOAT : \
	!strcmp((x), "bool")  ? PROPERTY_BOOL  : \
	!strcmp((x), "string")? PROPERTY_STRING: \
	!strcmp((x), "color") ? PROPERTY_COLOUR: \
	!strcmp((x), "file")  ? PROPERTY_FILE  : \
	!strcmp((x), "object")? PROPERTY_OBJECT: \
	!strcmp((x), "class") ? PROPERTY_CLASS : \
	PROPERTY_NONE)
enum PropertyType {
	PROPERTY_NONE,
	PROPERTY_INT,
	PROPERTY_FLOAT,
	PROPERTY_BOOL,
	PROPERTY_STRING,
	PROPERTY_COLOUR,
	PROPERTY_FILE,
	PROPERTY_OBJECT,
	PROPERTY_CLASS,
};
struct Property {
	String key;
	enum PropertyType type;
	union {
		int64  vint;
		double vfloat;
		bool   vbool;
		String vstring;
		uint8  vcolour[4];
		String vfile;
		struct Property* vobject;
	} value;
};

static void parse_map(struct Map* map, char* json, jsmntok_t* tokens, isize tokenc);
static int parse_layer(struct MapLayer* layer, int i, char* json, jsmntok_t* tokens);
static int parse_chunk(struct MapChunk* chunk, int i, char* json, jsmntok_t* tokens);
static int parse_data(uint32* data, int i, char* json, jsmntok_t* tokens);
static int parse_object(struct MapObject* obj, int i, char* json, jsmntok_t* tokens);
static int parse_tileset(struct Tileset* tset, int i, char* json, jsmntok_t* tokens);
static int parse_property(struct Property* prop, int i, char* json, jsmntok_t* tokens);
static void build_lights(struct Map* map);
static void build_mesh(struct Map* map);
static void mesh_tile(uint16* inds, Vec2i v);

static VkVertexInputBindingDescription vert_binds[] = {
	{ .binding   = 0,
	  .stride    = SIZEOF_VERTEX, /* xyztuv */
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, }
};
static VkVertexInputAttributeDescription vert_attrs[] = {
	{ .binding  = 0,
	  .location = 0,
	  .format   = VK_FORMAT_R8G8B8_UINT, /* xyz */
	  .offset   = 0, },
	{ .binding  = 0,
	  .location = 1,
	  .format   = VK_FORMAT_R8_UINT, /* tile number */
	  .offset   = sizeof(byte[3]), },
	{ .binding  = 0,
	  .location = 2,
	  .format   = VK_FORMAT_R8G8_UINT, /* uv */
	  .offset   = sizeof(byte[4]), },
};

static UBO cam_ubo;
static VBO lattice_vbo;
static char buf[BUFFER_SIZE];
static struct Arena* arena;

static UBO lights_ubo;
static struct {
	int point_lightc;
	int spot_lightc;
	float pad1[2];
	struct PointLight point_lights[MAP_POINT_LIGHTS_PER_CHUNK];
	struct SpotLight  spot_lights[MAP_SPOT_LIGHTS_PER_CHUNK];
} lights_data;

void maps_init()
{
	cam_ubo    = ubo_new(sizeof(Mat4x4[2]));
	lights_ubo = ubo_new(sizeof(lights_data));

	arena = arena_new(16*1024, 0);

	/* Generate the vertex lattice -> 3 versions, 1 for each normal */
	isize vertc = 12*MAP_CHUNK_WIDTH*MAP_CHUNK_HEIGHT;
	uint8* verts = smalloc(vertc*sizeof(uint8[VERTEX_ELEMENTS]));
	isize vc;
	uint i = 0;
	for (int y = 0; y < MAP_CHUNK_HEIGHT; y++) {
		for (int x = 0; x < MAP_CHUNK_WIDTH; x++) {
			/* x | y | z | normal | tile_id | uv.x | uv.y */
			/* For uvs: 0 = 0.0f; 1 = 0.25f; 2 = 0.5f; 3 = 0.75f; 4 = 1.0f */
			#define V(vi) verts[VERTEX_STRIDE*i + vi]
			vc = 0;

			/* Left */
			V(vc++) = x    ; V(vc++) = y    ; V(vc++) = 0; V(vc++) = i; V(vc++) = 2; V(vc++) = 0;
			V(vc++) = x + 1; V(vc++) = y    ; V(vc++) = 0; V(vc++) = i; V(vc++) = 4; V(vc++) = 1;
			V(vc++) = x + 1; V(vc++) = y + 1; V(vc++) = 0; V(vc++) = i; V(vc++) = 2; V(vc++) = 2;
			V(vc++) = x    ; V(vc++) = y + 1; V(vc++) = 0; V(vc++) = i; V(vc++) = 0; V(vc++) = 1;

			/* Top */
			V(vc++) = x    ; V(vc++) = y + 1; V(vc++) = 0; V(vc++) = i; V(vc++) = 0; V(vc++) = 1;
			V(vc++) = x + 1; V(vc++) = y + 1; V(vc++) = 0; V(vc++) = i; V(vc++) = 2; V(vc++) = 2;
			V(vc++) = x + 1; V(vc++) = y + 1; V(vc++) = 1; V(vc++) = i; V(vc++) = 2; V(vc++) = 4;
			V(vc++) = x    ; V(vc++) = y + 1; V(vc++) = 1; V(vc++) = i; V(vc++) = 0; V(vc++) = 3;

			/* Right */
			V(vc++) = x + 1; V(vc++) = y + 1; V(vc++) = 0; V(vc++) = i; V(vc++) = 2; V(vc++) = 2;
			V(vc++) = x + 1; V(vc++) = y    ; V(vc++) = 0; V(vc++) = i; V(vc++) = 4; V(vc++) = 1;
			V(vc++) = x + 1; V(vc++) = y    ; V(vc++) = 1; V(vc++) = i; V(vc++) = 4; V(vc++) = 3;
			V(vc++) = x + 1; V(vc++) = y + 1; V(vc++) = 1; V(vc++) = i; V(vc++) = 2; V(vc++) = 4;

			i++;
			#undef V
		}
	}
	lattice_vbo = vbo_new(vertc*sizeof(uint8[VERTEX_ELEMENTS]), verts, false);
	free(verts);

	DEBUG(3, "[MAP] Map system initialized");
}

void map_new(struct Map* map, const char* name)
{
	char path[PATH_BUFFER_SIZE];
	snprintf(path, PATH_BUFFER_SIZE, MAP_PATH "/%s.tmj", name);
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

	struct MapLayer* layer;
	struct MapChunk* chunk;
	int minx = 0, miny = 0;
	int maxx = 0, maxy = 0;
	for (int l = 0; l < map->layerc; l++) {
		layer = &map->layers[l];
		if (map->layers[l].type == MAP_LAYER_TILE) {
			for (int c = 0; c < layer->tile.chunkc; c++) {
				chunk = &layer->tile.chunks[c];
				minx = MIN(minx, chunk->x);
				miny = MIN(miny, chunk->y);
				maxx = MAX(maxx, chunk->x);
				maxy = MAX(maxy, chunk->y);
			}
		}
	}
	map->cw = (maxx - minx) / MAP_CHUNK_WIDTH  + 1;
	map->ch = (maxy - miny) / MAP_CHUNK_HEIGHT + 1;

	map->ubo = ubo_new(sizeof(Vec2i[2]));
	buffer_update(map->ubo, sizeof(Vec2i[2]), (Vec2i[]){
		VEC2I(map->cw, map->ch),
		VEC2I(4096 / 128, 4096 / 128),
	}, 0);

	build_lights(map);
	build_mesh(map);

	map->chunks_sbo = sbo_new(map->cw*map->ch*sizeof(struct MapChunkData));

	for (int l = 0; l < map->layerc; l++) {
		if (map->layers[l].type != MAP_LAYER_TILE)
			continue;

		layer = &map->layers[l];
		isize offset;
		for (int c = 0; c < layer->tile.chunkc; c++) {
			chunk = &layer->tile.chunks[c];
			offset = chunk->x / MAP_CHUNK_WIDTH + chunk->y / MAP_CHUNK_HEIGHT * map->cw;
			buffer_update(map->chunks_sbo, sizeof(struct MapChunkData) , &chunk->data, offset*sizeof(struct MapChunkData));
		}
	}

	map->pipeln = (struct Pipeline){
		.vshader     = create_shader(SHADER_PATH "/map.vert"),
		.fshader     = create_shader(SHADER_PATH "/map.frag"),
		.topology    = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.vert_bindc  = 1,
		.vert_binds  = vert_binds,
		.vert_attrc  = 3,
		.vert_attrs  = vert_attrs,
		.push_stages = VK_SHADER_STAGE_VERTEX_BIT,
		.push_sz     = sizeof(Vec2i),
		.dset_cap    = 1,
		.uboc        = 3,
		.sboc        = 3,
		.imgc        = 2,
	};
	pipeln_alloc_dsets(&map->pipeln);
	pipeln_create_dset(&map->pipeln, 3, (UBO[]){ cam_ubo, global_light_ubo, map->ubo },
	                                 3, (SBO[]){ map->chunks_sbo, map->spot_lights_sbo, map->point_lights_sbo },
	                                 2, (VkImageView[]){ map->tileset.diffuse.image_view,
	                                                     map->tileset.normal.image_view });
	pipeln_init(&map->pipeln, renderpass);

cleanup:
	if (json)
		free(json);
	if (tokens)
		free(tokens);
	arena_reset(arena);
	DEBUG(2, "[MAP] Loaded \"%s\" (%dx%d of %dx%dpx) (%ld tokens):",
			path, map->w, map->h, map->tw, map->th, tokenc);
	DEBUG(2, "        Tileset \"%s\"", map->tileset.name);
	DEBUG(2, "        %d layers:", map->layerc);
	for (int i = 0; i < map->layerc; i++)
		DEBUG(2, "        - %d (%d chunks/%d objects)", i, map->layers[i].tile.chunkc, map->layers[i].obj.objc);
}

noreturn MapTile map_get_tile(struct Map* map, Vec3i pos)
{
	(void)map;
	(void)pos;
	assert(false);
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
		if (layer->type != MAP_LAYER_TILE && layer->type != MAP_LAYER_IMAGE)
			continue;
		for (int j = 0; j < layer->tile.chunkc; j++) {
			chunk = &layer->tile.chunks[j];

			vkCmdBindIndexBuffer(cmd_buf, chunk->ibo.buf, 0, VK_INDEX_TYPE_UINT16);
			vkCmdPushConstants(cmd_buf, map->pipeln.layout, map->pipeln.push_stages, 0, map->pipeln.push_sz,
			                   &VEC2I(chunk->x / MAP_CHUNK_WIDTH, chunk->y / MAP_CHUNK_HEIGHT));
			vkCmdDrawIndexed(cmd_buf, chunk->tilec*18, 1, 0, 0, 0);
		}
	}
}

void map_free(struct Map* map)
{
	struct MapLayer* layer;
	for (int i = 0; i < map->layerc; i++) {
		layer = &map->layers[i];
		if (layer->type == MAP_LAYER_TILE) {
			for (int j = 0; j < layer->tile.chunkc; j++)
				ibo_free(&layer->tile.chunks[j].ibo);
			free(layer->tile.chunks);
		} else if (layer->type == MAP_LAYER_OBJECT) {
			free(layer->obj.objs);
		}
	}
	free(map->layers);
	ubo_free(&map->ubo);
	sbo_free(&map->chunks_sbo);
	sbo_free(&map->spot_lights_sbo);
	sbo_free(&map->point_lights_sbo);
	texture_free(&map->tileset.diffuse);
	texture_free(&map->tileset.normal);
	pipeln_free(&map->pipeln);
	*map = (struct Map){ 0 };
}

void maps_free()
{
	ubo_free(&cam_ubo);
	ubo_free(&lights_ubo);
	vbo_free(&lattice_vbo);
	arena_free(arena);
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
				map->tw = atoi(buf);
				if (map->tw != 128)
					ERROR("[MAP] Tile width should be 128px, got: %d", map->tw);
			} else if (!strcmp(buf, "tileheight")) {
				NEXT();
				map->th = atoi(buf);
				if (map->th != 64)
					ERROR("[MAP] Tile height should be 64px, got: %d", map->th);
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
	String compression = { 0 };
	struct VArray chunks;
	struct VArray objects;

	int len = token.size;
	NEXT();
	for (int j = 0; j < len; j++) {
		if (!strcmp(buf, "chunks")) {
			chunks = varray_new(8, sizeof(struct MapChunk));
			struct MapChunk chunk;
			NEXT();
			int chunkc = token.size;
			NEXT();
			for (int c = 0; c < chunkc; c++) {
				chunk = (struct MapChunk){ 0 };
				i = parse_chunk(&chunk, i, json, tokens);
				varray_push(&chunks, &chunk);
			}
			continue;
		} else if (!strcmp(buf, "objects")) {
			objects = varray_new(32, sizeof(struct MapObject));
			struct MapObject obj;
			NEXT();
			int objc = token.size;
			if (token.type == JSMN_ARRAY)
				NEXT();
			for (int o = 0; o < objc; o++) {
				obj = (struct MapObject){ 0 };
				i = parse_object(&obj, i, json, tokens);
				varray_push(&objects, &obj);
			}
			continue;
		} else if (!strcmp(buf, "properties")) {

		} else if (!strcmp(buf, "image")) {
			NEXT();
		}
		else if (!strcmp(buf, "type"))        { NEXT(); layer->type = LAYER_TYPE_OF_STRING(buf) ; }
		else if (!strcmp(buf, "name"))        { NEXT(); strncpy(layer->name, buf, MAP_NAME_LEN) ; }
		else if (!strcmp(buf, "compression")) { NEXT(); compression = string_new(buf, -1, arena); }
		else if (!strcmp(buf, "width"))       { NEXT(); layer->w    = atoi(buf); }
		else if (!strcmp(buf, "height"))      { NEXT(); layer->h    = atoi(buf); }
		else if (!strcmp(buf, "parallaxx"))   { NEXT(); layer->px   = atof(buf); }
		else if (!strcmp(buf, "parallaxy"))   { NEXT(); layer->py   = atof(buf); }
		else if (!strcmp(buf, "startx"))      { NEXT(); layer->x    = atoi(buf); }
		else if (!strcmp(buf, "starty"))      { NEXT(); layer->y    = atoi(buf); }
		else if (!strcmp(buf, "id"))        NEXT();
		else if (!strcmp(buf, "opacity"))   NEXT();
		else if (!strcmp(buf, "visible"))   NEXT();
		else if (!strcmp(buf, "x"))         NEXT();
		else if (!strcmp(buf, "y"))         NEXT();
		else if (!strcmp(buf, "draworder")) NEXT();
		else {
			ERROR("[MAP] Unmatched layer token : \"%s\"", buf);
			NEXT();
			ERROR("[MAP] \"%s\"", buf);
		}
		NEXT();
	}

	if (compression.len > 0)
		ERROR("!! Compression: %s", compression.data);

	switch (layer->type) {
	case MAP_LAYER_TILE:
		if (!chunks.data) {
			ERROR("[MAP] Map layer is a tile layer but does not appear to have any chunks");
		} else {
			layer->tile.chunkc = chunks.len;
			layer->tile.chunks = smalloc(chunks.len*sizeof(struct MapChunk));
			memcpy(layer->tile.chunks, chunks.data, chunks.len*sizeof(struct MapChunk));
			varray_free(&chunks);
		}
		break;
	case MAP_LAYER_OBJECT:
		if (!objects.data) {
			ERROR("[MAP] Map layer is an object layer but does not appear to have any objects");
		} else {
			layer->obj.objc = objects.len;
			layer->obj.objs = smalloc(objects.len*sizeof(struct MapObject));
			memcpy(layer->obj.objs, objects.data, objects.len*sizeof(struct MapObject));
			varray_free(&objects);
		}
		break;
	default:
		ERROR("[MAP] Unhandled layer of type %d", layer->type);
	}

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
				ERROR("[MAP] Map chunks should be <= 256 tiles");
			i = parse_data(chunk->data.tiles, i, json, tokens);
		}
		else if (!strcmp(buf, "width"))  { NEXT(); }
		else if (!strcmp(buf, "height")) { NEXT(); }
		else if (!strcmp(buf, "x"))      { NEXT(); chunk->x = atoi(buf); }
		else if (!strcmp(buf, "y"))      { NEXT(); chunk->y = atoi(buf); }
		else ERROR("[MAP] Unmatched chunk token: \"%s\"", buf);
		NEXT();
	}

	return i;
}

static int parse_data(uint32* data, int i, char* json, jsmntok_t* tokens)
{
	jsmntok_t token;

	NEXT();
	int len = token.size;
	for (int j = 0; j < len; j++) {
		NEXT();
		data[j] = atoi(buf);
	}

	return i;
}

static int parse_object(struct MapObject* obj, int i, char* json, jsmntok_t* tokens)
{
	jsmntok_t token;
	struct VArray props = varray_new(8, sizeof(struct Property));

	// IDK, size fix
	i--;
	NEXT();

	int len = token.size;
	NEXT();
	obj->type = MAP_OBJECT_NONE;
	for (int j = 0; j < len; j++) {
		if (!strcmp(buf, "properties")) {
			struct Property prop;
			NEXT();
			int propc = token.size;
			NEXT();
			for (int p = 0; p < propc; p++) {
				prop = (struct Property){ 0 };
				i = parse_property(&prop, i, json, tokens);
				varray_push(&props, &prop);
			}
			continue;
		} else if (!strcmp(buf, "point")) {
			NEXT();
			if (!strcmp(buf, "true"))
				obj->type = MAP_OBJECT_POINT;
		} else if (!strcmp(buf, "ellipse")) {
			NEXT();
			if (!strcmp(buf, "true"))
				obj->type = MAP_OBJECT_ELLIPSE;
		}
		else if (!strcmp(buf, "x"))        { NEXT(); obj->x = atof(buf); }
		else if (!strcmp(buf, "y"))        { NEXT(); obj->y = atof(buf); }
		else if (!strcmp(buf, "width"))    { NEXT(); obj->w = atof(buf); }
		else if (!strcmp(buf, "height"))   { NEXT(); obj->h = atof(buf); }
		else if (!strcmp(buf, "rotation")) { NEXT(); obj->rotation = atof(buf); }
		else if (!strcmp(buf, "id"))      NEXT();
		else if (!strcmp(buf, "name"))    NEXT();
		else if (!strcmp(buf, "type"))    NEXT();
		else if (!strcmp(buf, "visible")) NEXT();
		else {
			ERROR("[MAP] Unmatched token: \"%s\"", buf);
			NEXT();
		}
		NEXT();
	}

	if (obj->type == MAP_OBJECT_NONE)
		ERROR("[MAP] Map object of type %d not mapped to enum", obj->type);

	switch (obj->type) {
	case MAP_OBJECT_ELLIPSE:
	case MAP_OBJECT_POINT:
		memcpy(obj->ambient , DEFAULT_LIGHT_AMBIENT , sizeof(obj->ambient));
		memcpy(obj->diffuse , DEFAULT_LIGHT_DIFFUSE , sizeof(obj->diffuse));
		memcpy(obj->specular, DEFAULT_LIGHT_SPECULAR, sizeof(obj->specular));
		obj->constant     = DEFAULT_LIGHT_CONSTANT;
		obj->linear       = DEFAULT_LIGHT_LINEAR;
		obj->quadratic    = DEFAULT_LIGHT_QUADRATIC;
		obj->cutoff       = DEFAULT_LIGHT_CUTOFF;
		obj->outer_cutoff = DEFAULT_LIGHT_OUTER_CUTOFF;
		break;
		break;
	default:
		WARNING("[MAP] No defaults set for objects of type %d", obj->type);
	}
	struct Property* prop;
	for (int j = 0; j < props.len; j++) {
		prop = varray_get(&props, j);
		if (!strcmp(prop->key.data, "ambient")) {
			obj->ambient[3] = prop->value.vcolour[0];
			obj->ambient[0] = prop->value.vcolour[1];
			obj->ambient[1] = prop->value.vcolour[2];
			obj->ambient[2] = prop->value.vcolour[3];
		} else if (!strcmp(prop->key.data, "diffuse")) {
			obj->diffuse[3] = prop->value.vcolour[0];
			obj->diffuse[0] = prop->value.vcolour[1];
			obj->diffuse[1] = prop->value.vcolour[2];
			obj->diffuse[2] = prop->value.vcolour[3];
		} else if (!strcmp(prop->key.data, "specular")) {
			obj->specular[3] = prop->value.vcolour[0];
			obj->specular[0] = prop->value.vcolour[1];
			obj->specular[1] = prop->value.vcolour[2];
			obj->specular[2] = prop->value.vcolour[3];
		}
		else if (!strcmp(prop->key.data, "constant"))     { obj->constant     = prop->value.vfloat; }
		else if (!strcmp(prop->key.data, "linear"))       { obj->linear       = prop->value.vfloat; }
		else if (!strcmp(prop->key.data, "quadratic"))    { obj->quadratic    = prop->value.vfloat; }
		else if (!strcmp(prop->key.data, "cutoff"))       { obj->cutoff       = prop->value.vfloat; }
		else if (!strcmp(prop->key.data, "outer-cutoff")) { obj->outer_cutoff = prop->value.vfloat; }
		else WARNING("[MAP] Ignoring object property \"%s\"", prop->key.data);
	}

	varray_free(&props);
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
	char path[PATH_BUFFER_SIZE] = MAP_PATH;
	jsmntok_t tset_tokens[256];
	for (int j = 0; j < len; j++) {
		if (!strcmp(buf, "source")) {
			NEXT();
			if (snprintf(path, PATH_BUFFER_SIZE, MAP_PATH "/%s", buf) >= PATH_BUFFER_SIZE)
				ERROR("[MAP] Path \"%s\" is greater than PATH_BUFFER_SIZE (%d)", buf, PATH_BUFFER_SIZE);
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
						if (snprintf(path, PATH_BUFFER_SIZE, MAP_PATH "/%s", buf) >= PATH_BUFFER_SIZE)
							ERROR("[MAP] Path \"%s\" is greater than PATH_BUFFER_SIZE (%d)", buf, PATH_BUFFER_SIZE);

						strcpy(tset->image, path);
						tset->diffuse = texture_new_from_image(path);

						strcpy(path + strlen(path) - sizeof(".png") + 1, "_normals.png");
						tset->normal = texture_new_from_image(path);
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

static int parse_property(struct Property* prop, int i, char* json, jsmntok_t* tokens)
{
	jsmntok_t token;
	String    value;

	NEXT();
	for (int j = 0; j < 3; j++) {
		if      (!strcmp(buf, "type"))  { NEXT(); prop->type = PROPERTY_TYPE_OF_STRING(buf); }
		else if (!strcmp(buf, "name"))  { NEXT(); prop->key  = string_new(buf, -1, arena)  ; }
		else if (!strcmp(buf, "value")) { NEXT(); value = string_new(buf, -1, arena)       ; }
		else ERROR("[MAP] Unhandled token: \"%s\"", buf);
		NEXT();
	}

	// DEBUG_VALUE(prop->type);
	uint colour;
	switch (prop->type) {
	case PROPERTY_INT   : prop->value.vint    = atoi(value.data)        ; break;
	case PROPERTY_FLOAT : prop->value.vfloat  = atof(value.data)        ; break;
	case PROPERTY_STRING: prop->value.vstring = string_copy(value, NULL); break;
	case PROPERTY_COLOUR:
		/* +1 to skip the `#` */
		colour = strtoul(value.data + 1, NULL, 16);
		prop->value.vcolour[3] = (colour >> 0 ) & 0xFF;
		prop->value.vcolour[2] = (colour >> 8 ) & 0xFF;
		prop->value.vcolour[1] = (colour >> 16) & 0xFF;
		prop->value.vcolour[0] = (colour >> 24) & 0xFF;
		break;
	case PROPERTY_BOOL:
		if (!strcmp(value.data, "true"))
			prop->value.vbool = true;
		else
			prop->value.vbool = false;
		break;
	default:
		ERROR("[MAP] Unhandled property type: \"%d\" (%s)", prop->type, value.data);
	}

	return i;
}

/* -------------------------------------------------------------------- */

static void build_lights(struct Map* map)
{
	struct VArray point_lights = varray_new(32, sizeof(struct PointLight));
	struct VArray spot_lights  = varray_new(16, sizeof(struct SpotLight));

	struct MapLayer* layer = NULL;
	for (int i = 0; i < map->layerc; i++)
		if (map->layers[i].type == MAP_LAYER_OBJECT && !strcmp(map->layers[i].name, "lights"))
			layer = &map->layers[i];

	struct MapObject* obj;
	struct MapChunk*  chunk;
	struct SpotLight  slight;
	struct PointLight plight;
	int index;
	Vec3 dir;
	float dist;
	float cf = 1.0f / 255.0f;
	if (!layer) {
		WARNING("[MAP] Map does not have a \"lights\" object layer");
	} else {
		for (int o = 0; o < layer->obj.objc; o++) {
			obj = &layer->obj.objs[o];
			Vec3 point = VEC3(2.0f * obj->x / map->tw + 1.0f, obj->y / map->th + 1.0f, -1.0f);
			switch (obj->type) {
			case MAP_OBJECT_POINT:
				plight = (struct PointLight){
					.pos     = point,
					.ambient = (Vec3){
						obj->ambient[0] * cf,
						obj->ambient[1] * cf,
						obj->ambient[2] * cf,
					},
					.diffuse = (Vec3){
						obj->diffuse[0] * cf,
						obj->diffuse[1] * cf,
						obj->diffuse[2] * cf,
					},
					.specular = (Vec3){
						obj->specular[0] * cf,
						obj->specular[1] * cf,
						obj->specular[2] * cf,
					},
					.constant  = obj->constant,
					.linear    = obj->linear,
					.quadratic = obj->quadratic,
				};
				index = varray_push(&point_lights, &plight);

				dist = 1000;
				for (int t = 0; t < map->layerc; t++) {
					if (map->layers[t].type != MAP_LAYER_TILE)
						continue;
					for (int c = 0; c < map->layers[t].tile.chunkc; c++) {
						chunk = &map->layers[t].tile.chunks[c];
						if (distance(plight.pos, VEC3(chunk->x, chunk->y, 0.0f)) <= dist) {
							if (chunk->data.pointc < MAP_POINT_LIGHTS_PER_CHUNK)
								chunk->data.points[chunk->data.pointc++] = index;
							else
							 	WARNING("[MAP] Chunk exceeded maximum point lights");
						}
					}
				}
				break;
			case MAP_OBJECT_ELLIPSE:
				dir = VEC3(0, 0, 1);
				slight = (struct SpotLight){
					.pos     = point,
					.dir     = dir,
					.ambient = (Vec3){
						obj->ambient[0] * cf,
						obj->ambient[1] * cf,
						obj->ambient[2] * cf,
					},
					.diffuse = (Vec3){
						obj->diffuse[0] * cf,
						obj->diffuse[1] * cf,
						obj->diffuse[2] * cf,
					},
					.specular = (Vec3){
						obj->specular[0] * cf,
						obj->specular[1] * cf,
						obj->specular[2] * cf,
					},
					.constant     = obj->constant,
					.linear       = obj->linear,
					.quadratic    = obj->quadratic,
					.cutoff       = obj->cutoff,
					.outer_cutoff = obj->outer_cutoff,
				};
				index = varray_push(&spot_lights, &slight);

				dist = MAX(obj->w, obj->h) + 1;
				for (int t = 0; t < map->layerc; t++) {
					if (map->layers[t].type != MAP_LAYER_TILE)
						continue;
					for (int c = 0; c < map->layers[t].tile.chunkc; c++) {
						chunk = &map->layers[t].tile.chunks[c];
						if (distance(slight.pos, VEC3(chunk->x, chunk->y, 0.0f)) <= dist) {
							if (chunk->data.spotc < MAP_SPOT_LIGHTS_PER_CHUNK)
								chunk->data.spots[chunk->data.spotc++] = index;
							else
							 	WARNING("[MAP] Chunk exceeded maximum spot lights");
						}
					}
				}
				break;
			default:
				ERROR("[MAP] Lighting for object of type %d not implemented", obj->type);
			}
		}
	}

	map->spot_lights_sbo  = sbo_new(MAX(1, sizeof(struct SpotLight[spot_lights.len])));
	map->point_lights_sbo = sbo_new(MAX(1, sizeof(struct SpotLight[point_lights.len])));
	if (spot_lights.len)
		buffer_update(map->spot_lights_sbo, spot_lights.len*sizeof(struct SpotLight), spot_lights.data, 0);
	if (point_lights.len)
		buffer_update(map->point_lights_sbo, point_lights.len*sizeof(struct PointLight), point_lights.data, 0);

	varray_free(&spot_lights);
	varray_free(&point_lights);
}

static void build_mesh(struct Map* map)
{
	uint16* inds = smalloc(MAP_CHUNK_SIZE*sizeof(uint16[18]));
	struct MapLayer* layer;
	struct MapChunk* chunk;
	for (int i = 0; i < map->layerc; i++) {
		layer = &map->layers[i];
		if (layer->type != MAP_LAYER_TILE)
			continue;

		for (int j = 0; j < layer->tile.chunkc; j++) {
			chunk = &layer->tile.chunks[j];
			for (int k = 0; k < MAP_CHUNK_SIZE; k++) {
				if (chunk->data.tiles[k]) {
					mesh_tile(&inds[18*chunk->tilec++], VEC2I(k % MAP_CHUNK_WIDTH,
					                                          k / MAP_CHUNK_WIDTH));
				}
			}
			chunk->ibo = ibo_new(18*chunk->tilec*sizeof(inds[0]), inds);
		}
	}

	free(inds);
}

static void mesh_tile(uint16* inds, Vec2i v)
{
	#define V(vi) (12*tile_index + vi)
	int tile_index = v.y*MAP_CHUNK_WIDTH + v.x;
	/* Top: 0-3 */
	*inds++ = V(0); *inds++ = V(3); *inds++ = V(1);
	*inds++ = V(2); *inds++ = V(1); *inds++ = V(3);

	/* Left: 4-7 */
	*inds++ = V(4); *inds++ = V(7); *inds++ = V(5);
	*inds++ = V(6); *inds++ = V(5); *inds++ = V(7);

	/* Right: 8-11 */
	*inds++ = V(8 ); *inds++ = V(11); *inds++ = V(9 );
	*inds++ = V(10); *inds++ = V(9 ); *inds++ = V(11);
	#undef V
}

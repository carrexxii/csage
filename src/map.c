#include "vulkan/vulkan.h"
#define JSMN_PARENT_LINKS
#include "jsmn/jsmn.h"

#include "util/file.h"
#include "util/varray.h"
#include "maths/maths.h"
#include "gfx/vulkan.h"
#include "gfx/buffers.h"
#include "gfx/pipeline.h"
#include "map.h"

#define VERTEX_ELEMENTS 6
#define SIZEOF_VERTEX   sizeof(int8[VERTEX_ELEMENTS])
#define BUFFER_SIZE     4096
#define MAX_CHUNKS      256

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
static void mesh_tile(int16* inds, Vec3i v);

static VkRenderPass renderpass;
static struct Pipeline pipeln;
static VkVertexInputBindingDescription vert_binds[] = {
	{ .binding   = 0,
	  .stride    = SIZEOF_VERTEX, /* xyznnn */
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, }
};
static VkVertexInputAttributeDescription vert_attrs[] = {
	{ .binding  = 0,
	  .location = 0,
	  .format   = VK_FORMAT_R8G8B8_SINT, /* xyz */
	  .offset   = 0, },
	{ .binding  = 0,
	  .location = 1,
	  .format   = VK_FORMAT_R8G8B8_SINT, /* normal */
	  .offset   = sizeof(int8[3]), },
};

static VBO lattice_vbo;
static char buf[BUFFER_SIZE];

void maps_init(VkRenderPass rpass)
{
	renderpass = rpass;

	/* Generate the vertex lattice -> 3 versions, 1 for each normal */
	int w = MAP_CHUNK_WIDTH  + 1;
	int h = MAP_CHUNK_HEIGHT + 1;
	intptr vert_size = 3*w*h*2;
	int8* verts = scalloc(vert_size, sizeof(int8[VERTEX_ELEMENTS]));
	int i;
	for (int dir = 0; dir < 3; dir++) {
		for (int z = 0; z < 2; z++) {
			for (int y = 0; y < h; y++) {
				for (int x = 0; x < w; x++) {
					i = VERTEX_ELEMENTS*(z*w*h + y*w + x) + VERTEX_ELEMENTS*dir*(w*h*2);
					verts[i + 0] = x;
					verts[i + 1] = y;
					verts[i + 2] = z;
					verts[i + 3 + dir] = 1;
					// DEBUG(1, "[%d] %hhd %hhd %hhd (%hhd %hhd %hhd)", i, verts[i], verts[i+1], verts[i+2], verts[i+3], verts[i+4], verts[i+5]);
				}
			}
		}
	}
	lattice_vbo = vbo_new(vert_size, verts, false);
	free(verts);

	DEBUG(3, "[MAP] Map system initialized");
}

struct Map map_new(const char* name)
{
	struct Map map = { 0 };

	char path[128];
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

	parse_map(&map, json, tokens, tokenc);
	build_mesh(&map);

cleanup:
	if (tokens) {
		free(tokens);
		DEBUG(2, "[MAP] Loaded \"%s\" (%hux%hu) (%ld tokens)", path, map.w, map.h, tokenc);
	}

	// DEBUG(1, "Map: %dx%d w/%d layers and %d tilesets", map.w, map.h, map.layerc, map.tilesetc);
	// struct Tileset  tileset;
	// struct MapLayer layer;
	// struct MapChunk chunk;
	// DEBUG(1, "\tTileset \"%s\" (%dx%d tiles): \"%s\"", tileset.name, tileset.tw, tileset.th, tileset.image);
	// for (int i = 0; i < map.layerc; i++) {
	// 	layer = map.layers[i];
	// 	DEBUG(1, "\tLayer \"%s\" %d (%d, %d -> %dx%d) w/%d chunks",
	// 	      layer.name, i, layer.x, layer.y, layer.w, layer.h, layer.chunkc);
	// 	for (int j = 0; j < layer.chunkc; j++) {
	// 		chunk = layer.chunks[j];
	// 		DEBUG(1, "\t\t Chunk %d (%d, %d)", j, chunk.x, chunk.y);
	// 		fprintf(stderr, "\t\t");
	// 		for (int k = 0; k < MAP_CHUNK_SIZE; k++)
	// 			fprintf(stderr, "%d, ", chunk.data[k]);
	// 		fprintf(stderr, "\n");
	// 	}
	// }

	exit(0);
	return map;
}

void map_free(struct Map* map)
{
	for (int i = 0; i < map->layerc; i++)
		free(map->layers[i].chunks);
	free(map->layers);
	*map = (struct Map){ 0 };
}

void maps_free()
{
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
		else if (!strcmp(buf, "x"))      { NEXT(); chunk->x = atoi(buf); NEXT(); }
		else if (!strcmp(buf, "y"))      { NEXT(); chunk->y = atoi(buf); NEXT(); }
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
	char path[128];
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
						strcpy(tset->image, buf);
					}
					else if (!strcmp(buf, "name"))       { TSET_NEXT(); strcpy(tset->name, buf); }
					else if (!strcmp(buf, "margin"))     { TSET_NEXT(); tset->margin    = atoi(buf); }
					else if (!strcmp(buf, "spacing"))    { TSET_NEXT(); tset->spacing   = atoi(buf); }
					else if (!strcmp(buf, "columns"))    { TSET_NEXT(); tset->columns   = atoi(buf); }
					else if (!strcmp(buf, "tilewidth"))  { TSET_NEXT(); tset->tw        = atoi(buf); }
					else if (!strcmp(buf, "tileheight")) { TSET_NEXT(); tset->th        = atoi(buf); }
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
		} else if (!strcmp(buf, "firstgid")) {
			NEXT();
			NEXT();
		}
	}

	return i;
}

static void build_mesh(struct Map* map)
{
	int16* inds = smalloc(MAP_CHUNK_WIDTH*MAP_CHUNK_HEIGHT*2*sizeof(int16[18]));
	isize indc;
	struct MapLayer* layer;
	struct MapChunk* chunk;
	for (int i = 0; i < map->layerc; i++) {
		layer = &map->layers[i];
		for (int j = 0; j < layer->chunkc; j++) {
			chunk = &layer->chunks[j];
			indc = 0;
			for (int k = 0; k < MAP_CHUNK_SIZE; k++) {
				if (chunk->data[k]) {
					mesh_tile(inds, VEC3I(k % MAP_CHUNK_WIDTH,
					                      k / MAP_CHUNK_WIDTH,
					                      chunk->z));
					inds += 18;
					indc += 18;
					// DEBUG(1, "[%d] %d, %d, %d", k, k % MAP_CHUNK_WIDTH, k / MAP_CHUNK_WIDTH, chunk->z);
				}
			}
			chunk->tilec = indc / 18;
			chunk->ibo = ibo_new(indc*sizeof(inds[0]), inds);
		}
	}
}

/* 0 = x-axis; 1 = y-axis; 2 = z-axis */
#define VERTEX_INDEX(x, y, z, axis) ((z)*(MAP_CHUNK_WIDTH+1) + (y)*(MAP_CHUNK_HEIGHT+1) + (x) + axis*2)
static void mesh_tile(int16* inds, Vec3i v)
{
	/* Left side triangles */
	*inds++ = VERTEX_INDEX(v.x + 1, v.y + 1, v.z    , 0);
	*inds++ = VERTEX_INDEX(v.x + 1, v.y + 1, v.z + 1, 0);
	*inds++ = VERTEX_INDEX(v.x + 1, v.y    , v.z    , 0);
	*inds++ = VERTEX_INDEX(v.x + 1, v.y    , v.z    , 0);
	*inds++ = VERTEX_INDEX(v.x + 1, v.y + 1, v.z + 1, 0);
	*inds++ = VERTEX_INDEX(v.x + 1, v.y    , v.z + 1, 0);
	/* Right side triangles */
	*inds++ = VERTEX_INDEX(v.x    , v.y + 1, v.z    , 1);
	*inds++ = VERTEX_INDEX(v.x    , v.y + 1, v.z + 1, 1);
	*inds++ = VERTEX_INDEX(v.x + 1, v.y + 1, v.z    , 1);
	*inds++ = VERTEX_INDEX(v.x + 1, v.y + 1, v.z    , 1);
	*inds++ = VERTEX_INDEX(v.x    , v.y + 1, v.z + 1, 1);
	*inds++ = VERTEX_INDEX(v.x + 1, v.y + 1, v.z + 1, 1);
	/* Top triangles */
	*inds++ = VERTEX_INDEX(v.x    , v.y    , v.z, 2);
	*inds++ = VERTEX_INDEX(v.x    , v.y + 1, v.z, 2);
	*inds++ = VERTEX_INDEX(v.x + 1, v.y    , v.z, 2);
	*inds++ = VERTEX_INDEX(v.x + 1, v.y    , v.z, 2);
	*inds++ = VERTEX_INDEX(v.x    , v.y + 1, v.z, 2);
	*inds++ = VERTEX_INDEX(v.x + 1, v.y + 1, v.z, 2);
}

#include "common.h"
#include "lang/lexer.h"
#include <stdint.h>
#define JSMN_PARENT_LINKS
#include "jsmn/jsmn.h"

#include "util/file.h"
#include "util/varray.h"
#include "map.h"

#define BUFFER_SIZE 4096
#define MAX_CHUNKS 256

#define STRING_OF_JSMN_TYPE(x) (           \
	x == JSMN_UNDEFINED? "JSMN_UNDEFINED": \
	x == JSMN_OBJECT   ? "JSMN_OBJECT"   : \
	x == JSMN_ARRAY    ? "JSMN_ARRAY"    : \
	x == JSMN_STRING   ? "JSMN_STRING"   : \
	x == JSMN_PRIMITIVE? "JSMN_PRIMITIVE": \
	"<Not a JSMN type>")

static void parse_map(struct Map* map, char* json, jsmntok_t* tokens, isize tokenc);
static int parse_layer(struct MapLayer* layer, int i, struct Map* map, char* json, jsmntok_t* tokens);
static int parse_chunk(struct MapChunk* chunk, int i, struct Map* map, char* json, jsmntok_t* tokens);
static int parse_data(uint8* data, int i, char* json, jsmntok_t* tokens);

static char buf[4096];

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
		ERROR("[MAP] JSON file appears to be incomplete (JSMN_ERROR_PART) for \"%s\"", path);
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

cleanup:
	if (tokens) {
		free(tokens);
		DEBUG(2, "[MAP] Loaded \"%s\" (%hux%hu) (%ld tokens)", path, map.w, map.h, tokenc);
	}

	DEBUG(1, "Map: %dx%d w/%d layers", map.w, map.h, map.layerc);
	struct MapLayer layer;
	struct MapChunk chunk;
	for (int i = 0; i < map.layerc; i++) {
		layer = map.layers[i];
		DEBUG(1, "\tLayer %d (%d, %d -> %dx%d) w/%d chunks", i, layer.x, layer.y, layer.w, layer.h, layer.chunkc);
		for (int j = 0; j < layer.chunkc; j++) {
			chunk = layer.chunks[j];
			DEBUG(1, "\t\t Chunk %d (%d, %d -> %dx%d)", j, chunk.x, chunk.y, chunk.w, chunk.h);
			fprintf(stderr, "\t\t");
			for (int k = 0; k < 256; k++) {
				fprintf(stderr, "%d, ", chunk.data[k]);
			}
			fprintf(stderr, "\n");
		}
	}

	exit(0);
	return map;
}

/* -------------------------------------------------------------------- */

#define NEXT() do {                                              \
		token = tokens[i++];                                      \
		start = json + token.start;                                \
		if (token.end - token.start < BUFFER_SIZE)                  \
			snprintf(buf, token.end - token.start + 1, "%s", start); \
	} while (0)

static void parse_map(struct Map* map, char* json, jsmntok_t* tokens, isize tokenc)
{
	struct VArray layers = varray_new(8 , sizeof(struct MapLayer));

	jsmntok_t token;
	isize size;
	char* start;
	for (int i = 1; i < tokenc;) {
		NEXT();
		switch (token.type) {
		case JSMN_STRING:
			if (!strcmp(buf, "orientation")) {
				NEXT();
				if (!strcmp(start, "isometric")) {
					ERROR("[MAP] Map has type \"%s\", should be \"isometric\"", buf);
					return;
				}
			} else if (!strcmp(start, "tilewidth")) {
				NEXT();
				int tw = atoi(buf);
				if (tw != 128)
					ERROR("[MAP] Tile width should be 128px, got: %d", tw);
			} else if (!strcmp(start, "tileheight")) {
				NEXT();
				int th = atoi(buf);
				if (th != 64)
					ERROR("[MAP] Tile height should be 64px, got: %d", th);
			} else if (!strcmp(buf, "compressionlevel")) {
				NEXT();
				DEBUG(1, "[MAP] TODO: compressionlevel (%s)", buf);
			} else if (!strcmp(buf, "width")) {
				NEXT();
				map->w = atoi(buf);
			} else if (!strcmp(buf, "height")) {
				NEXT();
				map->h = atoi(buf);
			} else if (!strcmp(buf, "chunk")) {
				if (map->layerc <= 0)
					ERROR("[MAP] Found chunk before layer");
				NEXT();
			}
			break;
		case JSMN_ARRAY:
			if (!strcmp(buf, "layers")) {
				struct MapLayer layer;
				int layerc = token.size;
				NEXT();
				for (int l = 0; l < layerc; l++) {
					layer = (struct MapLayer){ 0 };
					i = parse_layer(&layer, i, map, json, tokens);
					varray_push(&layers, &layer);
				}
			}
			break;
		default:
			ERROR("[MAP] Unmatched token \"%s\"", buf);
		}
	}

	map->layerc = layers.len;
	map->layers = smalloc(map->layerc*sizeof(struct MapLayer));
	memcpy(map->layers, layers.data, map->layerc*sizeof(struct MapLayer));
	varray_free(&layers);
}

static int parse_layer(struct MapLayer* layer, int i, struct Map* map, char* json, jsmntok_t* tokens)
{
	jsmntok_t token = tokens[i - 1];
	char* start;
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
				i = parse_chunk(&chunk, i, map, json, tokens);
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
	layer->chunks = smalloc(layer->chunkc*sizeof(struct MapChunk));
	memcpy(layer->chunks, chunks.data, layer->chunkc*sizeof(struct MapChunk));
	varray_free(&chunks);
	return i;
}

static int parse_chunk(struct MapChunk* chunk, int i, struct Map* map, char* json, jsmntok_t* tokens)
{
	jsmntok_t token = tokens[i - 1];
	char* start;

	int len = token.size;
	NEXT();
	for (int j = 0; j < len; j++) {
		if (!strcmp(buf, "data")) {
			if (token.size > UINT8_MAX + 1)
				ERROR("[MAP] Map chunks need to be <= 256 tiles");
			i = parse_data(chunk->data, i, json, tokens);
		}
		else if (!strcmp(buf, "width"))  { NEXT(); chunk->w = atoi(buf); NEXT(); }
		else if (!strcmp(buf, "height")) { NEXT(); chunk->h = atoi(buf); NEXT(); }
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
	char* start;

	NEXT();
	int len = token.size;
	for (int j = 0; j < len; j++) {
		NEXT();
		data[j] = atoi(buf);
	}

	NEXT();
	return i;
}

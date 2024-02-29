#include "lang/lexer.h"
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
static int parse_layer(struct MapLayer* layer, int i, char* json, jsmntok_t* tokens);
static int parse_chunk(struct MapChunk* chunk, int i, char* json, jsmntok_t* tokens);
static int parse_data(uint8* data, int i, char* json, jsmntok_t* tokens);
static int parse_tilesets(struct Tileset* tset, int i, char* json, jsmntok_t* tokens);

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

cleanup:
	if (tokens) {
		free(tokens);
		DEBUG(2, "[MAP] Loaded \"%s\" (%hux%hu) (%ld tokens)", path, map.w, map.h, tokenc);
	}

	DEBUG(1, "Map: %dx%d w/%d layers and %d tilesets", map.w, map.h, map.layerc, map.tilesetc);
	struct Tileset  tileset;
	struct MapLayer layer;
	struct MapChunk chunk;
	for (int i = 0; i < map.tilesetc; i++) {
		tileset = map.tilesets[i];
		DEBUG(1, "\tTileset \"%s\" %d (%dx%d tiles): \"%s\"",
		      tileset.name, i, tileset.tw, tileset.th, tileset.image);
	}
	for (int i = 0; i < map.layerc; i++) {
		layer = map.layers[i];
		DEBUG(1, "\tLayer \"%s\" %d (%d, %d -> %dx%d) w/%d chunks",
		      layer.name, i, layer.x, layer.y, layer.w, layer.h, layer.chunkc);
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

#define NEXT() do {                                                            \
		token = tokens[i++];                                                    \
		if (token.end - token.start < BUFFER_SIZE)                               \
			snprintf(buf, token.end - token.start + 1, "%s", json + token.start); \
	} while (0)

static void parse_map(struct Map* map, char* json, jsmntok_t* tokens, isize tokenc)
{
	struct VArray layers   = varray_new(8, sizeof(struct MapLayer));
	struct VArray tilesets = varray_new(4, sizeof(struct Tileset));

	jsmntok_t token = tokens[0];
	isize len = token.size;
	int i = 0;
	NEXT();
	NEXT();
	for (int j = 0; j < len; j++) {
		// DEBUG_VALUE(buf);
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
				struct Tileset tset;
				NEXT();
				int tsetc = token.size;
				NEXT();
				for (int ts = 0; ts < tsetc; ts++) {
					tset = (struct Tileset){ 0 };
					i = parse_tilesets(&tset, i, json, tokens);
					varray_push(&tilesets, &tset);
				}
			} else if (!strcmp(buf, "layers")) {
				struct MapLayer layer;
				int layerc = tokens[i].size;
				NEXT();
				NEXT();
				for (int l = 0; l < layerc; l++) {
					layer = (struct MapLayer){ 0 };
					i = parse_layer(&layer, i, json, tokens);
					varray_push(&layers, &layer);
				}
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

	map->tilesetc = tilesets.len;
	map->tilesets = smalloc(map->tilesetc*sizeof(struct Tileset));
	memcpy(map->tilesets, tilesets.data, map->tilesetc*sizeof(struct Tileset));
	varray_free(&tilesets);

	map->layerc = layers.len;
	map->layers = smalloc(map->layerc*sizeof(struct MapLayer));
	memcpy(map->layers, layers.data, map->layerc*sizeof(struct MapLayer));
	varray_free(&layers);
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
	layer->chunks = smalloc(layer->chunkc*sizeof(struct MapChunk));
	memcpy(layer->chunks, chunks.data, layer->chunkc*sizeof(struct MapChunk));
	varray_free(&chunks);
	return i;
}

static int parse_chunk(struct MapChunk* chunk, int i, char* json, jsmntok_t* tokens)
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
static int parse_tilesets(struct Tileset* tset, int i, char* json, jsmntok_t* tokens)
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
					else if (!strcmp(buf, "firstgid"))   { TSET_NEXT(); tset->first_gid = atoi(buf); }
					else if (!strcmp(buf, "margin"))     { TSET_NEXT(); tset->margin    = atoi(buf); }
					else if (!strcmp(buf, "spacing"))    { TSET_NEXT(); tset->spacing   = atoi(buf); }
					else if (!strcmp(buf, "columns"))    { TSET_NEXT(); tset->columns   = atoi(buf); }
					else if (!strcmp(buf, "tilewidth"))  { TSET_NEXT(); tset->tw        = atoi(buf); }
					else if (!strcmp(buf, "tileheight")) { TSET_NEXT(); tset->th        = atoi(buf); }
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
			tset->first_gid = atoi(buf);
			NEXT();
		}
	}

	return i;
}

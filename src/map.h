#ifndef MAP_H
#define MAP_H

#include "util/string.h"
#include "gfx/vulkan.h"
#include "gfx/buffers.h"
#include "gfx/pipeline.h"
#include "gfx/texture.h"
#include "gfx/renderer.h"
#include "maths/types.h"
#include "camera.h"

#define MAP_CHUNK_WIDTH            16
#define MAP_CHUNK_HEIGHT           16
#define MAP_CHUNK_SIZE             (MAP_CHUNK_WIDTH*MAP_CHUNK_HEIGHT)
#define MAP_POINT_LIGHTS_PER_CHUNK 8
#define MAP_SPOT_LIGHTS_PER_CHUNK  4
#define MAP_MAX_SPOT_LIGHTS        32
#define MAP_MAX_POINT_LIGHTS       128
#define MAP_NAME_LEN               32

typedef uint32 MapTile;

struct Tileset {
	struct Texture diffuse;
	struct Texture normal;
	char image[64];
	char name[32];
	int margin;
	int spacing;
	int columns;
	int tw, th;
};

struct MapChunkData {
	int spotc, pointc;
	int spots[MAP_SPOT_LIGHTS_PER_CHUNK];
	int points[MAP_POINT_LIGHTS_PER_CHUNK];
	MapTile tiles[MAP_CHUNK_SIZE];
};

struct MapChunk {
	IBO ibo;
	int x, y;
	int tilec;
	struct MapChunkData data;
};
struct MapTileLayer {
	int chunkc;
	struct MapChunk* chunks;
};

enum MapObjectType {
	MAP_OBJECT_NONE,
	MAP_OBJECT_POINT,
	MAP_OBJECT_RECT,
	MAP_OBJECT_ELLIPSE,
	MAP_OBJECT_POLYGON,
	MAP_OBJECT_POLYLINE,
	MAP_OBJECT_TEXT,
	MAP_OBJECT_TILE,
};
struct MapObject {
	enum MapObjectType type;
	float x, y, w, h;
	float rotation;
	union {
		Vec2*  points;
		String text;
		struct {
			uint8 ambient[4];
			uint8 diffuse[4];
			uint8 specular[4];
			float constant;
			float linear;
			float quadratic;
			float cutoff;
			float outer_cutoff;
		};
	};
};
struct MapObjectLayer {
	int w, h;
	int objc;
	struct MapObject* objs;
};

struct MapImageLayer {
	char img[MAP_NAME_LEN];
};

#define LAYER_TYPE_OF_STRING(x) (                  \
	!strcmp((x), "tilelayer")  ? MAP_LAYER_TILE  : \
	!strcmp((x), "objectgroup")? MAP_LAYER_OBJECT: \
	!strcmp((x), "imagelayer") ? MAP_LAYER_IMAGE : \
	!strcmp((x), "group")      ? MAP_LAYER_GROUP : \
	MAP_LAYER_NONE)
enum MapLayerType {
	MAP_LAYER_NONE,
	MAP_LAYER_TILE,
	MAP_LAYER_OBJECT,
	MAP_LAYER_IMAGE,
	MAP_LAYER_GROUP,
};
struct MapLayer {
	enum MapLayerType type;
	char name[MAP_NAME_LEN];
	int x, y, w, h;
	int px, py;
	union {
		struct MapTileLayer   tile;
		struct MapObjectLayer obj;
		struct MapImageLayer  img;
	};
};

struct Map {
	SBO chunks_sbo;
	SBO spot_lights_sbo;
	SBO point_lights_sbo;
	UBO ubo;
	int w, h, tw, th, cw, ch;
	int tilesetc;
	int layerc;
	struct Tileset   tileset;
	struct Pipeline  pipeln;
	struct MapLayer* layers;
};

void    maps_init(void);
void    map_new(struct Map* map, const char* name);
MapTile map_get_tile(struct Map* map, Vec3i pos);
void    map_record_commands(VkCommandBuffer cmd_buf, struct Camera* cam, struct Map* map);
void    map_free(struct Map* map);
void    maps_free(void);

#endif

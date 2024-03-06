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

#define MAP_CHUNK_WIDTH      16
#define MAP_CHUNK_HEIGHT     16
#define MAP_CHUNK_DEPTH      1
#define MAP_CHUNK_SIZE       (MAP_CHUNK_WIDTH*MAP_CHUNK_HEIGHT*MAP_CHUNK_DEPTH)
#define MAP_LIGHTS_PER_CHUNK 8
#define MAP_NAME_LEN         32

typedef int MapTile;

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

struct MapChunk {
	IBO ibo;
	int tilec;
	Vec3i pos;
	uint32 data[MAP_CHUNK_SIZE];
	struct Light lights[MAP_LIGHTS_PER_CHUNK];
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
	int x, y, w, h;
	float rotation;
	Vec2*  points;
	String text;
};
struct MapObjectLayer {
	int w, h;
	uint8 rgb[3];
	int objc;
	struct MapObject* objs;
};

struct MapImageLayer {
	char img[MAP_NAME_LEN];
};

#define LAYER_TYPE_OF_STRING(x) (                    \
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
	UBO ubo;
	int w, h, tw, th;
	int tilesetc;
	int layerc;
	struct Tileset   tileset;
	struct Pipeline  pipeln;
	struct MapLayer* layers;
};

struct MapData {
	uint32 block_data[MAP_CHUNK_SIZE];
};

void    maps_init(VkRenderPass renderpass);
void    map_new(struct Map* map, const char* name);
MapTile map_get_tile(struct Map* map, Vec3i pos);
void    map_record_commands(VkCommandBuffer cmd_buf, struct Camera* cam, struct Map* map);
void    map_free(struct Map* map);
void    maps_free(void);

#endif

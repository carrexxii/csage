#ifndef MAP_H
#define MAP_H

#include "gfx/vulkan.h"
#include "gfx/buffers.h"
#include "gfx/pipeline.h"

#define MAP_CHUNK_WIDTH  16
#define MAP_CHUNK_HEIGHT 16
#define MAP_CHUNK_SIZE   (MAP_CHUNK_WIDTH*MAP_CHUNK_HEIGHT)

typedef int MapTile;

struct Tileset {
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
	uint16 x, y, z;
	uint8 data[MAP_CHUNK_SIZE];
};

struct MapLayer {
	char name[32];
	int x, y, w, h;
	int chunkc;
	struct MapChunk* chunks;
};

struct Map {
	int w, h, tw, th;
	int tilesetc;
	int layerc;
	struct Tileset   tileset;
	struct Pipeline  pipeln;
	struct MapLayer* layers;
};

void maps_init(VkRenderPass renderpass);
struct Map map_new(const char* name);
void map_free(struct Map* map);
void maps_free(void);

#endif

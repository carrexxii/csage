#ifndef MAP_H
#define MAP_H

#include "gfx/vulkan.h"
#include "gfx/buffers.h"
#include "gfx/pipeline.h"
#include "gfx/texture.h"
#include "maths/types.h"
#include "camera.h"

#define MAP_CHUNK_WIDTH  16
#define MAP_CHUNK_HEIGHT 16
#define MAP_CHUNK_DEPTH  1
#define MAP_CHUNK_SIZE   (MAP_CHUNK_WIDTH*MAP_CHUNK_HEIGHT*MAP_CHUNK_DEPTH)

typedef int MapTile;

struct Tileset {
	struct Texture texture;
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
	uint8 data[MAP_CHUNK_SIZE];
};

struct MapLayer {
	char name[32];
	int x, y, w, h;
	int chunkc;
	struct MapChunk* chunks;
};

struct MapData {
	uint8 block_data[MAP_CHUNK_SIZE];
};

struct Map {
	UBO ubo;
	int w, h, tw, th;
	int tilesetc;
	int layerc;
	struct Tileset   tileset;
	struct Pipeline  pipeln;
	struct MapLayer* layers;
	struct MapData data;
};

void maps_init(VkRenderPass renderpass);
void map_new(struct Map* map, const char* name);
void map_record_commands(VkCommandBuffer cmd_buf, struct Camera* cam, struct Map* map);
void map_free(struct Map* map);
void maps_free(void);

#endif

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
#define MAP_CHUNK_SIZE   (MAP_CHUNK_WIDTH*MAP_CHUNK_HEIGHT)

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
	Mat4x4 proj;
	Mat4x4 view;
	Vec3i  block_size;
	float pad1;
};

struct Map {
	struct MapData data;
	UBO ubo;
	int w, h, tw, th;
	int tilesetc;
	int layerc;
	struct Tileset   tileset;
	struct Pipeline  pipeln;
	struct MapLayer* layers;
};

void maps_init(VkRenderPass renderpass);
void map_new(struct Map* map, const char* name);
void map_record_commands(struct Map* map, struct Camera* cam, VkCommandBuffer cmd_buf);
void map_free(struct Map* map);
void maps_free(void);

#endif

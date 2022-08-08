#ifndef MAP_MAP_H
#define MAP_MAP_H

#include "gfx/buffers.h"
#include "gfx/model.h"

#define MAP_BLOCK_WIDTH  16
#define MAP_BLOCK_HEIGHT 16
#define MAP_BLOCK_DEPTH  12
#define MAP_CELLS_PER_BLOCK (MAP_BLOCK_WIDTH*MAP_BLOCK_HEIGHT*MAP_BLOCK_DEPTH)
#define MAP_INDICES_PER_VXL 9
#define MAP_VERTEX_COUNT ((MAP_BLOCK_WIDTH + 1)*(MAP_BLOCK_HEIGHT + 1)*(MAP_BLOCK_DEPTH + 1))

enum MapType {
	MAPTYPE_NONE,
	MAPTYPE_FILLED,
	MAPTYPE_RANDOM,
};

enum CellType {
	CELLTYPE_EMPTY,
	CELLTYPE_GRASS,
	CELLTYPE_DIRT,
};

enum CellShape {
	CELLSHAPE_FULL,
};

struct MapCell {
	uint8 data;
	uint8 shape;
}; static_assert(sizeof(struct MapCell) == 2, "struct MapCell");

struct MapBlock {
	IBO    ibo;
	uint32 indc;
	int16  zlvl;
	bool   visible;
}; static_assert(sizeof(struct MapBlock) == 32, "struct MapBlock");

struct Map {
	struct Dim dim;
	uint32 indc;
	VBO    verts;
	struct MapBlock* inds;
	struct MapCell data[];
}; static_assert(sizeof(struct Map) == 48, "struct Map");

struct MapDrawData {
	struct Dim4 dim;
	struct Dim4 stride;
	// struct Material materials[UINT8_MAX];
};

void init_map(enum MapType type, struct Dim dim);
bool is_block_visible(uint block);
void free_map();
void print_map();

void generate_meshes();

extern struct Map* map;
extern struct MapDrawData mapdd;
extern uintptr mapcellc;
extern uintptr mapblockc;

inline static uint32 get_block_x(uint32 start) {
	return (start % DIV_CEIL(map->dim.w, MAP_BLOCK_WIDTH)) * MAP_BLOCK_WIDTH;
}
inline static uint32 get_block_y(uint32 start) {
	return (start / DIV_CEIL(map->dim.w, MAP_BLOCK_HEIGHT) * MAP_BLOCK_HEIGHT) % (map->dim.h + map->dim.h % 2);
}
inline static uint32 get_block_z(uint32 start) {
	return (start / (DIV_CEIL(map->dim.w, MAP_BLOCK_WIDTH)*DIV_CEIL(map->dim.h, MAP_BLOCK_HEIGHT))) * MAP_BLOCK_DEPTH;
}

#endif

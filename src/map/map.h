#ifndef MAP_MAP_H
#define MAP_MAP_H

#include "maths/maths.h"
#include "gfx/buffers.h"
#include "gfx/model.h"

#define MAP_BLOCK_WIDTH  8
#define MAP_BLOCK_HEIGHT 8
#define MAP_BLOCK_DEPTH  8
#define MAP_CELLS_PER_BLOCK (MAP_BLOCK_WIDTH*MAP_BLOCK_HEIGHT*MAP_BLOCK_DEPTH)
#define MAP_INDICES_PER_VXL 9
#define MAP_VERTEX_COUNT ((MAP_BLOCK_WIDTH + 1)*(MAP_BLOCK_HEIGHT + 1)*(MAP_BLOCK_DEPTH + 1))

enum MapType {
	MAPTYPE_NONE,
	MAPTYPE_FILLED,
	MAPTYPE_ALTERNATING,
	MAPTYPE_RANDOM,
	MAPTYPE_HOLLOW,
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
	uvec3  dim;
	uint32 indc;
	VBO    verts;
	struct MapBlock* inds;
	struct MapCell data[];
}; static_assert(sizeof(struct Map) == 48, "struct Map");

struct MapDrawData {
	alignas(vec4) uvec3 dim;
	alignas(vec4) uvec3 stride;
	// struct Material materials[UINT8_MAX];
};

void map_init(enum MapType type, uvec3 dim);
bool map_is_block_visible(uint block);
bool map_is_cell_visible(uint32 cell);
void map_free();
void map_print();

void map_generate_meshes();

extern struct Map* map;
extern struct MapDrawData mapdd;
extern uintptr mapcellc;
extern uintptr mapblockc;

inline static uint32 map_get_block_index(uint32 x, uint32 y, uint32 z) {
	return x + y*map->dim.w + z*map->dim.w*map->dim.h;
}

inline static uint32 map_get_block_x(uint32 cell) {
	return (cell % DIV_CEIL(map->dim.w, MAP_BLOCK_WIDTH)) * MAP_BLOCK_WIDTH;
}
inline static uint32 map_get_block_y(uint32 cell) {
	return (cell / DIV_CEIL(map->dim.w, MAP_BLOCK_HEIGHT) * MAP_BLOCK_HEIGHT) % (map->dim.h + map->dim.h % 2);
}
inline static uint32 map_get_block_z(uint32 cell) {
	return (cell / (DIV_CEIL(map->dim.w, MAP_BLOCK_WIDTH)*DIV_CEIL(map->dim.h, MAP_BLOCK_HEIGHT))) * MAP_BLOCK_DEPTH;
}

inline static uint32 map_get_cell_x(uint32 cell) { return cell % map->dim.w;                             }
inline static uint32 map_get_cell_y(uint32 cell) { return cell % (map->dim.w * map->dim.h) / map->dim.w; }
inline static uint32 map_get_cell_z(uint32 cell) { return cell / (map->dim.w * map->dim.h);              }

#endif

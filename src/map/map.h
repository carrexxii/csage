#ifndef MAP_MAP_H
#define MAP_MAP_H

#include "gfx/buffers.h"
#include "gfx/model.h"

#define MAP_BLOCK_WIDTH  4
#define MAP_BLOCK_HEIGHT 4
#define MAP_BLOCK_DEPTH  1
#define MAP_CELLS_PER_BLOCK (MAP_BLOCK_WIDTH*MAP_BLOCK_HEIGHT*MAP_BLOCK_DEPTH)
#define MAP_VERTS_PER_VXL 18
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

struct MapBlockIndices {
	uint16 indc;
	IBO    ibo;
}; static_assert(sizeof(struct MapBlockIndices) == 32, "struct MapBlockIndices");

struct Map {
	struct Dim dim;
	uint16 indc;
	VBO    verts;
	struct MapBlockIndices* inds;
	struct MapCell data[];
}; static_assert(sizeof(struct Map) == 48, "struct Map");

struct MapDrawData {
	struct Dim blockstride;
	// struct Material materials[UINT8_MAX];
};

void init_map(enum MapType type, struct Dim dim);

void generate_meshes();

extern struct Map* map;
extern struct MapDrawData mapdd;

#endif

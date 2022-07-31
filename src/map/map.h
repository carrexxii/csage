#ifndef MAP_MAP_H
#define MAP_MAP_H

#include "gfx/buffers.h"
#include "gfx/model.h"

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

struct Map {
	struct Dim dim;
	struct Dim blockdim;
	uint16 meshc;
	struct Model* meshes;
	struct MapCell data[];
}; static_assert(sizeof(struct Map) == 40, "struct Map");

struct Map* create_map(enum MapType type, struct Dim dim, struct Dim blockdim);

struct Model* generate_meshes(struct Map* map);

#endif

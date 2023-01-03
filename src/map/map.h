#ifndef MAP_MAP_H
#define MAP_MAP_H

#include "gfx/buffers.h"
#include "gfx/model.h"

#define MAP_BLOCK_WIDTH  36
#define MAP_BLOCK_HEIGHT 36
#define MAP_BLOCK_DEPTH  4
#define MAP_CELLS_PER_BLOCK (MAP_BLOCK_WIDTH * MAP_BLOCK_HEIGHT * MAP_BLOCK_DEPTH)
#define MAP_INDICES_PER_VXL 9
#define MAP_VERTEX_COUNT ((MAP_BLOCK_WIDTH + 1) * (MAP_BLOCK_HEIGHT + 1) * (MAP_BLOCK_DEPTH + 1))

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
	intptr indc;
	int    zlvl;
	bool   visible;
}; static_assert(sizeof(struct MapBlock) == 40, "struct MapBlock");

struct Map {
	int    w, h, d;    /* dimensions of the map in cells */
	int    bw, bh, bd; /* dimensions of the map in blocks */
	int    blockc;
	struct MapBlock* blocks;
	VBO    verts;
	struct MapCell data[];
}; static_assert(sizeof(struct Map) == 64, "struct Map");

struct MapDrawData {
	alignas(vec4) ivec3 dim;
	alignas(vec4) ivec3 stride;
	// struct Material materials[UINT8_MAX];
};

void map_init(enum MapType type, int w, int h, int d);
bool map_is_block_visible(int block);
bool map_is_cell_visible(int32 cell);
void map_select_cell(bool btndown, int x, int y);
void map_free();
void map_print();

void map_generate_meshes();

extern struct Map* map;
extern struct MapDrawData mapdd;
extern intptr mapcellc;
extern vec4 mapplane;

inline static int map_get_cell_x(int cell) { return cell % map->w;                     }
inline static int map_get_cell_y(int cell) { return cell % (map->w * map->h) / map->w; }
inline static int map_get_cell_z(int cell) { return cell / (map->w * map->h);          }
inline static int map_get_block_index(int x, int y, int z) { return z*map->w*map->h + y*map->w + x; }
inline static int map_get_block_x(int cell) { return cell % (map->w*map->h) / MAP_BLOCK_WIDTH % map->bw;                    }
inline static int map_get_block_y(int cell) { return cell % (map->w*map->h) / (MAP_BLOCK_WIDTH*MAP_BLOCK_HEIGHT) % map->bh; }
inline static int map_get_block_z(int cell) { return cell / (map->w*map->h*MAP_BLOCK_DEPTH);                                }

#endif

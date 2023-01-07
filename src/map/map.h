#ifndef MAP_MAP_H
#define MAP_MAP_H

#include "gfx/buffers.h"
#include "gfx/model.h"

#define MAP_BLOCK_WIDTH  16
#define MAP_BLOCK_HEIGHT 16
#define MAP_BLOCK_DEPTH  8
#define MAP_CELLS_PER_BLOCK (MAP_BLOCK_WIDTH*MAP_BLOCK_HEIGHT*MAP_BLOCK_DEPTH)
#define MAP_INDICES_PER_VXL 18
#define MAP_VERTEX_COUNT    ((MAP_BLOCK_WIDTH + 1)*(MAP_BLOCK_HEIGHT + 1)*(MAP_BLOCK_DEPTH + 1))

enum MapType {
	MAPTYPE_NONE,
	MAPTYPE_FILLED,
	MAPTYPE_ALTERNATING,
	MAPTYPE_RANDOM,
	MAPTYPE_HOLLOW,
	MAPTYPE_TEST,
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
	int w, h, d;    /* dimensions of the map in cells */
	int bw, bh, bd; /* dimensions of the map in blocks */
	int cellc, blockc;
	VBO verts;
	IBO* ibos;
	int* indcs;
	struct MapCell** blocks;
}; static_assert(sizeof(struct Map) == 80, "struct Map");

struct MapDrawData {
	alignas(vec4)    ivec3 dim;          /* Dimensions of a block                  */
	alignas(vec4)    ivec3 stride;       /* Dimensions of the map (in blocks)      */
	alignas(vec4[2]) ivec4 selection[2]; /* Top left and bottom right of selection */
	alignas(vec4)    int zlvl;
};

void map_init(enum MapType type, int w, int h, int d);
bool map_is_block_visible(int block);
bool map_is_visible(int x, int y, int z);
bool map_select_cells_cb(int btn, bool btndown, int x, int y);
bool map_deselect_cells_cb(int btn, bool btndown, int x, int y);
void map_update();
void map_free();
void map_print();

void map_generate_meshes();

extern struct Map map;
extern struct MapDrawData mapdd;

inline static int map_get_cell(int x, int y, int z) { return z*map.w*map.h + y*map.w + x; }
inline static int map_get_block_index(int x, int y, int z) { return map_get_cell(x, y, z) / MAP_CELLS_PER_BLOCK; }
inline static int map_get_block_cell(int x, int y, int z)  { return map_get_cell(x, y, z) % MAP_CELLS_PER_BLOCK; }
inline static int map_get_cell_block_x(int cell) { return cell % map.w;                 }
inline static int map_get_cell_block_y(int cell) { return cell % (map.w*map.h) / map.w; }
inline static int map_get_cell_block_z(int cell) { return cell / (map.w*map.h);         }
inline static int map_get_block_start_x(int block) { return (block % map.bw)*MAP_BLOCK_WIDTH;                    }
inline static int map_get_block_start_y(int block) { return (block % (map.bw*map.bh) / map.bw)*MAP_BLOCK_HEIGHT; }
inline static int map_get_block_start_z(int block) { return (block / (map.bw*map.bh))*MAP_BLOCK_DEPTH;           }
inline static bool map_is_cell(int x, int y, int z) {
	return ((x >= 0 && x < map.w) && (y >= 0 && y < map.h) && (z >= 0 && z < map.d) &&
	        map.blocks[map_get_block_index(x, y, z)][map_get_block_cell(x, y, z)].data);
}

#endif


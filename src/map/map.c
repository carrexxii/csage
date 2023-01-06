#include "config.h"
#include "util/maths.h"
#include "input.h"
#include "camera.h"
#include "map.h"

struct Map map;
struct MapDrawData mapdd;

void map_init(enum MapType type, int w, int h, int d)
{
	map.bw = DIV_CEIL(w, MAP_BLOCK_WIDTH);
	map.bh = DIV_CEIL(h, MAP_BLOCK_HEIGHT);
	map.bd = DIV_CEIL(d, MAP_BLOCK_DEPTH);
	map.w = map.bw*MAP_BLOCK_WIDTH;
	map.h = map.bh*MAP_BLOCK_HEIGHT;
	map.d = map.bd*MAP_BLOCK_DEPTH;
	map.cellc  = w*h*d;
	map.blockc = DIV_CEIL(w, MAP_BLOCK_WIDTH)*DIV_CEIL(h, MAP_BLOCK_HEIGHT)*DIV_CEIL(d, MAP_BLOCK_DEPTH);
	map.ibos   = scalloc(map.blockc, sizeof(IBO));
	map.indcs  = scalloc(map.blockc, sizeof(map.indcs[0]));
	map.blocks = scalloc(map.blockc, MAP_CELLS_PER_BLOCK*sizeof(struct MapCell));
	// TODO: leave this as a sparse array
	for (int block = 0; block < map.blockc; block++)
		map.blocks[block] = scalloc(MAP_CELLS_PER_BLOCK, sizeof(struct MapCell));

	int x, y, z;
	switch (type) {
		case MAPTYPE_NONE:
			ERROR("[MAP] Map type should not be none");
			break;
		case MAPTYPE_FILLED:
			for (int block = 0; block < map.blockc; block++)
				memset(map.blocks[block], CELLTYPE_GRASS, MAP_CELLS_PER_BLOCK*sizeof(struct MapCell));
			break;
		case MAPTYPE_TEST:
			for (int block = 0; block < map.blockc; block++)
				memset(map.blocks[block], CELLTYPE_GRASS, MAP_CELLS_PER_BLOCK*sizeof(struct MapCell));
			for (int cell = 0; cell < MAP_CELLS_PER_BLOCK; cell++) {
				x = map_get_cell_block_x(cell);
				y = map_get_cell_block_y(cell);
				z = map_get_cell_block_z(cell);
				if ((z < map.d/2) || (z == map.d/2 &&
				    ((x > 0 && x < map.w - 1) && (y > 0 && y < map.h - 1)) &&
				    ((y != map.h/2) || !(x > 2 && x < map.w - 3))))
					map.blocks[0][cell].data = 0;
			}
			break;
		default:
			ERROR("[MAP] Invalid map type %d", type);
	}

	map_print();
	map_generate_meshes(map);

	glm_ivec3_copy((ivec3){ map.bw, map.bh, map.bh }, mapdd.dim);
	glm_ivec3_copy((ivec3){ MAP_BLOCK_WIDTH, MAP_BLOCK_HEIGHT, MAP_BLOCK_DEPTH }, mapdd.stride);
	map_deselect_cells(0, false, 0, 0);

	DEBUG(1, "[MAP] Created map with size %dx%dx%d (%d total) (%dx%dx%d = %d blocks)",
	      w, h, d, map.cellc, map.bw, map.bh, map.bd, map.blockc);
}

bool map_is_block_visible(int block)
{
	// if (abs(map.d - camzlvl) > MAP_BLOCK_DEPTH)
		// return false;

	return true;
}

bool map_is_visible(int x, int y, int z)
{
	int block = map_get_block_index(x, y, z);
	int cell  = map_get_block_cell(x, y, z);
	if (!map.blocks[block][cell].data)
		return false;

	return true;
	// int zstride = map.w * map.h;
	// int ystride = map.h;
	// int32 x = map_get_cell_x(cell);
	// int32 y = map_get_cell_y(cell);
	// int32 z = map_get_cell_z(cell);

	// /* Check if it at the top of a block */
	// if (!(z % MAP_BLOCK_DEPTH))
	// 	return true;

	// /* Edge checks (left edge || right edge */
	// if (!(cell % map.w) || cell % (map.w*map.h) < map.w)
	// 	return true;

	// /* Diagonal check */
	// int diag1 = cell;
	// int diag2 = cell - 1;
	// int diag3 = cell - ystride;
	// bool isblocked = false;
	// while (diag3 >= zstride + ystride + 1) {
	// 	diag1 -= zstride + ystride + 1;
	// 	diag2 -= zstride + ystride + 1;
	// 	diag3 -= zstride + ystride + 1;
	// 	if (map.data[diag1].data && map.data[diag2].data && map.data[diag3].data) {
	// 		isblocked = true;
	// 		break;
	// 	}
	// }

	// return !isblocked;
}

void map_select_cells(int btn, bool btndown, int x, int y)
{
	static bool mbdown;

	vec3 p;
	if (btn == MOUSE_LEFT) {
		mbdown = btndown;
		if (btndown) {
			camera_unproject((float)x, (float)y, p);
			ivec3_copy_vec3(p, mapdd.selection[0]);
			ivec3_copy_vec3(p, mapdd.selection[1]);
		}
	} else {
		if (!mbdown) {
			glm_vec3_copy((vec3){ 0, 0, 0 }, p);
		} else {
			camera_unproject((float)x, (float)y, p);
			ivec3_copy_vec3(p, mapdd.selection[1]);
		}
	}
	mapdd.selection[0][2] = camzlvl;
	mapdd.selection[1][2] = camzlvl;
}

void map_deselect_cells(int btn, bool btndown, int x, int y)
{
	glm_ivec4_copy((ivec4){ -1, -1, -1, 0 }, mapdd.selection[0]);
	glm_ivec4_copy((ivec4){ -1, -1, -1, 0 }, mapdd.selection[1]);
}

void map_update()
{
	/* check if the z-lvl was changed */
	if (mapdd.zlvl != camzlvl)
		map_deselect_cells(0, false, 0, 0);
	mapdd.zlvl = camzlvl;
}

void map_free()
{
	DEBUG(1, "[MAP] Freeing map...");
	for (int block = 0; block < map.blockc; block++) {
		buffer_free(&map.ibos[block]);
		free(map.blocks[block]);
	}
	buffer_free(&map.verts);
	free(map.blocks);
	free(map.indcs);
}

void map_print()
{
	fprintf(stderr, "Map (%ux%ux%u):\n[ z  y]\n", map.w, map.h, map.d);
	for (int block = 0; block < map.blockc; block++) {
		for (int z = 0; z < MAP_BLOCK_DEPTH; z++) {
			for (int y = 0; y < MAP_BLOCK_HEIGHT; y++) {
				fprintf(stderr, "[%2d-%2d] ", z, y);
				for (int x = 0; x < MAP_BLOCK_WIDTH; x++)
					fprintf(stderr, "%3hu ", map.blocks[block][z*MAP_BLOCK_WIDTH*MAP_BLOCK_HEIGHT + y*MAP_BLOCK_WIDTH + x].data);
				fprintf(stderr, "\n");
			}
			fprintf(stderr, "\n");
		}
		fprintf(stderr, "^block %2d |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n\n", block);
	}
}


#include "config.h"
#include "util/maths.h"
#include "input.h"
#include "camera.h"
#include "map.h"

struct Map* map;
struct MapDrawData mapdd;
intptr mapcellc;
vec4   mapplane;

void map_init(enum MapType type, int w, int h, int d)
{
	mapcellc  = w * h * d;
	if (map)
		free(map);
	map = scalloc(sizeof(struct Map) + mapcellc*sizeof(struct MapCell), 1);
	map->blockc = DIV_CEIL(w, MAP_BLOCK_WIDTH) * DIV_CEIL(h, MAP_BLOCK_HEIGHT) * DIV_CEIL(d, MAP_BLOCK_DEPTH);
	map->blocks = scalloc(map->blockc, sizeof(struct MapBlock));
	map->w = w;
	map->h = h;
	map->d = d;
	map->bw = DIV_CEIL(w, MAP_BLOCK_WIDTH);
	map->bh = DIV_CEIL(h, MAP_BLOCK_HEIGHT);
	map->bd = DIV_CEIL(d, MAP_BLOCK_DEPTH);

	int x, y, z;
	switch (type) {
		case MAPTYPE_NONE:
			ERROR("[MAP] Map type should not be none");
			break;
		case MAPTYPE_FILLED:
			memset(map->data, CELLTYPE_GRASS, mapcellc*sizeof(struct MapCell));
			break;
		case MAPTYPE_ALTERNATING:
			for (int i = 0; i < mapcellc; i++)
				map->data[i].data = i % 2 - i/map->h % 2;
			break;
		case MAPTYPE_RANDOM:
			for (int i = 0; i < mapcellc; i++)
				map->data[i].data = random_int(0, 1);
			break;
		case MAPTYPE_HOLLOW:
			for (int i = 0; i < mapcellc; i++) {
				x = map_get_cell_x(i);
				y = map_get_cell_y(i);
				z = map_get_cell_z(i);
				map->data[i].data = (uint)((x == 0 && y == 0) || (x == w - 1 && y == h - 1) ||
				                           (x == w - 1) || (y == h - 1) ||
				                           (z == d - 1) || (z == 0 && (x == 0 || y == 0)));
			}
			break;
		default:
			ERROR("[MAP] Invalid map type %d", type);
	}
	
	for (int i = 0; i < map->blockc; i++)
		map->blocks[i].visible = map_is_block_visible(i);

	// DEBUG(1, "x  y  z");
	// for (int i = 0; i < mapcellc; i++) {
	// 	DEBUG(1, "[%d] %d, %d, %d", i, map_get_block_x(i), map_get_block_y(i), map_get_block_z(i));
	// }

	// map_print();
	map_generate_meshes(map);

	// DEBUG(1, "Total blocks: %d", map->blockc);
	// for (int i = 0; i < map->blockc; i++)
	// 	DEBUG(1, "[%d] %d", i, map->blocks[i].zlvl);

	glm_ivec3_copy((ivec3){ map->bw, map->bh, map->bh }, mapdd.dim);
	glm_ivec3_copy((ivec3){ MAP_BLOCK_WIDTH, MAP_BLOCK_HEIGHT, MAP_BLOCK_DEPTH }, mapdd.stride);
	map_deselect_cells(0, false, 0, 0);

	if (d <= MAP_BLOCK_DEPTH)
		camzlvlmax = 0;
	else
		camzlvlmax = map->bd - 1;
	camzlvlscale = MAP_BLOCK_DEPTH;

	glm_vec4_copy((vec4){ 0.0, 0.0, -1.0, 0.0 }, mapplane);
	glm_plane_normalize(mapplane);

	DEBUG(1, "[MAP] Created map with size %dx%dx%d (%lu total) (%dx%dx%d = %d blocks)",
	      w, h, d, mapcellc, map->bw, map->bh, map->bd, map->blockc);
}

bool map_is_block_visible(int block)
{
	return true;
}

bool map_is_cell_visible(int32 cell)
{
	if (!map->data[cell].data)
		return false;

	int zstride = map->w * map->h;
	int ystride = map->h;
	int32 x = map_get_cell_x(cell);
	int32 y = map_get_cell_y(cell);
	int32 z = map_get_cell_z(cell);
	return true; // !!!!!!

	/* Check if it at the top of a block */
	if (!(z % MAP_BLOCK_DEPTH))
		return true;

	/* Edge checks (left edge || right edge */
	if (!(cell % map->w) || cell % (map->w*map->h) < map->w)
		return true;

	/* Diagonal check */
	int diag1 = cell;
	int diag2 = cell - 1;
	int diag3 = cell - ystride;
	bool isblocked = false;
	while (diag3 >= zstride + ystride + 1) {
		diag1 -= zstride + ystride + 1;
		diag2 -= zstride + ystride + 1;
		diag3 -= zstride + ystride + 1;
		if (map->data[diag1].data && map->data[diag2].data && map->data[diag3].data) {
			isblocked = true;
			break;
		}
	}

	return !isblocked;
}

static bool mbdown;
void map_select_cells(int btn, bool btndown, int x, int y)
{
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
}

void map_deselect_cells(int btn, bool btndown, int x, int y)
{
	glm_ivec4_copy((ivec4){ -1, -1, -1, 0 }, mapdd.selection[0]);
	glm_ivec4_copy((ivec4){ -1, -1, -1, 0 }, mapdd.selection[1]);
}

void map_free()
{
	DEBUG(1, "[MAP] Freeing map...");
	buffer_free(&map->verts);
	for (int i = 0; i < map->blockc; i++)
		buffer_free(&map->blocks[i].ibo);
	free(map);
}

void map_print()
{
	fprintf(stderr, "Map (%ux%ux%u):\n", map->w, map->h, map->d);
	int w = map->w,
	    h = map->h,
	    d = map->d;
	for (int z = 0; z < d; z++) {
		for (int y = 0; y < h; y++) {
			fprintf(stderr, "[%2u] ", y);
			for (int x = 0; x < w; x++)
				fprintf(stderr, "%3hu ", map->data[z*w*h + y*w + x].data);
			fprintf(stderr, "\n");
		}
		fprintf(stderr, "%2d |- - - - - - - - - - - - - - - -\n", z);
	}
}

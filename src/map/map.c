#include "camera.h"
#include "map.h"

struct Map* map;
struct MapDrawData mapdd;
uintptr mapcellc;
uintptr mapblockc;

void map_init(enum MapType type, uvec3 dim)
{
	mapcellc  = vec_volume(dim);
	mapblockc = DIV_CEIL(dim.w, MAP_BLOCK_WIDTH)  *
	            DIV_CEIL(dim.h, MAP_BLOCK_HEIGHT) * 
	            DIV_CEIL(dim.d, MAP_BLOCK_DEPTH);
	if (map)
		free(map);
	map = scalloc(sizeof(struct Map) + mapcellc*sizeof(struct MapCell), 1);
	map->inds = scalloc(mapblockc, sizeof(struct MapBlock));
	map->dim  = dim;
	uint x, y, z;
	switch (type) {
		case MAPTYPE_NONE:
			ERROR("[MAP] Map type should not be none");
			break;
		case MAPTYPE_FILLED: /* Top half is left empty */
			memset(map->data, CELLTYPE_EMPTY, mapcellc/2*sizeof(struct MapCell));
			memset(map->data + mapcellc/2, CELLTYPE_GRASS, mapcellc/2*sizeof(struct MapCell));
			break;
		case MAPTYPE_ALTERNATING:
			for (uint i = 0; i < mapcellc; i++)
				map->data[i].data = i % 2 - i/map->dim.h % 2;
			break;
		case MAPTYPE_RANDOM:
			for (uint i = 0; i < mapcellc; i++)
				map->data[i].data = random_int(0, 1);
			break;
		case MAPTYPE_HOLLOW:
			for (uint i = 0; i < mapcellc; i++) {
				x = map_get_cell_x(i);
				y = map_get_cell_y(i);
				z = map_get_cell_z(i);
				map->data[i].data = (uint)((x == 0 && y == 0) || (x == dim.w - 1 && y == dim.h - 1) ||
				                           (x == dim.w - 1) || (y == dim.h - 1) ||
				                           (z == dim.d - 1) || (z == 0 && (x == 0 || y == 0)));
			}
			break;
		default:
			ERROR("[MAP] Invalid map type %d", type);
	}
	
	for (uint i = 0; i < mapblockc; i++)
		map->inds[i].visible = map_is_block_visible(i);

	map_generate_meshes(map);

	mapdd.dim = UVEC3(DIV_CEIL(dim.w, MAP_BLOCK_WIDTH), DIV_CEIL(dim.h, MAP_BLOCK_HEIGHT), DIV_CEIL(dim.d, MAP_BLOCK_DEPTH));
	mapdd.stride = UVEC3(MAP_BLOCK_WIDTH, MAP_BLOCK_HEIGHT, MAP_BLOCK_DEPTH);

	if (dim.d < MAP_BLOCK_DEPTH)
		camZlvlMax = 0;
	else
		camZlvlMax = DIV_CEIL(dim.d, MAP_BLOCK_DEPTH) - 1;
	camZlvlScale = MAP_BLOCK_DEPTH;

	DEBUG(1, "[MAP] Created map with size %ux%ux%u (%lu total)", dim.w, dim.h, dim.d, mapcellc);
}

bool map_is_block_visible(uint block)
{
	return true;
}

bool map_is_cell_visible(uint32 cell)
{
	if (!map->data[cell].data)
		return false;

	uint zstride = map->dim.w * map->dim.h;
	uint ystride = map->dim.h;
	uint32 x = map_get_cell_x(cell);
	uint32 y = map_get_cell_y(cell);
	uint32 z = map_get_cell_z(cell);
	return true;

	/* Check if it at the top of a block */
	if (!(z % MAP_BLOCK_DEPTH))
		return true;

	/* Edge checks (left edge || right edge */
	if (!(cell % map->dim.w) || cell % (map->dim.w*map->dim.h) < map->dim.w)
		return true;

	/* Diagonal check */
	uint diag1     = cell;
	uint diag2     = cell - 1;
	uint diag3     = cell - ystride;
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

void map_select_cell(bool btndown)
{
	D;
}

void map_free()
{
	DEBUG(1, "[MAP] Freeing map...");
	buffer_free(&map->verts);
	for (uint i = 0; i < map->indc; i++)
		buffer_free(&map->inds[i].ibo);
	free(map);
}

void map_print()
{
	fprintf(stderr, "Map (%ux%ux%u):\n", map->dim.w, map->dim.h, map->dim.d);
	for (uint z = 0; z < map->dim.d; z++) {
		for (uint y = 0; y < map->dim.h; y++) {
			fprintf(stderr, "[%u] ", y);
			for (uint x = 0; x < map->dim.w; x++)
				fprintf(stderr, "%2hu ", map->data[z*MAP_BLOCK_HEIGHT*MAP_BLOCK_DEPTH + y*MAP_BLOCK_WIDTH + x].data);
			fprintf(stderr, "\n");
		}
		fprintf(stderr, " - - - - - - - - - - - - - - - -\n");
	}
}

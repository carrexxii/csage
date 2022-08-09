#include "camera.h"
#include "map.h"

struct Map* map;
struct MapDrawData mapdd;
uintptr mapcellc;
uintptr mapblockc;

static uint skipc;

void init_map(enum MapType type, struct Dim dim)
{
	mapcellc  = volume_of(dim);
	mapblockc = DIV_CEIL(dim.w, MAP_BLOCK_WIDTH)  *
	            DIV_CEIL(dim.h, MAP_BLOCK_HEIGHT) * 
	            DIV_CEIL(dim.d, MAP_BLOCK_DEPTH);
	if (map)
		free(map);
	map = scalloc(sizeof(struct Map) + mapcellc*sizeof(struct MapCell), 1);
	map->inds = scalloc(mapblockc, sizeof(struct MapBlock));
	map->dim  = dim;
	switch (type) {
		case MAPTYPE_NONE:
			ERROR("[MAP] Block type should not be none");
			break;
		case MAPTYPE_FILLED:
			memset(map->data, CELLTYPE_GRASS, mapcellc*sizeof(struct MapCell));
			break;
		case MAPTYPE_ALTERNATING:
			for (uint i = 0; i < mapcellc; i++)
				map->data[i].data = i % 2 - i/map->dim.h % 2;
			break;
		case MAPTYPE_RANDOM:
			for (uint i = 0; i < mapcellc; i++)
				map->data[i].data = random_int(0, 1);
			break;
		default:
			ERROR("[MAP] Invalid map type %d", type);
	}
	
	for (uint i = 0; i < mapblockc; i++) {
		map->inds[i].visible = is_block_visible(i);
		// DEBUG(1, "\t%s", STRING_TF(map->inds[i].visible));
	}

	generate_meshes(map);

	mapdd.dim = (struct Dim4){
		.w = DIV_CEIL(dim.w, MAP_BLOCK_WIDTH),
		.h = DIV_CEIL(dim.h, MAP_BLOCK_HEIGHT),
		.d = DIV_CEIL(dim.d, MAP_BLOCK_DEPTH),
	};
	mapdd.stride = (struct Dim4){
		.w = MAP_BLOCK_WIDTH,
		.h = MAP_BLOCK_HEIGHT,
		.d = MAP_BLOCK_DEPTH,
	};

	if (dim.d < MAP_BLOCK_DEPTH)
		camzlvlmax = 0;
	else
		camzlvlmax = DIV_CEIL(dim.d, MAP_BLOCK_DEPTH) - 1;
	camzlvlscale = MAP_BLOCK_DEPTH;

	DEBUG(1, "[MAP] Created map with size %ux%ux%u (%lu total) (%u skipped blocks)", dim.w, dim.h, dim.d, mapcellc, skipc);
}

bool is_block_visible(uint block)
{
	uint x = get_block_x(block);
	uint y = get_block_y(block);
	uint z = get_block_z(block);
	// DEBUG(1, "%u %u %u", x, y, z);
	// if (x == 0 || y == 0 || z == 0)
		return true;
	// return false;
}

bool is_cell_visible(uintptr cell)
{
	if (!map->data[cell].data)
		return false;

	uint zstride = map->dim.w * map->dim.h;
	uint ystride = map->dim.h;
	// DEBUG(1, "stride: z:%u y:%u | cell: %lu", zstride, ystride, cell);
	// DEBUG(1, "Checking %lu above (%hu)", cell - zstride, map->data[cell - zstride].data);
	/* Check if it at the top of a block */
	if (!(get_cell_zlvl(cell) % MAP_BLOCK_DEPTH)) {
		// DEBUG(1, "[%lu] zlvl: %u", cell, get_cell_zlvl(cell));
		// DEBUG(1, "[%lu] Top block...", cell);
		skipc++;
		return true;
	}
	/* Edge checks (left edge || right edge */
	if (!(cell % map->dim.w) || cell % (map->dim.w*map->dim.h) < map->dim.w) {
		// DEBUG(1, "[%lu] Edge found...", cell);
		skipc++;
		return true;
	}
	// DEBUG(1, "[%lu] x", cell);
	return false;
}

void free_map()
{
	DEBUG(1, "[MAP] Freeing map...");
	free_buffer(&map->verts);
	for (uint i = 0; i < map->indc; i++)
		free_buffer(&map->inds[i].ibo);
	free(map);
}

void print_map()
{
	fprintf(stderr, "Map (%ux%ux%u):\n", map->dim.w, map->dim.h, map->dim.d);
	for (uint z = 0; z < map->dim.d; z++) {
		for (uint y = 0; y < map->dim.h; y++) {
			fprintf(stderr, "[%u] ", y);
			for (uint x = 0; x < map->dim.w; x++) {
				fprintf(stderr, "%2hu ", map->data[z*MAP_BLOCK_HEIGHT*MAP_BLOCK_DEPTH + y*MAP_BLOCK_WIDTH + x].data);
			}
			fprintf(stderr, "\n");
		}
		fprintf(stderr, " - - - - - - - - - - - - - - - -\n");
	}
}

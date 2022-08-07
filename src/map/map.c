#include "map.h"

static void print_map();

struct Map* map;
struct MapDrawData mapdd;

void init_map(enum MapType type, struct Dim dim)
{
	uint cellc = volume_of(dim);
	if (map)
		free(map);
	map = scalloc(sizeof(struct Map) + cellc*sizeof(struct MapCell), 1);
	map->dim = dim;
	switch (type) {
		case MAPTYPE_NONE:
			ERROR("[MAP] Block type should not be none");
			break;
		case MAPTYPE_FILLED:
			memset(map->data, CELLTYPE_GRASS, cellc*sizeof(struct MapCell));
			break;
		case MAPTYPE_RANDOM:
			break;
		default:
			ERROR("[MAP] Invalid map type %d", type);
	}
	print_map();
	generate_meshes(map);

	mapdd.blockstride = (struct Dim){
		.w = MAP_BLOCK_WIDTH  * sizeof(uint16),
		.h = MAP_BLOCK_HEIGHT * sizeof(uint16),
		.d = MAP_BLOCK_DEPTH  * sizeof(uint16),
	};

	DEBUG(1, "[MAP] Created map with size %ux%ux%u (%u total)", dim.w, dim.h, dim.d, cellc);
}

static void print_map()
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

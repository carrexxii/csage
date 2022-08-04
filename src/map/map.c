#include "map.h"

struct Map* map;
struct MapDrawData mapdd;

void init_map(enum MapType type, struct Dim dim)
{
	uint blockc = volume_of(dim);
	if (map)
		free(map);
	map = scalloc(sizeof(struct Map) + blockc*sizeof(struct MapCell), 1);
	map->dim = dim;
	switch (type) {
		case MAPTYPE_NONE:
			ERROR("[MAP] Block type should not be none");
			break;
		case MAPTYPE_FILLED:
			memset(map->data, CELLTYPE_GRASS, blockc);
			break;
		case MAPTYPE_RANDOM:
			break;
		default:
			ERROR("[MAP] Invalid map type %d", type);
	}
	generate_meshes(map);

	mapdd.blockstride = (struct Dim){
		.w = MAP_BLOCK_WIDTH  * sizeof(uint16),
		.h = MAP_BLOCK_HEIGHT * sizeof(uint16),
		.d = MAP_BLOCK_DEPTH  * sizeof(uint16),
	};

	DEBUG(1, "[MAP] Created map with size %ux%ux%u (%u total)", dim.w, dim.h, dim.d, blockc);
}

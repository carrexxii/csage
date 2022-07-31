#include "map.h"

struct Map* create_map(enum MapType type, struct Dim dim, struct Dim blockdim)
{
	uintptr blockc  = volume_of(dim)*volume_of(blockdim);
	struct Map* map = scalloc(sizeof(struct Map) + blockc, sizeof(struct MapCell));
	map->dim      = dim;
	map->blockdim = blockdim;

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
	map->meshes = generate_meshes(map);

	DEBUG(1, "[MAP] Created map: %ux%ux%u blocks each of %ux%ux%u (%lu total)", dim.w, dim.h, dim.d, 
	      blockdim.w, blockdim.h, blockdim.d, blockc);
	return map;
}

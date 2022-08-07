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
	// print_map();
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

	DEBUG(1, "[MAP] Created map with size %ux%ux%u (%u total)", dim.w, dim.h, dim.d, cellc);
}

void free_map()
{
	DEBUG(1, "[MAP] Freeing map...");
	free_buffer(&map->verts);
	for (uint i = 0; i < map->indc; i++)
		free_buffer(&map->inds[i].ibo);
	free(map);
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

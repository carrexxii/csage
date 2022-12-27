#include "camera.h"
#include "map.h"

struct Map* map;
struct MapDrawData mapdd;
uintptr mapcellc;
uintptr mapblockc;

void map_init(enum MapType type, uvec3 dim)
{
	mapcellc  = dim[0] * dim[1] * dim[2];
	mapblockc = DIV_CEIL(dim[0], MAP_BLOCK_WIDTH)  *
	            DIV_CEIL(dim[1], MAP_BLOCK_HEIGHT) * 
	            DIV_CEIL(dim[2], MAP_BLOCK_DEPTH);
	if (map)
		free(map);
	map = scalloc(sizeof(struct Map) + mapcellc*sizeof(struct MapCell), 1);
	map->inds = scalloc(mapblockc, sizeof(struct MapBlock));
	uvec3_copy(dim, map->dim);
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
				map->data[i].data = i % 2 - i/map->dim[1] % 2;
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
				map->data[i].data = (uint)((x == 0 && y == 0) || (x == dim[0] - 1 && y == dim[1] - 1) ||
				                           (x == dim[0] - 1) || (y == dim[1] - 1) ||
				                           (z == dim[2] - 1) || (z == 0 && (x == 0 || y == 0)));
			}
			break;
		default:
			ERROR("[MAP] Invalid map type %d", type);
	}
	
	for (uint i = 0; i < mapblockc; i++)
		map->inds[i].visible = map_is_block_visible(i);

	map_generate_meshes(map);

	uvec3_copy((uvec3){ DIV_CEIL(dim[0], MAP_BLOCK_WIDTH),
	                    DIV_CEIL(dim[1], MAP_BLOCK_HEIGHT),
	                    DIV_CEIL(dim[2], MAP_BLOCK_DEPTH) },
	            mapdd.dim);
	uvec3_copy((uvec3){ MAP_BLOCK_WIDTH, MAP_BLOCK_HEIGHT, MAP_BLOCK_DEPTH }, mapdd.stride);

	if (dim[2] < MAP_BLOCK_DEPTH)
		camzlvlmax = 0;
	else
		camzlvlmax = DIV_CEIL(dim[2], MAP_BLOCK_DEPTH) - 1;
	camzlvlscale = MAP_BLOCK_DEPTH;

	DEBUG(1, "[MAP] Created map with size %ux%ux%u (%lu total)", dim[0], dim[1], dim[2], mapcellc);
}

bool map_is_block_visible(uint block)
{
	return true;
}

bool map_is_cell_visible(uint32 cell)
{
	if (!map->data[cell].data)
		return false;

	uint zstride = map->dim[0] * map->dim[1];
	uint ystride = map->dim[1];
	uint32 x = map_get_cell_x(cell);
	uint32 y = map_get_cell_y(cell);
	uint32 z = map_get_cell_z(cell);
	return true;

	/* Check if it at the top of a block */
	if (!(z % MAP_BLOCK_DEPTH))
		return true;

	/* Edge checks (left edge || right edge */
	if (!(cell % map->dim[0]) || cell % (map->dim[0]*map->dim[1]) < map->dim[0])
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

void map_select_cell(bool btndown, int x, int y)
{
	vec4 mp = { (float)x, (float)y, 0.0, 1.0 };
	mat4 vp;
	// mat4 inv;
	camera_get_vp(vp);

	DEBUG(1, "Mouse position: %d %d", x, y);
	// mat_copy(&inv, &vp);
	// mat_inv(&inv);
	// mat_mul_v_ip(&mp, &inv);
	// mat_mul_v_ip(&mp, &vp);
	// vec_print(mp);
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
	fprintf(stderr, "Map (%ux%ux%u):\n", map->dim[0], map->dim[1], map->dim[2]);
	for (uint z = 0; z < map->dim[2]; z++) {
		for (uint y = 0; y < map->dim[1]; y++) {
			fprintf(stderr, "[%u] ", y);
			for (uint x = 0; x < map->dim[0]; x++)
				fprintf(stderr, "%2hu ", map->data[z*MAP_BLOCK_HEIGHT*MAP_BLOCK_DEPTH + y*MAP_BLOCK_WIDTH + x].data);
			fprintf(stderr, "\n");
		}
		fprintf(stderr, " - - - - - - - - - - - - - - - -\n");
	}
}

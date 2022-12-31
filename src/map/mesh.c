#include "common.h"
#include "gfx/vertices.h"
#include "map.h"

struct Vertex {
	uint32 z: 10;
	uint32 y: 10;
	uint32 x: 10;
	uint32 a: 2;
}; static_assert(sizeof(struct Vertex) == 4, "struct Vertex");

static uint generate_block_indices(uint16* inds, intptr start);

static struct Vertex verts[(MAP_BLOCK_WIDTH + 1)*(MAP_BLOCK_HEIGHT + 1)*(MAP_BLOCK_DEPTH + 1)];

void map_generate_meshes()
{
	/* Vertices */
	int vertc = (MAP_BLOCK_DEPTH + 1) * (MAP_BLOCK_HEIGHT + 1) * (MAP_BLOCK_WIDTH + 1);
	assert(MAP_INDICES_PER_VXL*vertc < UINT16_MAX);

	struct Vertex* v = verts; 
	for (int z = 0; z < MAP_BLOCK_DEPTH + 1; z++)
		for (int y = 0; y < MAP_BLOCK_HEIGHT + 1; y++)
			for (int x = 0; x < MAP_BLOCK_WIDTH + 1; x++)
				*v++ = (struct Vertex){ .x = x, .y = y, .z = z, .a = 1 };
	map->verts = vbo_new(vertc*sizeof(struct Vertex), verts);

	/* Indices */
	intptr indc       = 0;
	intptr totalindc  = 0;
	uint16* inds      = smalloc(MAP_CELLS_PER_BLOCK * MAP_INDICES_PER_VXL * sizeof(uint16));
	intptr currblock  = 0;
	for (int z = 0; z < MAX((map->d + MAP_BLOCK_DEPTH - 1) / MAP_BLOCK_DEPTH, 1); z++) {
		for (int y = 0; y < MAX((map->h + MAP_BLOCK_HEIGHT - 1) / MAP_BLOCK_HEIGHT, 1); y++) {
			for (int x = 0; x < MAX((map->w + MAP_BLOCK_WIDTH - 1) / MAP_BLOCK_WIDTH, 1); x++) {
				indc = generate_block_indices(inds, currblock);
				totalindc += indc;
				map->blocks[currblock].ibo  = ibo_new(indc*sizeof(uint16), inds);
				map->blocks[currblock].indc = indc;
				map->blocks[currblock].zlvl = currblock / (map->bw*map->bh);
				currblock++;
			}
		}
	}

	free(inds);
	DEBUG(3, "[MAP] Generated mesh for map with %ld cells (%ld vertices in %d blocks)", mapcellc, totalindc, map->blockc);
} 

static uint generate_block_indices(uint16* inds, intptr start)
{
	int blockx = map_get_block_x(start);
	int blocky = map_get_block_y(start);
	int blockz = map_get_block_z(start);

	int indc   = 0;
	int rowc   =  MAP_BLOCK_WIDTH + 1;                           /* # of cells per row     */
	int layerc = (MAP_BLOCK_WIDTH + 1) * (MAP_BLOCK_HEIGHT + 1); /* # of cells per z-level */
	int vx, vy, vz;
	for (int z = 0; z < MAP_BLOCK_DEPTH; z++) {
		for (int y = 0; y < MAP_BLOCK_HEIGHT; y++) {
			for (int x = 0; x < MAP_BLOCK_WIDTH; x++) {
				if (blockx + x >= map->w || blocky + y >= map->h || blockz + z >= map->d) 
					continue;
				if (!map_is_cell_visible(map_get_block_index(blockx + x, blocky + y, blockz + z)))
					continue;
				indc += MAP_INDICES_PER_VXL;
				vx = x % MAP_BLOCK_WIDTH;
				vy = y % MAP_BLOCK_HEIGHT;
				vz = z % MAP_BLOCK_DEPTH;

				/* Triangle fan
				 * 5---6---7
				 * | \ | / |
				 * 4---1---8   + restart = 9 verts
				 * | / |
				 * 3---2
				 */
				*inds++ = vx     +       vy*rowc +       vz*layerc;
				*inds++ = vx     +       vy*rowc + (vz + 1)*layerc;
				*inds++ = vx     + (vy + 1)*rowc + (vz + 1)*layerc;
				*inds++ = vx     + (vy + 1)*rowc +       vz*layerc;
				*inds++ = vx + 1 + (vy + 1)*rowc +       vz*layerc;
				*inds++ = vx + 1 +       vy*rowc +       vz*layerc;
				*inds++ = vx + 1 +       vy*rowc + (vz + 1)*layerc;
				*inds++ = vx     +       vy*rowc + (vz + 1)*layerc;
				*inds++ = UINT16_MAX;
			}
		}
	}

	return indc;
}

#include "common.h"
#include "gfx/vertices.h"
#include "map.h"

struct Vertex {
	uint32 z: 10;
	uint32 y: 10;
	uint32 x: 10;
	uint32 a: 2;
}; static_assert(sizeof(struct Vertex) == 4, "struct Vertex");

static uint generate_block_indices(uint16* inds, uintptr start);

static struct Vertex verts[(MAP_BLOCK_WIDTH + 1)*(MAP_BLOCK_HEIGHT + 1)*(MAP_BLOCK_DEPTH + 1)];

void map_generate_meshes()
{
	/* Vertices */
	uint vertc = (MAP_BLOCK_DEPTH + 1) * (MAP_BLOCK_HEIGHT + 1) * (MAP_BLOCK_WIDTH + 1);
	struct Vertex* v = verts;
	for (uint z = 0; z < MAP_BLOCK_DEPTH + 1; z++)
		for (uint y = 0; y < MAP_BLOCK_HEIGHT + 1; y++)
			for (uint x = 0; x < MAP_BLOCK_WIDTH + 1; x++)
				*v++ = (struct Vertex){ .x = x, .y = y, .z = z, .a = 1 };
	map->verts = vbo_new(vertc*sizeof(struct Vertex), verts);

	/* Indices */
	uintptr indc      = 0;
	uintptr totalindc = 0;
	uint16* inds      = smalloc(MAP_CELLS_PER_BLOCK * MAP_INDICES_PER_VXL * sizeof(uint16));
	uintptr currblock = 0;
	for (uint z = 0; z < MAX((map->dim[2] + MAP_BLOCK_DEPTH - 1) / MAP_BLOCK_DEPTH, 1); z++) {
		for (uint y = 0; y < MAX((map->dim[1] + MAP_BLOCK_HEIGHT - 1) / MAP_BLOCK_HEIGHT, 1); y++) {
			for (uint x = 0; x < MAX((map->dim[0] + MAP_BLOCK_WIDTH - 1) / MAP_BLOCK_WIDTH, 1); x++) {
				if (!(indc = generate_block_indices(inds, currblock)))
					continue;
				totalindc += indc;
				map->inds[map->indc  ].ibo  = ibo_new(indc*sizeof(uint16), inds);
				map->inds[map->indc  ].indc = indc;
				map->inds[map->indc++].zlvl = (int)(map_get_block_z(currblock) / MAP_BLOCK_DEPTH);
				currblock++;
			}
		}
	}
	// DEBUG(1, "map: indc[0] = %u && ibo = %lu", map->inds[0].indc, (uintptr)map->inds[0].ibo.buf);
	// for (uint i = 0; i < vertc; i++)
	// 	DEBUG(1, "[v%u] %u %u %u", i, verts[i].x, verts[i].y, verts[i].z);
	// DEBUG(1, "   ");
	// for (uint i = 0; i < totalindc; i += 9)
	// 	DEBUG(1, "[vxl%u] %u -> %u %u %u %u %u %u %u", i/9, inds[i], inds[i+1], inds[i+2], inds[i+3], inds[i+4], inds[i+5],
	// 	                                                           inds[i+6], inds[i+7]);

	free(inds);
	DEBUG(3, "[MAP] Generated mesh for map with %lu cells (%lu vertices in %lu blocks)", mapcellc, totalindc, mapblockc);
	// exit(0);
} 

static uint generate_block_indices(uint16* inds, uintptr start)
{
	uint blockx = map_get_block_x(start);
	uint blocky = map_get_block_y(start);
	uint blockz = map_get_block_z(start);
	// DEBUG(1, "Meshing %lu: %u %u %u", start, blockx, blocky, blockz);

	uint indc   = 0;
	uint rowc   =  MAP_BLOCK_WIDTH + 1;                           /* # of cells per row     */
	uint layerc = (MAP_BLOCK_WIDTH + 1) * (MAP_BLOCK_HEIGHT + 1); /* # of cells per z-level */
	uint vx, vy, vz;
	uintptr a = (uintptr)inds;
	for (uint z = 0; z < MAP_BLOCK_DEPTH; z++) {
		for (uint y = 0; y < MAP_BLOCK_HEIGHT; y++) {
			for (uint x = 0; x < MAP_BLOCK_WIDTH; x++) {
				// DEBUG(1, "%ux%ux%u", blockx + x, blocky + y, blockz + z);
				if (blockx + x >= map->dim[0] || blocky + y >= map->dim[1] || blockz + z >= map->dim[2]) 
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

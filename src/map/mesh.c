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

void generate_meshes()
{
	uint cellc  = volume_of(map->dim);
	uint blockc = ((map->dim.w + MAP_BLOCK_WIDTH  - 1) / MAP_BLOCK_WIDTH ) * 
	              ((map->dim.h + MAP_BLOCK_HEIGHT - 1) / MAP_BLOCK_HEIGHT) * 
	              ((map->dim.d + MAP_BLOCK_DEPTH  - 1) / MAP_BLOCK_DEPTH );
	DEBUG(1, "blockdim: %ux%ux%u", ((map->dim.w + MAP_BLOCK_WIDTH  - 1) / MAP_BLOCK_WIDTH ),
	      ((map->dim.h + MAP_BLOCK_HEIGHT - 1) / MAP_BLOCK_HEIGHT),
	      ((map->dim.d + MAP_BLOCK_DEPTH  - 1) / MAP_BLOCK_DEPTH ));
	DEBUG(1, "blockc: %u | cellc: %u", blockc, cellc);

	map->inds = scalloc(blockc, sizeof(struct MapBlockIndices));

	/* Vertices */
	uint vertc = (MAP_BLOCK_DEPTH + 1) * (MAP_BLOCK_HEIGHT + 1) * (MAP_BLOCK_WIDTH + 1);
	struct Vertex* v = verts;
	for (uint z = 0; z <= MAP_BLOCK_DEPTH; z++)
		for (uint y = 0; y <= MAP_BLOCK_HEIGHT; y++)
			for (uint x = 0; x <= MAP_BLOCK_WIDTH; x++)
				*v++ = (struct Vertex){ .x = x, .y = y, .z = z, .a = 1 };
	map->verts = create_vbo(vertc*sizeof(struct Vertex), verts);

	/* Indices */
	uintptr indc      = 0;
	uintptr totalindc = 0;
	uint16* inds = smalloc(MAP_CELLS_PER_BLOCK * MAP_INDICES_PER_VXL * sizeof(uint16));
	uintptr currblock = 0;
	for (uint z = 0; z < MAX((map->dim.d + MAP_BLOCK_DEPTH - 1) / MAP_BLOCK_DEPTH, 1); z++) {
		for (uint y = 0; y < MAX((map->dim.h + MAP_BLOCK_HEIGHT - 1) / MAP_BLOCK_HEIGHT, 1); y++) {
			for (uint x = 0; x < MAX((map->dim.w + MAP_BLOCK_WIDTH - 1) / MAP_BLOCK_WIDTH, 1); x++) {
				indc = generate_block_indices(inds, currblock++);
				totalindc += indc;
				map->inds[map->indc  ].ibo = create_ibo(indc*sizeof(uint16), inds);
				map->inds[map->indc++].indc = indc;
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
	DEBUG(3, "[MAP] Generated mesh for map with %u cells (%lu vertices in %u blocks)", cellc, totalindc, blockc);
	// exit(0);
} 

static uint generate_block_indices(uint16* inds, uintptr start)
{
	uint blockx = (start % DIV_CEIL(map->dim.w, MAP_BLOCK_WIDTH))  * MAP_BLOCK_WIDTH;
	uint blocky = (start / DIV_CEIL(map->dim.w, MAP_BLOCK_HEIGHT)) * MAP_BLOCK_HEIGHT;
	uint blockz = 0;
	DEBUG(1, "start %lu: %u %u %u", start, blockx, blocky, blockz);

	uint indc   = 0;
	uint rowc   =  MAP_BLOCK_WIDTH + 1;                           /* # of cells per row     */
	uint layerc = (MAP_BLOCK_WIDTH + 1) * (MAP_BLOCK_HEIGHT + 1); /* # of cells per z-level */
	uint vx, vy, vz;
	uintptr a = (uintptr)inds;
	for (uint z = 0; z < MAP_BLOCK_DEPTH; z++) {
		for (uint y = 0; y < MAP_BLOCK_HEIGHT; y++) {
			for (uint x = 0; x < MAP_BLOCK_WIDTH; x++) {
				// DEBUG(1, "%ux%ux%u", blockx + x, blocky + y, blockz + z);
				if (blockx + x >= map->dim.w || blocky + y >= map->dim.h || blockz + z >= map->dim.d)
					continue;
				// if (!map->data[indc/MAP_INDICES_PER_VXL].data)
					// continue;
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

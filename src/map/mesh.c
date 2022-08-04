#include "common.h"
#include "gfx/vertices.h"
#include "map.h"

struct Vertex {
	uint32 z: 10;
	uint32 y: 10;
	uint32 x: 10;
	uint32 a: 2;
}; static_assert(sizeof(struct Vertex) == 4, "struct Vertex");
static struct Vertex verts[(MAP_BLOCK_WIDTH + 1)*(MAP_BLOCK_HEIGHT + 1)*(MAP_BLOCK_DEPTH + 1)];

void generate_meshes()
{
	uint cellc  = volume_of(map->dim);
	uint blockc = ((map->dim.w + MAP_BLOCK_WIDTH  - 1) / MAP_BLOCK_WIDTH ) * 
	              ((map->dim.h + MAP_BLOCK_HEIGHT - 1) / MAP_BLOCK_HEIGHT) * 
	              ((map->dim.d + MAP_BLOCK_DEPTH  - 1) / MAP_BLOCK_DEPTH );
	DEBUG(1, "%u %u %u", ((map->dim.w + MAP_BLOCK_WIDTH  - 1) / MAP_BLOCK_WIDTH ),
	      ((map->dim.h + MAP_BLOCK_HEIGHT - 1) / MAP_BLOCK_HEIGHT),
	      ((map->dim.d + MAP_BLOCK_DEPTH  - 1) / MAP_BLOCK_DEPTH ));
	DEBUG(1, "blockc: %u | cellc: %u", blockc, cellc);

	/* Vertices */
	uint vertc = (MAP_BLOCK_DEPTH + 1) * (MAP_BLOCK_HEIGHT + 1) * (MAP_BLOCK_WIDTH + 1);
	struct Vertex* v = verts;
	for (uint z = 0; z <= MAP_BLOCK_DEPTH; z++)
		for (uint y = 0; y <= MAP_BLOCK_HEIGHT; y++)
			for (uint x = 0; x <= MAP_BLOCK_WIDTH; x++)
				*v++ = (struct Vertex){ .x = x, .y = y, .z = z, .a = 1 };

	/* Indices */
	uint indsc[blockc]; /* For the number of indices used in each block */
	memset(indsc, 0, sizeof(indsc));
	uint totalindc = 0; /* Indices for the whole map */
	uint16* inds     = scalloc(cellc, sizeof(uint16[MAP_VERTS_PER_VXL]));
	uint16* indstart = inds;
	uint rowc   =  MAP_BLOCK_WIDTH + 1;                           /* # of cells per row     */
	uint layerc = (MAP_BLOCK_WIDTH + 1) * (MAP_BLOCK_HEIGHT + 1); /* # of cells per z-level */
	for (uint z = 0; z < map->dim.d; z++) {
		for (uint y = 0; y < map->dim.h; y++) {
			for (uint x = 0; x < map->dim.w; x++) {
				// if (!map->data[indsc].data)
					// continue;
				/* 2--3   2
				 * | /  / |
				 * 1   1--3
				 */
				/* Top */
				*inds++ = x     +       y*rowc + z*layerc;
				*inds++ = x     + (y + 1)*rowc + z*layerc;
				*inds++ = x + 1 + (y + 1)*rowc + z*layerc;

				*inds++ = x     +       y*rowc + z*layerc;
				*inds++ = x + 1 + (y + 1)*rowc + z*layerc;
				*inds++ = x + 1 +       y*rowc + z*layerc;

				/* Left */
				*inds++ = x + (y + 1)*rowc + (z + 1)*layerc;
				*inds++ = x + (y + 1)*rowc +       z*layerc;
				*inds++ = x +       y*rowc + (z + 1)*layerc;

				*inds++ = x +       y*rowc +       z*layerc;
				*inds++ = x +       y*rowc + (z + 1)*layerc;
				*inds++ = x + (y + 1)*rowc +       z*layerc;

				/* Right */
				*inds++ = x     + y*rowc + (z + 1)*layerc;
				*inds++ = x     + y*rowc +       z*layerc;
				*inds++ = x + 1 + y*rowc +       z*layerc;

				*inds++ = x     + y*rowc + (z + 1)*layerc;
				*inds++ = x + 1 + y*rowc +       z*layerc;
				*inds++ = x + 1 + y*rowc + (z + 1)*layerc;

				totalindc += MAP_VERTS_PER_VXL;
				indsc[(totalindc - 1) / (MAP_CELLS_PER_BLOCK*MAP_VERTS_PER_VXL)] += MAP_VERTS_PER_VXL;
				// DEBUG(1, "--> %u (%u / %u)", (totalindc - 1) / (MAP_CELLS_PER_BLOCK*MAP_VERTS_PER_VXL),
					// (totalindc - 1), (MAP_CELLS_PER_BLOCK*MAP_VERTS_PER_VXL));
			}
		}
	}

	for (uint q = 0; q < blockc; q++)
		DEBUG(1, "[%u] %u", q, indsc[q]);
	// for (uint v = 0; v < vertc; v++)
	// 	DEBUG(1, "[v%u] %u %u %u", v, verts[v].x, verts[v].y, verts[v].z);
	// DEBUG(1, "   ");
	// for (uint v = 0; v < indsc; v += 3)
		// DEBUG(1, "[t%u] %u %u %u", v/3, indstart[v], indstart[v+1], indstart[v+2]);
	map->verts = create_vbo(vertc*sizeof(struct Vertex), verts);

	map->inds = scalloc(blockc, sizeof(struct MapBlockIndices));
	map->indc = blockc;
	DEBUG(1, "\t * inds size: %lu (%u cells)", cellc * sizeof(uint16[MAP_VERTS_PER_VXL]), cellc);
	inds = indstart; /* Reuse this to step through the indices for buffer creation */
	for (uint i = 0; i < blockc; i++) {
		DEBUG(1, "[%u] ibo from %lu of size %lu (%u inds)", i, i*MAP_CELLS_PER_BLOCK*sizeof(uint16[MAP_VERTS_PER_VXL]),
			MAP_CELLS_PER_BLOCK*sizeof(uint16[MAP_VERTS_PER_VXL]), MAP_CELLS_PER_BLOCK*MAP_VERTS_PER_VXL);
		DEBUG(1, "indc: %u", indsc[i]);
		map->inds[i].ibo = create_ibo(indsc[i]*sizeof(uint16[MAP_VERTS_PER_VXL]), inds);
		map->inds[i].indc = indsc[i];
		inds += indsc[i];
	}

	free(indstart);
	DEBUG(3, "[MAP] Generated mesh for map with %u cells (%u vertices)", cellc, totalindc);
	// exit(0);
} 

#include "common.h"
#include "gfx/vertices.h"
#include "map.h"

struct Vertex {
	int32 z: 10;
	int32 y: 10;
	int32 x: 10;
	int32 a: 2;
}; static_assert(sizeof(struct Vertex) == 4, "struct Vertex");

static int generate_block_indices(uint16* inds, int start);

static struct Vertex verts[MAP_VERTEX_COUNT];

void map_generate_meshes()
{
	/* Vertices */
	int vertc = MAP_VERTEX_COUNT;
	static_assert(MAP_VERTEX_COUNT < UINT16_MAX);

	struct Vertex* v = verts; 
	for (int z = 0; z < MAP_BLOCK_DEPTH + 1; z++)
		for (int y = 0; y < MAP_BLOCK_HEIGHT + 1; y++)
			for (int x = 0; x < MAP_BLOCK_WIDTH + 1; x++)
				*v++ = (struct Vertex){ .x = x, .y = y, .z = z, .a = 1 };
	map.verts = vbo_new(vertc*sizeof(struct Vertex), verts);

	/* Indices */
	int totalindc = 0;
	int blockindc = 0;
	uint16* inds  = smalloc(MAP_CELLS_PER_BLOCK*MAP_INDICES_PER_VXL*sizeof(uint16));
	int currblock = 0;
	for (int z = 0; z < map.bd; z++) {
		for (int y = 0; y < map.bh; y++) {
			for (int x = 0; x < map.bw; x++) {
				if (!(blockindc = generate_block_indices(inds, currblock))) {
					currblock++;
					continue;
				}
				totalindc += blockindc;
				map.ibos[currblock]  = ibo_new(blockindc*sizeof(uint16), inds);
				map.indcs[currblock] = blockindc;
				currblock++;
			}
		}
	}

	free(inds);
	DEBUG(3, "[MAP] Generated mesh for map with %d cells (%d vertices in %d blocks)", map.cellc, totalindc, map.blockc);
} 

static int generate_block_indices(uint16* inds, int block)
{
	int globaly = map_get_block_start_y(block);
	int globalz = map_get_block_start_z(block);
	int globalx = map_get_block_start_x(block);
	
	int indc = 0;
	int w    =  MAP_BLOCK_WIDTH + 1;
	int d    = (MAP_BLOCK_WIDTH + 1)*(MAP_BLOCK_HEIGHT + 1);
	for (int z = 0; z < MAP_BLOCK_DEPTH; z++) {
		for (int y = 0; y < MAP_BLOCK_HEIGHT; y++) {
			for (int x = 0; x < MAP_BLOCK_WIDTH; x++) {
				if (!map_is_cell(globalx + x, globaly + y, globalz + z) ||
				    !map_is_visible(globalx + x, globaly + y, globalz + z))
					continue;
				indc += MAP_INDICES_PER_VXL;

				/* *---*---*
				 * |1\2|3/4|
				 * *---*---*
				 * |5/6|
				 * *---*
				 */
				/* 1 */
				*inds++ = x + 1 + (y + 1)*w + z*d; /* Bottom-right */
				*inds++ = x     +       y*w + z*d; /* Top-left     */
				*inds++ = x     + (y + 1)*w + z*d; /* Bottom-left  */
				/* 2 */
				*inds++ = x + 1 +       y*w + z*d; /* Top-right    */
				*inds++ = x     +       y*w + z*d; /* Top-left     */
				*inds++ = x + 1 + (y + 1)*w + z*d; /* Bottom-right */
				/* 3 */
				*inds++ = x + (y + 1)*w +       z*d; /* Bottom-left  */
				*inds++ = x +       y*w +       z*d; /* Top-right    */
				*inds++ = x +       y*w + (z + 1)*d; /* Lower-bottom */
				/* 4 */
				*inds++ = x + (y + 1)*w +       z*d; /* Bottom-right */
				*inds++ = x +       y*w + (z + 1)*d; /* Lower-top    */
				*inds++ = x + (y + 1)*w + (z + 1)*d; /* Lower-bottom */
				/* 5 */
				*inds++ = x     + y*w +       z*d; /* Bottom-left  */
				*inds++ = x + 1 + y*w +       z*d; /* Bottom-right */
				*inds++ = x     + y*w + (z + 1)*d; /* Lower-left   */
				/* 6 */
				*inds++ = x + 1 + y*w +       z*d; /* Bottom-right */
				*inds++ = x + 1 + y*w + (z + 1)*d; /* Lower-right  */
				*inds++ = x     + y*w + (z + 1)*d; /* Lower-left   */
			}
		}
	}

	return indc;
}


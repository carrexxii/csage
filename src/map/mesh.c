#include "common.h"
#include "gfx/vertices.h"
#include "map.h"

#define VERTS_PER_VXL (3*6)
#define VERT(_x, _y, _z) ((struct Vertex){ .x = (float)(_x), .y = (float)(_y), .z = (float)(_z) })

struct Vertex {
	float x, y, z;
}; static_assert(sizeof(struct Vertex) == 12, "struct Vertex");

struct Model* generate_meshes(struct Map* map)
{
	uint blockc     = volume_of(map->dim);
	uint blockcellc = volume_of(map->blockdim);

	struct Model*  meshes    = scalloc(blockc, sizeof(struct Model));
	struct Vertex* verts     = scalloc(blockc * blockcellc * VERTS_PER_VXL, SIZEOF_VXL_VERT);
	struct Vertex* vertstart = verts;

	uint8 x = 0;
	uint8 y = 0;
	uint8 z = 0;
	uint vertc;
	struct MapCell* cell;
	struct MapCell* block = map->data;
	for (uint i = 0; i < blockc; i++) {
		cell  = block;
		vertc = 0;
		meshes[i].verts = (float*)verts;
		for (uint vxl = 0; vxl < blockcellc; vxl++) {
			if (!cell++)
				continue;
			vertc += VERTS_PER_VXL;
			/* Left */
			*verts++ = VERT(x, y    , z    );
			*verts++ = VERT(x, y    , z + 1);
			*verts++ = VERT(x, y + 1, z    );

			*verts++ = VERT(x, y + 1, z    );
			*verts++ = VERT(x, y    , z + 1);
			*verts++ = VERT(x, y + 1, z + 1);

			/* Right */
			*verts++ = VERT(x    , y, z    );
			*verts++ = VERT(x + 1, y, z    );
			*verts++ = VERT(x    , y, z + 1);

			*verts++ = VERT(x + 1, y, z    );
			*verts++ = VERT(x + 1, y, z + 1);
			*verts++ = VERT(x    , y, z + 1);

			/* Top */
			*verts++ = VERT(x    , y    , z);
			*verts++ = VERT(x    , y + 1, z);
			*verts++ = VERT(x + 1, y    , z);

			*verts++ = VERT(x + 1, y    , z);
			*verts++ = VERT(x    , y + 1, z);
			*verts++ = VERT(x + 1, y + 1, z);

			x++;
			if (x == map->blockdim.w) y++;
			if (y == map->blockdim.h) z++;
			y %= map->blockdim.h;
			x %= map->blockdim.w;
		}
		block += volume_of(map->blockdim);
		meshes[i].vertc = vertc;
	}

	map->meshc = blockc;
	for (uint i = 0; i < blockc; i++)
		meshes[i].vbo = create_vbo(meshes[i].vertc * SIZEOF_VXL_VERT, meshes[i].verts);

	free(vertstart);
	DEBUG(3, "[MAP] Generated mesh for map with %u meshes (%u vertices)", blockc, blockcellc*VERTS_PER_VXL);
	return meshes;
}

#include "maths/maths.h"
#include "body.h"

void bodies_update(int bodyc, struct Body* bodies)
{
	// Vec3i v;
	struct Body* body = bodies;
	for (int i = 0; i < bodyc; body++, i++) {
		// if (body->moving)
			// body->pos = add(body->pos, multiply(body->facing, body->vel));

		// v = ivec3s_of_vec3s(body->pos);
		// v.z += 1;
		// struct Voxel* vxl_below = map_get_voxel(v);
		// if (vxl_below && vxl_below->data == 0)
		// 	body->pos.z += G;
	}
}

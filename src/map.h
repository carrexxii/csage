#ifndef MAP_H
#define MAP_H

#include "gfx/vulkan.h"
#include "gfx/buffers.h"
#include "gfx/pipeline.h"
#include "gfx/texture.h"
#include "gfx/renderer.h"
#include "maths/types.h"

#define MAP_MAX_LAYERS       8
#define MAP_MAX_SPOT_LIGHTS  64
#define MAP_MAX_POINT_LIGHTS 256

typedef uint16 MapTile;

struct MapLayer {
	char name[32];
	int x, y, w, h;
	struct Sprite** sprites;
	MapTile data[];
};

struct Map {
	int w, h;
	struct SpriteSheet* sprite_sheet;
	int layerc, spot_lightc, point_lightc;
	struct MapLayer*   layers[MAP_MAX_LAYERS];
	struct SpotLight*  spot_lights;
	struct PointLight* point_lights;

	UBO ubo;
	SBO spot_lights_sbo;
	SBO point_lights_sbo;
	struct Pipeline pipeln;
};

void    maps_init(void);
void    map_new(struct Map* map, const char* name);
MapTile map_get_tile(struct Map* map, Vec3i pos);
Vec3    map_get_mouse_coords(struct Camera* cam);
Vec3    map_to_iso(Vec3 pos);
void    map_record_commands(VkCommandBuffer cmd_buf, struct Camera* cam, struct Map* map);
void    map_free(struct Map* map);

extern SBO spot_lights_sbo;
extern SBO point_lights_sbo;

#endif

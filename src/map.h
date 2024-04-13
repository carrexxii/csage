#ifndef MAP_H
#define MAP_H

#include "common.h"
#include "gfx/vulkan.h"
#include "gfx/buffers.h"
#include "gfx/pipeline.h"
#include "gfx/renderer.h"
#include "gfx/sprite.h"
#include "maths/types.h"

#define MAP_MAX_LAYERS       8
#define MAP_MAX_SPOT_LIGHTS  64
#define MAP_MAX_POINT_LIGHTS 256

typedef uint16 MapTile;

typedef struct MapLayer {
	char name[32];
	int x, y, w, h;
	Sprite** sprites;
	MapTile data[];
} MapLayer;

typedef struct Map {
	int w, h;
	SpriteSheet* sprite_sheet;
	int layerc, spot_lightc, point_lightc;
	MapLayer*   layers[MAP_MAX_LAYERS];
	SpotLight*  spot_lights;
	PointLight* point_lights;

	SBO spot_lights_sbo;
	SBO point_lights_sbo;
	Pipeline pipeln;
} Map;

void    maps_init(void);
void    map_new(Map* restrict map, const char* restrict name);
MapTile map_get_tile(Map* map, Vec3i pos);
Vec3    map_get_mouse_coords(Camera* cam);
Vec3    map_to_iso(Vec3 pos);
void    map_record_commands(VkCommandBuffer cmd_buf, Camera* restrict cam, Map* restrict map);
void    map_free(Map* map);

extern SBO global_spot_lights_sbo;
extern SBO global_point_lights_sbo;

#endif


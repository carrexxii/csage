#include "lua.h"
#include "gfx/buffers.h"
#include "gfx/renderer.h"
#include "gfx/sprite.h"
#include "map.h"

static void map_print(struct Map* map);

SBO global_spot_lights_sbo;
SBO global_point_lights_sbo;

void maps_init()
{

}

void map_new(struct Map* map, const char* name)
{
	char path[PATH_BUFFER_SIZE];
	snprintf(path, PATH_BUFFER_SIZE, MAP_PATH "/%s.lua", name);
	lua_getglobal(lua_state, "load_tiled_map");
	lua_pushstring(lua_state, path);
	if (lua_pcall(lua_state, 1, 1, 0)) {
		ERROR("[LUA] Failed in call to \"load_tiled_map\": \n\t%s", lua_tostring(lua_state, -1));
		return;
	}
	struct Map* map_data = lua_topointer(lua_state, -1);
	lua_pop(lua_state, 1);
	*map = *map_data;

	/*** -------------------- Load the Lights -------------------- ***/
	map->spot_lights  = smalloc(map->spot_lightc  * sizeof(struct SpotLight));
	map->point_lights = smalloc(map->point_lightc * sizeof(struct PointLight));
	memcpy(map->spot_lights , map_data->spot_lights , map->spot_lightc  * sizeof(struct SpotLight));
	memcpy(map->point_lights, map_data->point_lights, map->point_lightc * sizeof(struct PointLight));

	isize offset = sizeof(Vec4);
	map->spot_lights_sbo  = sbo_new(offset + map->spot_lightc  * sizeof(struct SpotLight));
	map->point_lights_sbo = sbo_new(offset + map->point_lightc * sizeof(struct PointLight));
	buffer_update(map->spot_lights_sbo , sizeof(map->spot_lightc) , &map->spot_lightc , 0);
	buffer_update(map->point_lights_sbo, sizeof(map->point_lightc), &map->point_lightc, 0);
	buffer_update(map->spot_lights_sbo , map->spot_lightc*sizeof(struct SpotLight)  , map->spot_lights , offset);
	buffer_update(map->point_lights_sbo, map->point_lightc*sizeof(struct PointLight), map->point_lights, offset);
	global_spot_lights_sbo  = map->spot_lights_sbo;
	global_point_lights_sbo = map->point_lights_sbo;

	/*** -------------------- Load the Sprite Sheet -------------------- ***/
	snprintf(path, PATH_BUFFER_SIZE, SPRITE_PATH "/%s.lua", "tiles");
	lua_getglobal(lua_state, "load_sprite_sheet");
	lua_pushstring(lua_state, path);
	if (lua_pcall(lua_state, 1, 1, 0)) {
		ERROR("[LUA] Failed in call to \"load_sprite_sheet\": \n\t%s", lua_tostring(lua_state, -1));
		sfree(map->spot_lights);
		sfree(map->point_lights);
		return;
	}
	map->sprite_sheet = sprite_sheet_load(lua_topointer(lua_state, -1));
	lua_pop(lua_state, 1);

	/*** -------------------- Build the Map -------------------- ***/
	Vec2* tiles = smalloc(map->w * map->h * sizeof(Vec2));
	enum SpriteStateType* tile_states = smalloc(map->w * map->h * sizeof(enum SpriteStateType));
	struct MapLayer* layer;
	for (int i = 0; i < map_data->layerc; i++) {
		layer = map_data->layers[i];
		map->layers[i] = smalloc(sizeof(struct MapLayer) + layer->w*layer->h*sizeof(MapTile));
		memcpy(map->layers[i], layer, sizeof(struct MapLayer) + layer->w*layer->h*sizeof(MapTile));

		int spritec = 0;
		for (int j = 0; j < layer->w*layer->h; j++)
			if (layer->data[j])
				spritec++;
		map->layers[i]->sprites = smalloc(spritec * sizeof(struct Sprite*));

		isize tilec = 0;
		for (int y = 0; y < layer->h; y++) {
			for (int x = 0; x < layer->w; x++) {
				if (layer->data[y*layer->w + x]) {
					tiles[tilec] = VEC2(x, y);
					tile_states[tilec] = layer->data[y*layer->w + x] - 1;
					tilec++;
				}
			}
		}
		sprite_new_batch(map->sprite_sheet, sprites_get_group(map->sprite_sheet, "tiles"), tilec, tiles, tile_states);
	}

	sfree(tiles);
	sfree(tile_states);
	DEBUG(1, "[MAP] Created new map \"%s\" (%dx%d) with %d layers, %d spot lights, %d point_lights",
	      name, map->w, map->h, map->layerc, map->spot_lightc, map->point_lightc);
}

noreturn MapTile map_get_tile(struct Map* map, Vec3i pos)
{
	(void)map;
	(void)pos;
	assert(false);
}

void map_free(struct Map* map)
{
	for (int i = 0; i < map->layerc; i++) {
		sfree(map->layers[i]->sprites);
		sfree(map->layers[i]);
	}

	sfree(map->spot_lights);
	sfree(map->point_lights);
	sbo_free(&map->spot_lights_sbo);
	sbo_free(&map->point_lights_sbo);

	global_spot_lights_sbo  = (SBO){ 0 };
	global_point_lights_sbo = (SBO){ 0 };
}

/* -------------------------------------------------------------------- */

static void map_print(struct Map* map)
{
	struct MapLayer* layer;
	DEBUG(1, "[MAP] Map %dx%d (%d layers, %d spot lights, %d point lights)",
	      map->w, map->h, map->layerc, map->spot_lightc, map->point_lightc);
	for (int i = 0; i < map->spot_lightc; i++)
		DEBUG(1, "\tSpotlight %d at %.2f, %.2f, %.2f (%.2f-%.2f-%.2f | %.2f-%.2f)",
		      i, map->spot_lights[i].pos.x, map->spot_lights[i].pos.y, map->spot_lights[i].pos.z,
		      map->spot_lights[i].constant, map->spot_lights[i].linear, map->spot_lights[i].quadratic,
		      map->spot_lights[i].cutoff, map->spot_lights[i].outer_cutoff);
	for (int i = 0; i < map->point_lightc; i++)
		DEBUG(1, "\tPointlight %d at %.2f, %.2f, %.2f", i, map->point_lights[i].pos.x, map->point_lights[i].pos.y, map->point_lights[i].pos.z);
	for (int i = 0; i < map->layerc; i++) {
		layer = map->layers[i];
		DEBUG(1, "\tLayer \"%s\" (%dx%d) at %d, %d", layer->name, layer->w, layer->h, layer->x, layer->y);
		for (int y = 0; y < layer->h; y++) {
			for (int x = 0; x < layer->w; x++)
				fprintf(stderr, "%d, ", layer->data[y*layer->w + x]);
			fprintf(stderr, "\n");
		}
	}
}

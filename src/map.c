#include "lua.h"
#include "gfx/buffers.h"
#include "gfx/renderer.h"
#include "gfx/sprite.h"
#include "map.h"

static void map_print(Map* map);

SBO global_spot_lights_sbo;
SBO global_point_lights_sbo;

void maps_init()
{

}

void map_new(Map* map, const char* name)
{
	char path[PATH_BUFFER_SIZE];
	snprintf(path, PATH_BUFFER_SIZE, MAP_PATH "/%s.lua", name);
	lua_getglobal(lua_state, "load_tiled_map");
	lua_pushstring(lua_state, path);
	if (lua_pcall(lua_state, 1, 1, 0)) {
		ERROR("[LUA] Failed in call to \"load_tiled_map\": \n\t%s", lua_tostring(lua_state, -1));
		return;
	}
	const Map* map_data = lua_topointer(lua_state, -1);
	lua_pop(lua_state, 1);
	*map = *map_data;

	/*** -------------------- Load the Lights -------------------- ***/
	map->spot_lights  = smalloc(map->spot_lightc  * sizeof(SpotLight));
	map->point_lights = smalloc(map->point_lightc * sizeof(PointLight));
	memcpy(map->spot_lights , map_data->spot_lights , map->spot_lightc  * sizeof(SpotLight));
	memcpy(map->point_lights, map_data->point_lights, map->point_lightc * sizeof(PointLight));

	isize offset = sizeof(Vec4);
	map->spot_lights_sbo  = sbo_new(offset + map->spot_lightc  * sizeof(SpotLight));
	map->point_lights_sbo = sbo_new(offset + map->point_lightc * sizeof(PointLight));
	buffer_update(map->spot_lights_sbo , sizeof(map->spot_lightc) , &map->spot_lightc , 0);
	buffer_update(map->point_lights_sbo, sizeof(map->point_lightc), &map->point_lightc, 0);
	buffer_update(map->spot_lights_sbo , map->spot_lightc*sizeof(SpotLight)  , map->spot_lights , offset);
	buffer_update(map->point_lights_sbo, map->point_lightc*sizeof(PointLight), map->point_lights, offset);
	global_spot_lights_sbo  = map->spot_lights_sbo;
	global_point_lights_sbo = map->point_lights_sbo;

	/*** -------------------- Load the Sprite Sheet -------------------- ***/
	// snprintf(path, PATH_BUFFER_SIZE, SPRITE_PATH "/%s.lua", "tiles");
	// lua_getglobal(lua_state, "load_sprite_sheet");
	// lua_pushstring(lua_state, path);
	// if (lua_pcall(lua_state, 1, 1, 0)) {
	// 	ERROR("[LUA] Failed in call to \"load_sprite_sheet\": \n\t%s", lua_tostring(lua_state, -1));
	// 	sfree(map->spot_lights);
	// 	sfree(map->point_lights);
	// 	return;
	// }
	// map->sprite_sheet = sprite_sheet_load(lua_topointer(lua_state, -1));
	// lua_pop(lua_state, 1);
	map->sprite_sheet = sprite_sheet_new("tiles", -1);

	/*** -------------------- Build the Map -------------------- ***/
	Vec2* tiles = smalloc(map->w * map->h * sizeof(Vec2));
	// char* tile_names = smalloc(map->w * map->h * sizeof(char[64]));
	MapLayer* layer;
	for (int i = 0; i < map_data->layerc; i++) {
		layer = map_data->layers[i];
		map->layers[i] = smalloc(sizeof(MapLayer) + layer->w*layer->h*sizeof(MapTile));
		memcpy(map->layers[i], layer, sizeof(MapLayer) + layer->w*layer->h*sizeof(MapTile));

		int spritec = 0;
		for (int j = 0; j < layer->w*layer->h; j++)
			if (layer->data[j])
				spritec++;
		map->layers[i]->sprites = smalloc(spritec * sizeof(Sprite*));

		isize tilec = 0;
		for (int y = 0; y < layer->h; y++) {
			for (int x = 0; x < layer->w; x++) {
				if (layer->data[y*layer->w + x]) {
					tiles[tilec] = VEC2(x, y);
					// tile_states[tilec] = layer->data[y*layer->w + x] - 1;
					tilec++;
				}
			}
		}
		Sprite* spr = sprite_new_batch(map->sprite_sheet, STRING("tiles-grass"), tilec, tiles);
		sprite_set_state(map->sprite_sheet, spr, STRING("tiles-dirt"), SPRITE_DIR_SW);
	}

	sfree(tiles);
	// sfree(tile_states);
	INFO(TERM_ORANGE "[MAP] Created new map \"%s\" (%dx%d) with %d layers, %d spot lights, %d point_lights",
	      name, map->w, map->h, map->layerc, map->spot_lightc, map->point_lightc);
}

MapTile map_get_tile(Map* map, Vec3i pos)
{
	(void)map;
	(void)pos;
	assert(false);
}

void map_free(Map* map)
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

static void map_print(Map* map)
{
	MapLayer* layer;
	INFO(TERM_ORANGE "[MAP] Map %dx%d (%d layers, %d spot lights, %d point lights)",
	      map->w, map->h, map->layerc, map->spot_lightc, map->point_lightc);
	for (int i = 0; i < map->spot_lightc; i++)
		INFO(TERM_ORANGE "\tSpotlight %d at %.2f, %.2f, %.2f (%.2f-%.2f-%.2f | %.2f-%.2f)",
		      i, map->spot_lights[i].pos.x, map->spot_lights[i].pos.y, map->spot_lights[i].pos.z,
		      map->spot_lights[i].constant, map->spot_lights[i].linear, map->spot_lights[i].quadratic,
		      map->spot_lights[i].cutoff, map->spot_lights[i].outer_cutoff);
	for (int i = 0; i < map->point_lightc; i++)
		INFO(TERM_ORANGE "\tPointlight %d at %.2f, %.2f, %.2f", i, map->point_lights[i].pos.x, map->point_lights[i].pos.y, map->point_lights[i].pos.z);
	for (int i = 0; i < map->layerc; i++) {
		layer = map->layers[i];
		INFO(TERM_ORANGE "\tLayer \"%s\" (%dx%d) at %d, %d", layer->name, layer->w, layer->h, layer->x, layer->y);
		for (int y = 0; y < layer->h; y++) {
			for (int x = 0; x < layer->w; x++)
				fprintf(stderr, "%d, ", layer->data[y*layer->w + x]);
			fprintf(stderr, "\n");
		}
	}
}


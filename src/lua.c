#include "lua.h"

lua_State* lua_state;
struct GlobalConfig config;

void lua_init()
{
	DEBUG(1, "[INFO] LuaJIT version: " LUAJIT_VERSION);
	DEBUG(1, "[INFO] Lua    version: " LUA_VERSION);

	lua_state = luaL_newstate();
	luaL_openlibs(lua_state);

	if (lua_get_file(SCRIPT_PATH "/core/csage.lua"))  exit(1);
	if (lua_get_file(SCRIPT_PATH "/core/map.lua"))    exit(1);
	if (lua_get_file(SCRIPT_PATH "/core/sprite.lua")) exit(1);

	luaL_dofile(lua_state, "config.lua");
	config.winw      = lua_get_int("window_width");
	config.winh      = lua_get_int("window_height");
	config.cam_speed = lua_get_float("camera_speed");

	DEBUG(1, "[INFO] Initialized Lua");
	DEBUG(1, "[INFO] Config:");
	fprintf(stderr, "\twindow_width  -> %d\n", config.winw);
	fprintf(stderr, "\twindow_heigth -> %d\n", config.winh);
}

int lua_get_file(char* path)
{
	if (luaL_loadfile(lua_state, path) || lua_pcall(lua_state, 0, 0, 0)) {
		ERROR("[LUA] Failed to load \"%s\": \n\t%s", path, lua_tostring(lua_state, -1));
		return -1;
	}

	return 0;
}

int lua_get_int(char* name)
{
	lua_getglobal(lua_state, name);
	if (!lua_isnumber(lua_state, -1))
		ERROR("[LUA] Expected a number for \"%s\", got: \"%s\"", name, lua_tostring(lua_state, -1));

	int num = lua_tonumber(lua_state, -1);
	lua_pop(lua_state, 1);
	return num;
}

float lua_get_float(char* name)
{
	lua_getglobal(lua_state, name);
	if (!lua_isnumber(lua_state, -1))
		ERROR("[LUA] Expected a number for \"%s\", got: \"%s\"", name, lua_tostring(lua_state, -1));

	float num = lua_tonumber(lua_state, -1);
	lua_pop(lua_state, 1);
	return num;
}

void lua_free()
{
	lua_close(lua_state);
}

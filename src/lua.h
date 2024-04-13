#ifndef LUA_H
#define LUA_H

#include "common.h"
#include "luajit/luajit.h"
#include "luajit/lualib.h"
#include "luajit/lauxlib.h"

void        lua_init(void);
int         lua_get_file(const char* path);
int         lua_get_int(const char* name);
float       lua_get_float(const char* name);
const char* lua_get_string(const char* name);
const void* lua_get_pointer(const char* name);
void        lua_free(void);

extern lua_State* lua_state;

#endif


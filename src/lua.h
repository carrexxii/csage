#ifndef LUA_H
#define LUA_H

#include "luajit/luajit.h"
#include "luajit/lualib.h"
#include "luajit/lauxlib.h"

void  lua_init(void);
int   lua_get_file(char* path);
int   lua_get_int(char* name);
float lua_get_float(char* name);
char* lua_get_string(char* name);
void  lua_free(void);

extern lua_State* lua_state;

#endif

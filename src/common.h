#ifndef COMMON_H
#define COMMON_H

#include "clib/clib.h"

#define SHADER_PATH  "shaders/spirv"
#define FONT_PATH    "gfx/fonts"
#define MODEL_PATH   "gfx/models"
#define TEXTURE_PATH "gfx/textures"
#define SPRITE_PATH  "gfx/sprites"
#define SCRIPT_PATH  "scripts"
#define MAP_PATH     "maps"
#define LEVEL_PATH   "scripts/levels"

#define PATH_BUFFER_SIZE 256
#define FPS              50.0
#define DT               (1.0/FPS)
#define DT_MS            (DT*1000.0)
#define DEBUG_MALLOC_MIN (128*1024)

#define COLOUR_WHITE   1.0f, 1.0f, 1.0f
#define COLOUR_BLACK   0.0f, 0.0f, 0.0f
#define COLOUR_RED     1.0f, 0.0f, 0.0f
#define COLOUR_GREEN   0.0f, 1.0f, 0.0f
#define COLOUR_BLUE    0.0f, 0.0f, 1.0f
#define COLOUR_MAGENTA 1.0f, 0.0f, 1.0f
#define COLOUR_YELLOW  1.0f, 1.0f, 0.0f
#define COLOUR_CYAN    0.0f, 1.0f, 1.0f

#define ASPECT_RATIO ((float)config.winw / config.winh)

#define DEFAULT_LIGHT_AMBIENT   (uint8[]){ 255, 255, 255, 255 }
#define DEFAULT_LIGHT_DIFFUSE   (uint8[]){ 255, 255, 255, 255 }
#define DEFAULT_LIGHT_SPECULAR  (uint8[]){ 255, 255, 255, 255 }
#define DEFAULT_LIGHT_CONSTANT     1.0f
#define DEFAULT_LIGHT_LINEAR       0.5f
#define DEFAULT_LIGHT_QUADRATIC    3.5f
#define DEFAULT_LIGHT_CUTOFF       0.9f
#define DEFAULT_LIGHT_OUTER_CUTOFF 0.7f

extern struct GlobalConfig {
	int winw;
	int winh;
	float cam_speed;
	char* font_name;
} config;

[[noreturn]] void quit(void);

extern int vk_err;

#endif


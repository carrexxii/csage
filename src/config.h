#ifndef CONFIG_H
#define CONFIG_H

#define SHADER_DIR   "shaders/spirv/"
#define FONT_PATH    "gfx/fonts/"
#define MODEL_PATH   "gfx/models/"
#define TEXTURE_PATH "gfx/textures/"
#define SCRIPT_PATH  "scripts/"
#define MAP_PATH     "maps/"

#define ASPECT_RATIO ((float)global_config.winw / global_config.winh)

#define DEFAULT_LIGHT_AMBIENT   (uint8[]){ 255, 255, 255, 255 }
#define DEFAULT_LIGHT_DIFFUSE   (uint8[]){ 255, 255, 255, 255 }
#define DEFAULT_LIGHT_SPECULAR  (uint8[]){ 255, 255, 255, 255 }
#define DEFAULT_LIGHT_CONSTANT     1.0f
#define DEFAULT_LIGHT_LINEAR       0.5f
#define DEFAULT_LIGHT_QUADRATIC    3.5f
#define DEFAULT_LIGHT_CUTOFF       0.9f
#define DEFAULT_LIGHT_OUTER_CUTOFF 0.7f

struct GlobalConfig {
	int winw;
	int winh;
};

noreturn void quit();

extern struct GlobalConfig global_config;

#endif

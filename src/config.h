#ifndef CONFIG_H
#define CONFIG_H

#define SHADER_DIR   "shaders/spirv/"
#define FONT_PATH    "gfx/fonts/"
#define MODEL_PATH   "gfx/models/"
#define TEXTURE_PATH "gfx/textures/"
#define SCRIPT_PATH  "scripts/"

#define ASPECT_RATIO ((float)global_config.winw/global_config.winh)

struct GlobalConfig {
	int winw;
	int winh;
};

noreturn void quit();

extern struct GlobalConfig global_config;

#endif

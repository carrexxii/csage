#ifndef CONFIG_H
#define CONFIG_H

#define ASPECT_RATIO ((float)global_config.winw/global_config.winh)

struct GlobalConfig {
	int winw;
	int winh;
};

noreturn void quit();

extern struct GlobalConfig global_config;

#endif

#ifndef CONFIG_H
#define CONFIG_H

struct GlobalConfig {
	int winw;
	int winh;
};

noreturn void quit();

extern struct GlobalConfig global_config;

#endif

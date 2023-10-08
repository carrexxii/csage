#ifndef GFX_PARTICLES_H
#define GFX_PARTICLES_H

#include "common.h"
#include "vulkan/vulkan.h"

#include "pipeline.h"
#include "buffers.h"
#include <stdalign.h>

#define MAX_PARTICLE_POOLS     256
#define MAX_PARTICLES_PER_POOL 128

enum ParticleType {
	PARTICLE_STREAM,
	PARTICLE_END,
};

struct Particle {
	vec2  s, v;
	int32 life;
	int: 32;
	int: 32;
	int: 32;
}; static_assert(sizeof(struct Particle) == 32);

struct ParticlePool {
	bool   enabled;
	int32  life;
	int32  particle_life;
	int32  interval;
	int32  timer;
	float* start_pos;
	float* start_vel;
	struct {
		struct Particle particles[MAX_PARTICLES_PER_POOL];
		float scale;
	};
};

void particles_init(VkRenderPass renderpass);
int  particles_new_pool(int32 pool_life, int32 particle_life, int32 interval, float* start_pos, float* start_vel, float scale);
void particles_enable(int particle_id);
void particles_disable(int particle_id);
void particles_update();
void particles_record_commands(VkCommandBuffer cmd_buf);
void particles_free();

#endif

#ifndef GFX_PARTICLES_H
#define GFX_PARTICLES_H

#include "maths/maths.h"
#include "vulkan/vulkan.h"
#include "pipeline.h"
#include "buffers.h"
#include "camera.h"

#define MAX_PARTICLE_POOLS     256
#define MAX_PARTICLES_PER_POOL 128

enum ParticleType {
	PARTICLE_STREAM,
	PARTICLE_END,
};

struct Particle {
	Vec2  s, v;
	int32 life;
	int: 32;
	int: 32;
	int: 32;
}; static_assert(sizeof(struct Particle) == 32);

struct ParticlePool {
	VkDescriptorSet dset;
	bool   enabled;
	int32  life;
	int32  particle_life;
	int32  interval;
	int32  timer;
	Vec2   start_pos;
	Vec2   start_vel;

	struct Particle particles[MAX_PARTICLES_PER_POOL];
	float scale;
};

void particles_init(void);
ID   particles_new_pool(int32 pool_life, int32 particle_life, int32 interval, Vec2 start_pos, Vec2 start_vel, float scale);
void particles_enable(ID particle_id);
void particles_disable(ID particle_id);
void particles_update(void);
void particles_record_commands(VkCommandBuffer cmd_buf);
void particles_free(void);
void particles_free_pool(ID pool_id);

#endif

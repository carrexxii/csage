#ifndef GFX_PARTICLES_H
#define GFX_PARTICLES_H

#include <vulkan/vulkan.h>

#include "common.h"
#include "maths/maths.h"
#include "pipeline.h"
#include "buffers.h"

#define MAX_PARTICLE_POOLS     256
#define MAX_PARTICLES_PER_POOL 128

typedef enum ParticleType {
	PARTICLE_STREAM,
	PARTICLE_END,
} ParticleType;

typedef struct Particle {
	Vec2  s, v;
	int32 life;
	byte pad[12];
} Particle;

typedef struct ParticlePool {
	VkDescriptorSet dset;
	bool  enabled;
	int32 life;
	int32 particle_life;
	int32 interval;
	int32 timer;
	Vec2  start_pos;
	Vec2  start_vel;

	Particle particles[MAX_PARTICLES_PER_POOL];
	float    scale;
} ParticlePool;

void  particles_init(void);
isize particles_new_pool(int32 pool_life, int32 particle_life, int32 interval, Vec2 start_pos, Vec2 start_vel, float scale);
void  particles_enable(isize particle_id);
void  particles_disable(isize particle_id);
void  particles_update(void);
void  particles_record_commands(VkCommandBuffer cmd_buf);
void  particles_free(void);
void  particles_free_pool(isize pool_id);

#endif


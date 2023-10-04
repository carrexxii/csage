#ifndef GFX_PARTICLES_H
#define GFX_PARTICLES_H

#include "vulkan/vulkan.h"

#include "pipeline.h"
#include "buffers.h"

#define MAX_PARTICLES_PER_POOL 255

enum ParticleType {
	PARTICLE_STREAM,
	PARTICLE_END,
};

struct Particle {
	int64 life;
	vec2  s, v, a;
	float θ, ω, α;
	vec3  colour;
};

struct ParticlePool {
	struct Particle* particles;
	uint8 particlec;
	bool  enabled;
	int64 life;
	VBO   vbo;
};

extern struct Pipeline particlepipelns[PARTICLE_END];

void particles_init(VkRenderPass renderpass);
struct ParticlePool particles_new_pool(int vertc, float* verts, uint8 maxparticles, int64 poollife, int64 particlelife);

#endif

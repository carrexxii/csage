#include "vulkan/vulkan.h"

#include "vulkan.h"
#include "pipeline.h"
#include "particles.h"

#define SIZEOF_PARTICLE_VERTEX sizeof(float[5])

struct Pipeline pipelns[PARTICLE_END];

/* -------------------------------------------------------------------- */
static VkVertexInputBindingDescription streamvertbinds = {
	.binding   = 0,
	.stride    = SIZEOF_PARTICLE_VERTEX, /* xyrgb */
	.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
};
static VkVertexInputAttributeDescription streamvertattrs[] = {
	/* xy */
	{ .binding  = 0,
	  .location = 0,
	  .format   = VK_FORMAT_R32G32_SFLOAT,
	  .offset   = 0, },
	/* rgb */
	{ .binding  = 0,
	  .location = 1,
	  .format   = VK_FORMAT_R32G32B32_SFLOAT,
	  .offset   = sizeof(float[2]), },
};
/* -------------------------------------------------------------------- */

void particles_init(VkRenderPass renderpass)
{
	pipelns[PARTICLE_STREAM] = (struct Pipeline){
		.vshader   = create_shader(SHADER_DIR "stream.vert"),
		.fshader   = create_shader(SHADER_DIR "stream.frag"),
		.vertbindc = 1,
		.vertbinds = &streamvertbinds,
		.vertattrc = 2,
		.vertattrs = streamvertattrs,
		// .uboc      = 1,
		// .ubos      = scalloc(1, sizeof(UBO)),
		// .ubos[0]   = &ubobufs[0],
		// .sbo       = &matbuf,
		// .sbosz     = sizeof(mat4)*RENDERER_MAX_OBJECTS,
	};

	for (int i = 0; i < PARTICLE_END; i++)
		init_pipeln(pipelns + i, renderpass);
}

struct ParticlePool particles_new_pool(int vertc, float* verts, uint8 maxparticles, int64 poollife, int64 particlelife)
{
	struct ParticlePool ppool = {
		.vbo       = vbo_new(vertc*SIZEOF_PARTICLE_VERTEX, verts),
		.particles = scalloc(sizeof(struct Particle), maxparticles),
		.life      = poollife,
	};
	for (int i = 0; i < maxparticles; i++)
		ppool.particles[i].life = particlelife;

	return ppool;
}

void particles_free()
{
	for (int i = 0; i < PARTICLE_END; i++)
		pipeln_free(&pipelns[i]);
}

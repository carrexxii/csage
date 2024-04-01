#include <vulkan/vulkan.h>

#include "resmgr.h"
#include "maths/maths.h"
#include "vulkan.h"
#include "buffers.h"
#include "pipeline.h"
#include "renderer.h"
#include "camera.h"
#include "image.h"
#include "particles.h"

#define SIZEOF_PARTICLE_VERTEX sizeof(float[4])
#define PARTICLES_UBO_SIZE     sizeof(struct Particle)*MAX_PARTICLES_PER_POOL
#define PARTICLE_DAMPNER       0.99

static struct Pipeline* atomic pipeln;
static UBO ubo;
static VBO vbo_buf;
static struct Image* img;

static struct ParticlePool pools[MAX_PARTICLE_POOLS];
static int poolc;

/* -------------------------------------------------------------------- */
static VkVertexInputBindingDescription streamvert_binds[] = {
	{ .binding   = 0,
	  .stride    = SIZEOF_PARTICLE_VERTEX, /* xyuv */
	  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX, },
};
static VkVertexInputAttributeDescription streamvert_attrs[] = {
	/* xy */
	{ .binding  = 0,
	  .location = 0,
	  .format   = VK_FORMAT_R32G32_SFLOAT,
	  .offset   = 0, },
	/* uv */
	{ .binding  = 0,
	  .location = 1,
	  .format   = VK_FORMAT_R32G32_SFLOAT,
	  .offset   = sizeof(float[2]), },
};
/* -------------------------------------------------------------------- */

void particles_init()
{
	img = load_image(STRING(TEXTURE_PATH "/star.png"));
	// TODO: separate
	ubo = ubo_new(PARTICLES_UBO_SIZE);

	struct PipelineCreateInfo pipeln_ci = {
		.vshader     = STRING(SHADER_PATH "/particle.vert"),
		.fshader     = STRING(SHADER_PATH "/particle.frag"),
		.vert_bindc  = ARRAY_SIZE(streamvert_binds),
		.vert_binds  = streamvert_binds,
		.vert_attrc  = ARRAY_SIZE(streamvert_attrs),
		.vert_attrs  = streamvert_attrs,
		.push_stages = VK_SHADER_STAGE_VERTEX_BIT,
		.push_sz     = sizeof(float),
		.uboc        = 2,
		.ubos[0]     = global_camera_ubo,
		.ubos[1]     = ubo,
		.imgc        = 1,
		.imgs        = img,
	};
	pipeln = pipeln_new(&pipeln_ci, "Particles");
	pipeln = pipeln_update(pipeln, &pipeln_ci);

	float verts[] = {
		-0.5,  0.5, 0.0, 0.0,   0.5, 0.5, 1.0, 0.0,   -0.5, -0.5, 0.0, 1.0,
		-0.5, -0.5, 0.0, 1.0,   0.5, 0.5, 1.0, 0.0,    0.5, -0.5, 1.0, 1.0,
	};
	vbo_buf = vbo_new(sizeof(verts), verts, false);
}

ID particles_new_pool(int32 pool_life, int32 particle_life, int32 interval, Vec2 start_pos, Vec2 start_vel, float scale)
{
	if (particle_life/interval >= MAX_PARTICLES_PER_POOL)
		ERROR("[GFX] Potentially too many particles");
	pools[poolc] = (struct ParticlePool){
		.life          = pool_life,
		.scale         = scale,
		.particle_life = particle_life,
		.interval      = interval,
		.start_pos     = start_pos,
		.start_vel     = start_vel,
	};

	return poolc++;
}

void particles_enable(ID particle_id)
{
	pools[particle_id].enabled = true;
}

void particles_disable(ID particle_id)
{
	pools[particle_id].enabled = false;
}

void particles_update()
{
	int first_free;
	struct ParticlePool* pool;
	for (int i = 0; i < poolc; i++) {
		pool = &pools[i];
		if (pool->enabled) {
			if (pool->timer >= pool->interval) {
				first_free = 0;
				while (pool->particles[first_free].life > 0)
					first_free++;

				pool->timer -= pool->interval;
				pool->particles[first_free] = (struct Particle){
					.life = pool->particle_life,
					.v.x = pool->start_vel.x,
					.v.y = pool->start_vel.y,
					.s.x = pool->start_pos.x,
					.s.y = pool->start_pos.y,
				};
			} else {
				pool->timer += DT_MS;
			}
		}
		for (int j = 0; j < MAX_PARTICLES_PER_POOL; j++) {
			if (pool->particles[j].life <= 0)
				continue;
			pool->particles[j].v.x *= PARTICLE_DAMPNER;
			pool->particles[j].v.y *= PARTICLE_DAMPNER;
			pool->particles[j].s.x += pool->particles[j].v.x*DT;
			pool->particles[j].s.y += pool->particles[j].v.y*DT;
			pool->particles[j].life -= DT_MS;
		}
	}
}

void particles_record_commands(VkCommandBuffer cmd_buf)
{
	struct Pipeline* pl = pipeln;
	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pl->pipeln);
	vkCmdBindVertexBuffers(cmd_buf, 0, 1, &vbo_buf.buf, (VkDeviceSize[]) { 0 });
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pl->layout, 0, 1, &pl->dset.set, 0, NULL);

	struct ParticlePool* pool;
	for (int i = 0; i < poolc; i++) {
		pool = &pools[i];
		if (pool->life <= 0)
			continue;
		buffer_update(ubo, PARTICLES_UBO_SIZE, pool->particles, 0);

		vkCmdPushConstants(cmd_buf, pl->layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pool->scale), &pool->scale);
		vkCmdDraw(cmd_buf, 6, MAX_PARTICLES_PER_POOL, 0, MAX_PARTICLES_PER_POOL*i);
	}
}

void particles_free()
{
	vbo_free(&vbo_buf);
	ubo_free(&ubo);
	pipeln_free(pipeln);
}

void particles_free_pool(ID pool_id)
{
	if (pool_id >= MAX_PARTICLE_POOLS)
		ERROR("[GFX] Invalid particle pool id: %ld", pool_id);
	pools[pool_id].life = -1;
}

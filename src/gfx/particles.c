#include "common.h"
#include "vulkan/vulkan.h"

#include "vulkan.h"
#include "pipeline.h"
#include "camera.h"
#include "texture.h"
#include "particles.h"

#define SIZEOF_PARTICLE_VERTEX sizeof(float[4])
#define PARTICLES_UBO_SIZE     sizeof(struct Particle)*MAX_PARTICLES_PER_POOL
#define PARTICLE_DAMPNER       0.99

static struct Pipeline pipeln;
static UBO ubos[2];
static VBO vbo_buf;
static struct Texture texture;

static struct ParticlePool pools[MAX_PARTICLE_POOLS];
static int poolc;

/* -------------------------------------------------------------------- */
static VkVertexInputBindingDescription streamvertbinds = {
	.binding   = 0,
	.stride    = SIZEOF_PARTICLE_VERTEX, /* xyuv */
	.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
};
static VkVertexInputAttributeDescription streamvertattrs[] = {
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

void particles_init(VkRenderPass renderpass)
{
	texture = texture_new_from_image(TEXTURE_PATH "star.png");
	ubos[0] = ubo_new(sizeof(mat4));
	ubos[1] = ubo_new(PARTICLES_UBO_SIZE);
	pipeln = (struct Pipeline){
		.vshader    = create_shader(SHADER_DIR "particle.vert"),
		.fshader    = create_shader(SHADER_DIR "particle.frag"),
		.vertbindc  = 1,
		.vertbinds  = &streamvertbinds,
		.vertattrc  = 2,
		.vertattrs  = streamvertattrs,
		.pushstages = VK_SHADER_STAGE_VERTEX_BIT,
		.pushsz     = sizeof(float),
		.uboc       = 2,
		.ubos       = ubos,
		.texturec   = 1,
		.textures   = &texture,
	};

	pipeln_init(&pipeln, renderpass);

	float verts[] = {
		-0.5,  0.5, 0.0, 0.0,   0.5, 0.5, 1.0, 0.0,   -0.5, -0.5, 0.0, 1.0,
		-0.5, -0.5, 0.0, 1.0,   0.5, 0.5, 1.0, 0.0,    0.5, -0.5, 1.0, 1.0,
	};
	vbo_buf = vbo_new(sizeof(verts), verts);
}

ID particles_new_pool(int32 pool_life, int32 particle_life, int32 interval, float* start_pos, float* start_vel, float scale)
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
					.v[0] = pool->start_vel[0],
					.v[1] = pool->start_vel[1],
					.s[0] = pool->start_pos[0],
					.s[1] = pool->start_pos[1],
				};
			} else {
				pool->timer += dt_ms;
			}
		}
		for (int j = 0; j < MAX_PARTICLES_PER_POOL; j++) {
			if (pool->particles[j].life <= 0)
				continue;
			pool->particles[j].v[0] *= PARTICLE_DAMPNER;
			pool->particles[j].v[1] *= PARTICLE_DAMPNER;
			pool->particles[j].s[0] += pool->particles[j].v[0]*dt;
			pool->particles[j].s[1] += pool->particles[j].v[1]*dt;
			pool->particles[j].life -= dt_ms;
		}
	}
}

void particles_record_commands(VkCommandBuffer cmd_buf)
{
	mat4 vp;
	camera_get_vp(vp);
	buffer_update(pipeln.ubos[0], sizeof(mat4), vp);

	vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.pipeln);
	vkCmdBindVertexBuffers(cmd_buf, 0, 1, &vbo_buf.buf, (VkDeviceSize[]) { 0 });
	vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeln.layout, 0, 1, &pipeln.dset, 0, NULL);
	struct ParticlePool* pool;
	for (int i = 0; i < poolc; i++) {
		pool = &pools[i];
		if (pool->life <= 0)
			continue;
		buffer_update(pipeln.ubos[1], PARTICLES_UBO_SIZE, pool->particles);
		vkCmdPushConstants(cmd_buf, pipeln.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, pipeln.pushsz, &pool->scale);

		vkCmdDraw(cmd_buf, 6, MAX_PARTICLES_PER_POOL, 0, MAX_PARTICLES_PER_POOL*i);
	}
}

void particles_free()
{
	texture_free(texture);
	vbo_free(&vbo_buf);
	pipeln_free(&pipeln);
}

void particles_free_pool(ID pool_id)
{
	if (pool_id >= MAX_PARTICLE_POOLS)
		ERROR("[GFX] Invalid particle pool id: %ld", pool_id);
	pools[pool_id].life = -1;
}

#include "common.h"
#include "gfx/model.h"
#include "player.h"

struct Player player;

void player_init()
{
	player.model  = model_new(MODEL_PATH "dwarf.glb", false);
	player.matrix = model_get_matrix(player.model);
}

void player_update()
{
	glm_mat4_identity(*player.matrix);
	glm_translate(*player.matrix, (vec3){ player.body.pos.x, player.body.pos.y, 0.0 });
	glm_rotate(*player.matrix, atan2f(player.body.pos.y, player.body.pos.x), (vec3){ 0.0, 0.0, 1.0 });
}

void player_free()
{
	model_free(player.model);
}

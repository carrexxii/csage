#ifndef GFX_SPRITE_H
#define GFX_SPRITE_H

#include "maths/types.h"
#include "util/string.h"
#include "texture.h"
#include "camera.h"

#define DEFAULT_SPRITE_SHEETS 8
#define DEFAULT_SPRITES       1024

void sprites_init(struct Camera* camera);
ID sprite_sheet_new(char* name);
ID sprite_new(ID sheet);
void sprite_record_commands(VkCommandBuffer cmd_buf);
void sprite_destroy(ID sprite);
void sprites_free(void);

#endif

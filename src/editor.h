#ifndef EDITOR_H
#define EDITOR_H

#include "camera.h"

#define SPRITE_SHEETS_PATH      SPRITE_PATH
#define UI_TILESET_GRID_COLUMNS 16
#define UI_TILESET_TILE_HEIGHT  0.2f

void editor_init(void);
void editor_register_keys(void);
void editor_update(struct Camera* cam);
void editor_free(void);

#endif

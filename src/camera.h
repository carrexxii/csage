#ifndef GFX_CAMERA_H
#define GFX_CAMERA_H

extern mat4 camproj;
extern mat4 camview;
extern struct Rect    camrect;
extern enum Direction camdir;
extern int camzlvl;
extern int camzlvlmax;
extern int camzlvlscale;

void camera_init();
void camera_get_vp(mat4 out);
void camera_set_perspective();
void camera_update();

static void camera_move_up_cb   (bool kdown) { if (kdown) camdir |= DIR_UP;    else camdir &= ~DIR_UP;    }
static void camera_move_down_cb (bool kdown) { if (kdown) camdir |= DIR_DOWN;  else camdir &= ~DIR_DOWN;  }
static void camera_move_right_cb(bool kdown) { if (kdown) camdir |= DIR_RIGHT; else camdir &= ~DIR_RIGHT; }
static void camera_move_left_cb (bool kdown) { if (kdown) camdir |= DIR_LEFT;  else camdir &= ~DIR_LEFT;  }
static void camera_zoom_in_cb   (bool kdown) { if (kdown) camdir |= DIR_FORWARDS;  else camdir &= ~DIR_FORWARDS;  }
static void camera_zoom_out_cb  (bool kdown) { if (kdown) camdir |= DIR_BACKWARDS; else camdir &= ~DIR_BACKWARDS; }

inline static void camera_zlvl_up_cb  (bool kdown) { camzlvl += 1; CLAMP(camzlvl, 0, camzlvlmax); }
inline static void camera_zlvl_down_cb(bool kdown) { camzlvl -= 1; CLAMP(camzlvl, 0, camzlvlmax); }

#endif

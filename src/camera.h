#ifndef GFX_CAMERA_H
#define GFX_CAMERA_H

extern mat4 camProj;
extern mat4 camView;
extern struct Rect    camRect;
extern enum Direction camDir;
extern int camZlvl;
extern int camZlvlMax;
extern int camZlvlScale;

void camera_init();
void camera_get_vp(mat4* out);
void camera_set_perspective();
void camera_update();

static void camera_move_up_cb   (bool kdown) { if (kdown) camDir |= DIR_UP;    else camDir &= ~DIR_UP;    }
static void camera_move_down_cb (bool kdown) { if (kdown) camDir |= DIR_DOWN;  else camDir &= ~DIR_DOWN;  }
static void camera_move_right_cb(bool kdown) { if (kdown) camDir |= DIR_RIGHT; else camDir &= ~DIR_RIGHT; }
static void camera_move_left_cb (bool kdown) { if (kdown) camDir |= DIR_LEFT;  else camDir &= ~DIR_LEFT;  }
static void camera_zoom_in_cb   (bool kdown) { if (kdown) camDir |= DIR_FORWARDS;  else camDir &= ~DIR_FORWARDS;  }
static void camera_zoom_out_cb  (bool kdown) { if (kdown) camDir |= DIR_BACKWARDS; else camDir &= ~DIR_BACKWARDS; }

inline static void camera_zlvl_up_cb  (bool kdown) { camZlvl += 1; CLAMP(camZlvl, 0, camZlvlMax); }
inline static void camera_zlvl_down_cb(bool kdown) { camZlvl -= 1; CLAMP(camZlvl, 0, camZlvlMax); }

#endif

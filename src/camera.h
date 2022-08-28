#ifndef GFX_CAMERA_H
#define GFX_CAMERA_H

extern mat4 camProj;
extern mat4 camView;
extern struct Rect    camRect;
extern enum Direction camDir;
extern int camZlvl;
extern int camZlvlMax;
extern int camZlvlScale;

void init_cam();
void get_cam_vp(mat4* out);
void set_cam_perspective();
void update_camera();

static void move_cam_up_cb   (bool kdown) { if (kdown) camDir |= DIR_UP;    else camDir &= ~DIR_UP;    }
static void move_cam_down_cb (bool kdown) { if (kdown) camDir |= DIR_DOWN;  else camDir &= ~DIR_DOWN;  }
static void move_cam_right_cb(bool kdown) { if (kdown) camDir |= DIR_RIGHT; else camDir &= ~DIR_RIGHT; }
static void move_cam_left_cb (bool kdown) { if (kdown) camDir |= DIR_LEFT;  else camDir &= ~DIR_LEFT;  }
static void zoom_cam_in_cb   (bool kdown) { if (kdown) camDir |= DIR_FORWARDS;  else camDir &= ~DIR_FORWARDS;  }
static void zoom_cam_out_cb  (bool kdown) { if (kdown) camDir |= DIR_BACKWARDS; else camDir &= ~DIR_BACKWARDS; }

inline static void cam_zlvl_up_cb  (bool kdown) { camZlvl += 1; CLAMP(camZlvl, 0, camZlvlMax); }
inline static void cam_zlvl_down_cb(bool kdown) { camZlvl -= 1; CLAMP(camZlvl, 0, camZlvlMax); }

#endif

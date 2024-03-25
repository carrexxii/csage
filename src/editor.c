#include "gfx/ui/ui.h"
#include "editor.h"

static void cb_test() { DEBUG_VALUE("Clicked"); }
void editor_init()
{
	struct Texture tex = texture_new_from_image(TEXTURE_PATH "/star.png");
	struct Container* sidebar = ui_new_container(RECT(0.4f, -1.0f, 0.6f, 2.0f), NULL);
	button_new(sidebar, STRING("Button 1"), &tex, RECT(-0.9f, 0.82f, 0.8f, 0.12f), cb_test);
	button_new(sidebar, STRING("Button 2"), NULL, RECT( 0.1f, 0.82f, 0.8f, 0.12f), cb_test);
	label_new(sidebar, STRING("Test Label"), RECT(-0.5f, 0.0f, 1.0f, 0.5f));
}

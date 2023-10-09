#include "config.h"
#include "ships/missile.h"
#include "ships/ship.h"
#include "gfx/font.h"
#include "camera.h"

ShipID ship1;

void test_init()
{
	ship1 = ship_new(SHIPTYPE_1);

	// int text = font_render("_- Hello, World! Testing more words. -_", 50.0, 600.0, 50.0);
}

void test_i_cb(bool kdown)
{
	struct Ship* ship = ship_get(ship1);
	if (kdown)
		ship->thruster.τ = ship->thruster.τ_max;
	else
		ship->thruster.τ = 0.0;
}

void test_o_cb(bool kdown)
{
	struct Ship* ship = ship_get(ship1);
	if (kdown)
		ship->thruster.F = ship->thruster.F_max;
	else
		ship->thruster.F = 0.0;
}

void test_p_cb(bool kdown)
{
	struct Ship* ship = ship_get(ship1);
	if (kdown)
		ship->thruster.τ = -ship->thruster.τ_max;
	else
		ship->thruster.τ = 0.0;
}

bool test_fire_cb(int btn, bool mb_down, int x, int y)
{
	if (!mb_down)
		return true;
	struct Ship* ship = ship_get(ship1);
	ID m = missile_new(MISSILE_TYPE_ONE, ship->body.s, ship->body.v, ship1);

	vec2 t;
	camera_get_point((float)x, (float)y, t);
	missile_set_target(m, t);

	return true;
}

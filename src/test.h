#include "ships/ship.h"
#include "gfx/font.h"

ShipID ship1;

void test_init()
{
	ship1 = ship_new(SHIPTYPE_1);

	int text = font_render("_- Hello, World! Testing more words. -_", 50.0, 600.0, 50.0);
}

void test_o_cb(bool kdown)
{
	struct Ship* ship = ship_get(ship1);
	if (kdown)
		ship->thrusters[0].F = 1.0;
	else
		ship->thrusters[0].F = 0.0;
}

void test_p_cb(bool kdown)
{
	struct Ship* ship = ship_get(ship1);
	if (kdown)
		ship->thrusters[1].F = 1.0;
	else
		ship->thrusters[1].F = 0.0;
}

void test_k_cb(bool kdown)
{
	struct Ship* ship = ship_get(ship1);
	if (kdown)
		ship->thrusters[0].F = -1.0;
	else
		ship->thrusters[0].F = 0.0;
}

void test_l_cb(bool kdown)
{
	struct Ship* ship = ship_get(ship1);
	if (kdown)
		ship->thrusters[1].F = -1.0;
	else
		ship->thrusters[1].F = 0.0;
}

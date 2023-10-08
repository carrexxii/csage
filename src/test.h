#include "ships/ship.h"
#include "gfx/font.h"

ShipID ship1;

void test_init()
{
	ship1 = ship_new(SHIPTYPE_1);

	int text = font_render("_- Hello, World! Testing more words. -_", 50.0, 600.0, 50.0);
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

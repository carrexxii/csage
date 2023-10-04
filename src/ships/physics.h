#ifndef PHYSICS_H
#define PHYSICS_H

struct Body {
	float θ;
	float m, I;
	vec2 cm;
	vec2 s, v, a;
	float ω, α;
};

struct Thruster {
	float F, Fmin, Fmax;
	float θ;
	vec2 s;
};

void physics_apply_thrust(struct Thruster* thruster, struct Body* body);
void physics_integrate(struct Body* body);

#endif

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
	struct Body* parent;
	float θ;
	vec2 s;
	float F, Fmin, Fmax;
};

#endif
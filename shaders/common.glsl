#define MAX_LIGHTS 8

const vec3 VIEW_DIR = normalize(vec3(1.0f, 1.0f, -1.0f));

struct Light {
	vec4 pos;
	vec4 colour;
};

bool is_in_rect(vec2 p, vec2 tl, vec2 br)
{
	vec2 s = step(tl, p) - step(br, p);
	return abs(s.x * s.y) > 0.0001;
}

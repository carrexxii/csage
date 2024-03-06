#define MAX_LIGHTS 8

bool is_in_rect(vec2 p, vec2 tl, vec2 br)
{
	vec2 s = step(tl, p) - step(br, p);
	return abs(s.x * s.y) > 0.0001;
}

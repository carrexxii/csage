#define MAX_CHUNK_POINT_LIGHTS 8
#define MAX_CHUNK_SPOT_LIGHTS  4
#define CHUNK_SIZE             (16 * 16)

const vec3 VIEW_DIR = normalize(vec3(1.0f, 1.0f, -1.0f));

struct DirectionalLight {
	vec3 dir;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
	vec3 pos;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float constant;
	float linear;
	float quadratic;
};

struct SpotLight {
	vec3 pos;
	vec3 dir;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float constant;
	float linear;
	float quadratic;
	float cutoff;
	float outer_cutoff;
};

struct MapChunkData {
	int spotc, pointc;
	int points[MAX_CHUNK_POINT_LIGHTS];
	int spots[MAX_CHUNK_SPOT_LIGHTS];
	uint tiles[CHUNK_SIZE];
};

const float material_shininess = 16.0f;
vec3 calc_dir_light(DirectionalLight light, vec3 normal, vec3 view_dir, vec3 albedo)
{
	float diffuse = max(dot(normal, light.dir), 0.0f);

	vec3  reflect_dir = reflect(light.dir, normal);
	float spec_str = pow(max(dot(view_dir, reflect_dir), 0.0f), material_shininess);
	// specular = light.specular * specular * specular_map;
	vec3 specular = light.specular * spec_str;

	return (light.ambient  * albedo) +
	       (light.diffuse  * diffuse  * albedo) +
	       (light.specular * specular * albedo);
}

vec3 calc_point_light(PointLight light, vec3 normal, vec3 Fpos, vec3 view_dir, vec3 albedo)
{
	vec3 light_dir = normalize(light.pos - Fpos);

	float diffuse = max(dot(normal, light_dir), 0.0f);

	vec3 reflect_dir = reflect(light_dir, normal);
	float spec_str = pow(max(dot(view_dir, reflect_dir), 0.0f), material_shininess);
	// vec3 specular = light.specular * spec_str * specular_map;
	vec3 specular = light.specular * spec_str;

	float dist = length(light.pos - Fpos);
	float attenuation = 1.0f / (light.constant + light.linear*dist + light.quadratic*dist*dist);

	return (attenuation * light.ambient * albedo) +
	       (attenuation * light.diffuse * diffuse * albedo) +
	       (attenuation * specular);
}

vec3 calc_spot_light(SpotLight light, vec3 normal, vec3 Fpos, vec3 view_dir, vec3 albedo)
{
	vec3 light_dir = normalize(light.pos - Fpos);

	float diffuse = max(dot(normal, light_dir), 0.0f);

	vec3 reflect_dir = reflect(light_dir, normal);
	float spec_str = pow(max(dot(view_dir, reflect_dir), 0.0f), material_shininess);
	// vec3 specular = light.specular * spec * specular_map;
	vec3 specular = light.specular * spec_str;

	float dist        = length(light.pos - Fpos);
	float attenuation = 1.0 / (light.constant + light.linear*dist + light.quadratic*dist*dist);
	float theta       = dot(light_dir, normalize(-light.dir));
	float epsilon     = light.cutoff - light.outer_cutoff;
	float intensity   = clamp((theta - light.outer_cutoff) / epsilon, 0.0f, 1.0f);

	return (attenuation * intensity * light.ambient * albedo) +
	       (attenuation * intensity * light.diffuse * diffuse * albedo) +
	       (attenuation * intensity * specular);
}

bool is_in_rect(vec2 p, vec2 tl, vec2 br)
{
	vec2 s = step(tl, p) - step(br, p);
	return abs(s.x * s.y) > 0.0001;
}

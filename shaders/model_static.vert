#version 460

layout(location = 0) in vec3 Vpos;
layout(location = 1) in vec3 Vnormal;
layout(location = 2) in vec2 Vuv;

layout(location = 0) out vec3 Fpos;
layout(location = 1) out vec3 Fnormal;
layout(location = 2) out vec2 Fuv;

layout(binding = 0) uniform CameraUBO {
	mat4 proj;
	mat4 view;
} cam;

struct ObjectData { mat4 mat; };
layout(binding = 10) readonly buffer ObjectBufferSBO {
	ObjectData objs[];
} mdls;

void main()
{
	mat4 mdl = mdls.objs[gl_BaseInstance].mat;

	Fpos    = vec3(mdl * vec4(Vpos, 1.0f));
	Fnormal = normalize(vec3(transpose(inverse(mdl)) * vec4(Vnormal, 1.0f)));
	Fuv     = Vuv;

	// gl_Position = cam.proj * cam.view * mdl * vec4(Vpos, 1.0f);
	gl_Position = cam.proj * cam.view * vec4(Vpos, 1.0f);
}


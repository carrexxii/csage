#version 460

layout(location = 0) in vec3 Vpos;
layout(location = 1) in vec3 Vnormal;
layout(location = 2) in vec2 Vuv;

layout(location = 0) out vec2 Fuv;
layout(location = 1) out vec3 Fnormal;
layout(location = 2) out vec3 Fpos;

struct ObjectData {
	mat4 mat;
};
layout(set = 0, binding = 1) readonly buffer ObjectBufferSBO {
	ObjectData objs[];
} mdls;

layout(set = 0, binding = 2) uniform CameraBufferUBO {
	mat4 vp;
} cam;

void main()
{
	mat4 mdl = mdls.objs[gl_BaseInstance].mat;

	Fpos    = vec3(mdl * vec4(Vpos, 1.0));
	Fnormal = normalize(vec3(mdl * vec4(Vnormal, 1.0)));
	Fuv     = Vuv;

	gl_Position = cam.vp * mdl * vec4(Vpos, 1.0);
}

#version 460

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in vec3 Gxyz[3];

layout(location = 0) out vec3 Fxyz;
layout(location = 1) out vec3 Fnorm;

void main() {
	vec3 v0 = Gxyz[0];
	vec3 v1 = Gxyz[1];
	vec3 v2 = Gxyz[2];
	if (v0.z == v1.z && v0.z == v2.z) /* Top */
		Fnorm = vec3(0.0, 0.0, 1.0);
	else if (v0.y == v1.y && v0.y == v2.y) /* Front */
		Fnorm = vec3(0.0, 1.0, 0.0);
	else /* Side */
		Fnorm = vec3(1.0, 0.0, 0.0);

	Fxyz = Gxyz[0];
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();

	Fxyz = Gxyz[1];
	gl_Position = gl_in[1].gl_Position;
	EmitVertex();

	Fxyz = Gxyz[2];
	gl_Position = gl_in[2].gl_Position;
	EmitVertex();
}

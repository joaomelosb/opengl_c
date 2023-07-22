#version 400 core

layout (location = 0) in vec4 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex;

out vec3 o_normal;
out vec2 o_tex;

uniform mat4 p, v, m;

void main() {
	gl_Position = p * v * m * pos;
	o_normal = normal;
	o_tex = tex;
}

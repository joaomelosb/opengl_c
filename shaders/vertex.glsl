#version 400 core

layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 tex;

out vec2 o_tex;

uniform mat4 transform;

void main() {
	gl_Position = transform * pos;
	o_tex = tex;
}

#version 400 core

in vec3 o_normal;
in vec2 o_tex;

out vec4 frag;

uniform sampler2D tex0;

void main() {
	frag = texture(tex0, o_tex);
}

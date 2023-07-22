#version 400 core

precision highp float;

in vec2 o_tex;
out vec4 frag;

uniform sampler2D tex0;
uniform struct {
	vec2 size;
	ivec2 pos;
	ivec2 base;
} frame;

void main() {
	frag = texture(tex0, o_tex + frame.size * frame.base +
		frame.size * frame.pos);
	gl_FragDepth = frag.a;
}

#version 330 core

layout (location = 0) out vec4 outColor;

uniform sampler2D textureMain;
uniform vec4 colorMain;


// This is the data we're receiving from the vertex shader.
// "attribs" is the name of the data block (must match in vert shader).
// "fs_in" is a local name we give it in this shader file.
in attribs {
	vec3 position;
	vec2 uv;
} fs_in;


void main() {
	outColor = mix(
		texture(textureMain, fs_in.uv).rgba,
		vec4(colorMain.rgb, 1.0),
		colorMain.a
	);
}

#version 330 core

layout (location = 0) out vec4 outColor;

uniform sampler2D textureMain;


// This is the data we're receiving from the vertex shader.
// "attribs" is the name of the data block (must match in vert shader).
// "fs_in" is a local name we give it in this shader file.
in attribs {
	vec3 position;
	vec2 uv;
} fs_in;


void main() {
	vec3 color = texture(textureMain, fs_in.uv).rgb;

	color = color / (color + 1.0);
	color = pow(color, vec3(1.0/2.2));

	outColor = vec4(
		color, 1.0
	);
}

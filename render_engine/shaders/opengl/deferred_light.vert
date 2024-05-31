#version 430 core

layout (location = 0) in vec3 position;
// This shader ignores other attributes.

uniform mat4 mat;


// This is the data we're sending to the fragment shader.
// "attribs" is the name of the data block (must match in frag shader).
// "vs_out" is a local name we give it in this shader file.
out attribs {
	vec3 position;
	vec2 uv;
} vs_out;


void main() {
	vec4 pos = mat * vec4(position, 1.0);
	vs_out.position = pos.xyz;
	vs_out.uv = pos.xy / pos.w;
	gl_Position = pos;
}
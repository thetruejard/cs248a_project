#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
// This shader ignores other attributes.

// Model-view matrix.
uniform mat4 mvMat;
// Normal matrix.
uniform mat4 normalMat;
// Model-view-projection matrix.
uniform mat4 mvpMat;

// This is the data we're sending to the fragment shader.
// "attribs" is the name of the data block (must match in frag shader).
// "vs_out" is a local name we give it in this shader file.
out attribs {
	vec3 position;
	vec3 normal;
} vs_out;


void main() {
	vs_out.position = (mvMat * vec4(position, 1.0)).xyz;
	vs_out.normal = normalize((normalMat * vec4(normal, 0.0)).xyz);
	gl_Position = mvpMat * vec4(position, 1.0);
}
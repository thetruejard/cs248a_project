#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec3 bitangent;
layout (location = 4) in vec2 uv;

// Model-view matrix.
uniform mat4 mvMat;
// Normal matrix, based on model matrix.
uniform mat4 normalMat;
// Model-view-projection matrix.
uniform mat4 mvpMat;

// This is the data we're sending to the fragment shader.
// "attribs" is the name of the data block (must match in frag shader).
// "vs_out" is a local name we give it in this shader file.
out attribs {
	vec3 position;
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
	vec2 uv;
} vs_out;


void main() {
	vs_out.position = (mvMat * vec4(position, 1.0)).xyz;
	vs_out.normal = normalize((normalMat * vec4(normal, 0.0)).xyz);
	vs_out.tangent = normalize((normalMat * vec4(tangent, 0.0)).xyz);
	vs_out.bitangent = normalize((normalMat * vec4(bitangent, 0.0)).xyz);
	vs_out.uv = uv;
	gl_Position = mvpMat * vec4(position, 1.0);
}
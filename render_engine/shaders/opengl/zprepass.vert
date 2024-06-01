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


void main() {

	gl_Position = mvpMat * vec4(position, 1.0);
}
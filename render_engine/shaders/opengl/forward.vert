#version 430 core

#define MAX_SHADOW_MAPS 4

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec3 bitangent;
layout (location = 4) in vec2 uv;

// Model matrix.
uniform mat4 mMat;
// Model-view matrix.
uniform mat4 mvMat;
// Normal matrix, based on model matrix.
uniform mat4 normalMat;
// Model-view-projection matrix.
uniform mat4 mvpMat;

// Shadow map matrices (world-to-shadow, including projection)
uniform mat4 shadowMapMats[MAX_SHADOW_MAPS];

// This is the data we're sending to the fragment shader.
// "attribs" is the name of the data block (must match in frag shader).
// "vs_out" is a local name we give it in this shader file.
out attribs {
	vec3 position;
	vec2 uv;
	mat3 TBN;
	vec4 shadowMapCoords[MAX_SHADOW_MAPS];
} vs_out;


void main() {
	vs_out.position = (mvMat * vec4(position, 1.0)).xyz;
	vs_out.uv = uv;

	// Tangent matrix.
	vec3 T = normalize(vec3(normalMat * vec4(tangent, 0.0)));
	vec3 B = normalize(vec3(normalMat * vec4(bitangent, 0.0)));
	vec3 N = normalize(vec3(normalMat * vec4(normal, 0.0)));
	vs_out.TBN = mat3(T, B, N);

	// Transform point to shadow map spaces.
	for (int i = 0; i < MAX_SHADOW_MAPS; i++) {
		vs_out.shadowMapCoords[i] = shadowMapMats[i] * mMat * vec4(position, 1.0);;
	}

	gl_Position = mvpMat * vec4(position, 1.0);
}
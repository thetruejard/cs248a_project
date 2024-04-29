#version 330 core

layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec3 outNormals;
layout (location = 2) out vec3 outColor;


// The diffuse texture/color.
uniform sampler2D textureDiffuse;
uniform vec4 colorDiffuse;


// Position of the camera.
uniform vec3 cameraPos;
// The (normalized) direction the camera is facing.
uniform vec3 cameraDir;

// The shininess of the specular component of the material.
uniform float specularShininess;
// How much the specular component contributes to the final color.
uniform float specular;

// This is the data we're receiving from the vertex shader..
// "attribs" is the name of the data block (must match in vert shader).
// "fs_in" is a local name we give it in this shader file.
in attribs {
	vec3 position;
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
	vec2 uv;
} fs_in;


void main() {

	// Sample the diffuse color.
	vec3 diffuseColor = mix(
		texture(textureDiffuse, fs_in.uv).rgb,
		max(colorDiffuse.rgb, vec3(0.1)),
		colorDiffuse.a);

	// TODO: Support normal mapping.
	
	outPosition = fs_in.position;
	outNormals = fs_in.normal;
	outColor = diffuseColor;
}
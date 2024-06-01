#version 330 core

layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec3 outNormals;
layout (location = 2) out vec3 outColor;
layout (location = 3) out vec2 outMetalRough;


// The diffuse texture/color.
uniform sampler2D textureDiffuse;
uniform vec4 colorDiffuse;

// The metalness texture/value.
uniform sampler2D textureMetalness;
uniform vec2 metalnessFac;		// first: value, second: active (1.0 if use value, 0.0 if use texture)

// The roughness texture/value.
uniform sampler2D textureRoughness;
uniform vec2 roughnessFac;		// first: value, second: active (1.0 if use value, 0.0 if use texture)

// The normal texture/flag.
uniform sampler2D textureNormal;
uniform int useNormalTex;






// This is the data we're receiving from the vertex shader..
// "attribs" is the name of the data block (must match in vert shader).
// "fs_in" is a local name we give it in this shader file.
in attribs {
	vec3 position;
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
	vec2 uv;
	mat3 TBN;
} fs_in;


void main() {

	// Sample the diffuse color.
	vec3 diffuseColor = mix(
		texture(textureDiffuse, fs_in.uv).rgb,
		colorDiffuse.rgb,
		colorDiffuse.a
	);

	// Sample the metalness.
	float metalness = mix(
		texture(textureMetalness, fs_in.uv).r,
		metalnessFac.r,
		metalnessFac.g
	);

	// Sample the roughness.
	float roughness = mix(
		texture(textureRoughness, fs_in.uv).r,
		roughnessFac.r,
		roughnessFac.g
	);

	// Sample the normal.
	vec3 normal;
	if (useNormalTex == 1) {
		vec3 normalMap = 2.0 * texture(textureNormal, fs_in.uv).xyz - 1.0;
		normal = normalize(fs_in.TBN * normalMap);
	}
	else {
		normal = normalize(fs_in.normal);
	}
	
	outPosition = fs_in.position;
	outNormals = normal;
	outColor = diffuseColor;
	outMetalRough = vec2(metalness, roughness);
}
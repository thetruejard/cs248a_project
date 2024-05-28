#version 330 core

layout (location = 0) out vec4 outColor;


// gBuffer textures.
uniform sampler2D texturePos;
uniform sampler2D textureNormals;
uniform sampler2D textureAlbedo;
uniform sampler2D textureMetalRough;


// Light parameters.
struct Light {
	// Position (xyz) and type (w).
	// Type matches the enum in go_light.h.
	// None=0, Dir=1, Point=2, Spot=3.
	vec4 positionType;
	// Normalized direction for point and spot lights.
	vec3 direction;
	// Inner & outer angles (radians) for spot lights.
	vec2 innerOuterAngles;
	// Color.
	vec3 color;
	// Attenuation: (constant, linear, quadratic).
	vec3 attenuation;
};

// TODO: Support multiple lights in a single pass?
uniform Light light;


// The shininess of the specular component of the material.
uniform float specularShininess;
// How much the specular component contributes to the final color.
uniform float specular;


// This is the data we're receiving from the vertex shader..
// "attribs" is the name of the data block (must match in vert shader).
// "fs_in" is a local name we give it in this shader file.
in attribs {
	vec3 position;
	vec2 uv;
} fs_in;



vec3 computeLightFromDir(
	vec3 dirToLight,	// normalized direction
	vec3 dirToCamera,	// normalized direction
	vec3 lightColor,	// color scaled by intensity
	vec3 baseColor,		// base color of the surface
	float metalness,	// metalness of the surface
	float roughness,	// roughness of the surface
	vec3 normal			// normal of surface
) {
	// TODO: Consider replacing with PBR rendering.

	// Kindasorta Phong shading.
	// Ambient.
	float light = 0.1;
	// Diffuse.
	light += max(0, dot(normal, dirToLight));
	
	// Specular.
	float specularComponent = max(0, dot(-dirToLight, reflect(dirToCamera, normal)));
	light += specular * pow(specularComponent, specularShininess);

	return max(vec3(0.0, 0.0, 0.0), light * lightColor * baseColor);
}


float computeAttenuation(float dist, vec3 attenuation) {
	return 1.0 / (attenuation.x + attenuation.y * dist + attenuation.z * dist * dist);
}


vec3 processLight(
	Light light,		// the light to process
	vec3 position,		// position of the fragment
	vec3 baseColor,		// base color of the surface
	float metalness,	// metalness of the surface
	float roughness,	// roughness of the surface
	vec3 normal			// normal of surface
) {
	float type = round(light.positionType.w);
	if (type == 0.0) {
		return vec3(0.0);
	}

	vec3 dirToLight;
	vec3 lightColor = light.color;

	if (type == 1.0) {
		// Directional.
		dirToLight = normalize(-light.direction);
	}
	else if (type == 2.0) {
		// Point.
		vec3 diff = light.positionType.xyz - position;
		dirToLight = normalize(diff);
		lightColor *= computeAttenuation(length(diff), light.attenuation);
	}
	else if (type == 3.0) {
		// Spot.
	}

	return computeLightFromDir(
		dirToLight,
		-normalize(position),
		lightColor,
		baseColor,
		metalness,
		roughness,
		normal
	);
}





void main() {

	vec2 uv = vec2(fs_in.position.x, fs_in.position.y) * 0.5 + 0.5;
	vec3 position = texture(texturePos, fs_in.uv).xyz;
	vec3 albedo = texture(textureAlbedo, fs_in.uv).rgb;
	vec2 metalRough = texture(textureMetalRough, fs_in.uv).xy;
	vec3 normal = texture(textureNormals, fs_in.uv).xyz;

	outColor = vec4(processLight(
		light,
		position,
		albedo,
		metalRough.x,
		metalRough.y,
		normal
	), 1.0);

}
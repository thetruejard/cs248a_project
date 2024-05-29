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

const float PI = 3.14159265358979323;



// This is the data we're receiving from the vertex shader..
// "attribs" is the name of the data block (must match in vert shader).
// "fs_in" is a local name we give it in this shader file.
in attribs {
	vec3 position;
	vec2 uv;
} fs_in;


float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}



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
	//float light = 0.1;
	// Diffuse.
	//light += max(0, dot(normal, dirToLight));
	
	// Specular.
	//float specularComponent = max(0, dot(-dirToLight, reflect(dirToCamera, normal)));
	//light += specular * pow(specularComponent, specularShininess);
	
	//return max(vec3(0.0, 0.0, 0.0), light * lightColor * baseColor);

	vec3 N = normal;
    vec3 V = dirToCamera;

	vec3 F0 = vec3(0.04); 
    F0 = mix(F0, baseColor, metalness);

    // reflectance equation
    vec3 Lo = vec3(0.0);

    // calculate per-light radiance
    vec3 L = dirToLight;
    vec3 H = normalize(V + L);
    vec3 radiance = lightColor;

    // cook-torrance brdf
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metalness;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);
    Lo += (kD * baseColor / PI + specular) * radiance * NdotL;

    vec3 ambient = vec3(0.0);//vec3(0.002) * baseColor; // * ao;		// We don't support AO
    vec3 color = ambient + Lo;
	
	// Part of post/gamma stuff:
    //color = color / (color + vec3(1.0));
	return color;
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
	vec3 albedo = pow(texture(textureAlbedo, fs_in.uv).rgb, vec3(2.2));
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
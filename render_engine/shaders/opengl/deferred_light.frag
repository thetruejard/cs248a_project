#version 430 core

layout (location = 0) out vec4 outColor;


// gBuffer textures.
uniform sampler2D texturePos;
uniform sampler2D textureNormals;
uniform sampler2D textureAlbedo;
uniform sampler2D textureMetalRough;


// Light parameters.
// We always use vec4 to avoid common alignment bugs in the OpenGL drivers
struct Light {
	// Position (xyz) and type (w).
	// Type matches the enum in go_light.h.
	// None=0, Dir=1, Point=2, Spot=3.
	vec4 positionType;			// vec4
	// Normalized direction for point and spot lights.
	vec4 direction;				// vec3
	// Inner & outer angles (radians) for spot lights.
	vec4 innerOuterAngles;		// vec2
	// Color.
	vec4 color;					// vec3
	// Attenuation: (constant, linear, quadratic).
	vec4 attenuation;			// vec3
};

// TODO: Support multiple lights in a single pass?
uniform Light light;


// First elem is culling method, second is meta
// None=0
// BoundingSphere=1
// RasterSphere=2 [meta is light index]
// Tiled=3
// Clustered=4
uniform ivec2 cullingMethod;



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

    vec3 ambient = vec3(0.0); //vec3(0.002) * baseColor; // * ao;		// We don't support AO
    vec3 color = ambient + Lo;
	
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
	vec3 lightColor = vec3(light.color);

	if (type == 1.0) {
		// Directional.
		dirToLight = normalize(-vec3(light.direction));
	}
	else if (type == 2.0) {
		// Point.
		vec3 diff = light.positionType.xyz - position;
		dirToLight = normalize(diff);
		lightColor *= computeAttenuation(length(diff), vec3(light.attenuation));
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
		normalize(normal)
	);
}



// Binding must align with rp_deferred_opengl.h
layout(std430, binding = 0) buffer lightBuffer
{
	// 4 elements to avoid alignment issues. Only use the first one.
	ivec4 numLights;
    vec4 lightData[];
};
Light getLightData(int idx) {
	Light l;
	int offset = idx * 5;
	l.positionType = lightData[offset + 0];
	l.direction = lightData[offset + 1];
	l.innerOuterAngles = lightData[offset + 2];
	l.color = lightData[offset + 3];
	l.attenuation = lightData[offset + 4];
	return l;
}

// pos: vec3, radius float
vec4 getBoundingSphere(Light light) {
	const float thresh = 0.2f;
	// See computation in go_light.cpp
	float color = length(light.color.rgb);
	float atten = light.attenuation.z;
	float rad = sqrt(color / (thresh * atten));
	return vec4(light.positionType.xyz, rad);
}




void main() {

	vec2 uv = fs_in.position.xy * 0.5 + 0.5;
	vec3 position = texture(texturePos, fs_in.uv).xyz;
	vec3 albedo = pow(texture(textureAlbedo, fs_in.uv).rgb, vec3(2.2));
	vec2 metalRough = texture(textureMetalRough, fs_in.uv).xy;
	vec3 normal = texture(textureNormals, fs_in.uv).xyz;

	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

	if (cullingMethod.x == 0) {
		// None
		for (int i = 0; i < numLights.x; i++) {
			color += vec4(processLight(
				getLightData(i),
				position,
				albedo,
				metalRough.x,
				metalRough.y,
				normal
			), 0.0);
		}
	}
	else if (cullingMethod.x == 1) {
		// BoundingSphere
		for (int i = 0; i < numLights.x; i++) {
			Light l = getLightData(i);
			vec4 boundingSphere = getBoundingSphere(l);
			if (l.positionType.w == 2.0 &&
				distance(position, boundingSphere.xyz) >= boundingSphere.w) {
				// Outside the sphere, cull the light
				continue;
			}
			color += vec4(processLight(
				getLightData(i),
				position,
				albedo,
				metalRough.x,
				metalRough.y,
				normal
			), 0.0);
		}
	}
	else {
		color += vec4(1.0, 0.0, 1.0, 0.0);
	}
	
	outColor = color;

}
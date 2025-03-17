#version 430 core

#define MAX_SHADOW_MAPS 16 // Must sync with forward.vert and rp_forward_opengl.cpp.
#define SHADOW_BIAS 0.003 // 0.01 for 256 map size; 0.001 for 2048 map size
#define SHADOW_OOB_LIT false
#define RAYCAST_STEPS 64 // 64 for "high quality"
#define RAYCAST_SURFACE_THICKNESS 0.1
#define DETERMINISTIC_SAMPLES false
#define PCSS_NUM_SAMPLES_BASE 6
#define PCSS_NUM_SAMPLES (PCSS_NUM_SAMPLES_BASE * PCSS_NUM_SAMPLES_BASE)

layout (location = 0) out vec4 outColor;



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

// Which channels the metalness/roughness textures should use
// (in case they're in a shared texture.)
uniform ivec2 metalRoughChannels;


uniform vec2 viewportSize;
uniform vec3 numTiles;

uniform float zNear;
uniform float zFar;


// Shadow Maps
uniform sampler2D shadowMaps[MAX_SHADOW_MAPS];
// Shadow map bounds (scales)
uniform vec3 shadowScales[MAX_SHADOW_MAPS];




// Light parameters.
// We always use vec4 to avoid common alignment bugs in the OpenGL drivers
struct Light {
	// Light type (x), shadow type (y), shadow map index (z), and radius (w).
	// Type and ShadowType match the enums in go_light.h.
	// Type: None=0, Dir=1, Point=2, Spot=3.
	// ShadowType: None=0, Basic=1
	vec4 typeShadowIndexRadius;		// vec4
	// Position (xyz) and type (w).
	vec4 position;				// vec3
	// Normalized direction for point and spot lights.
	vec4 direction;				// vec3
	// Inner & outer angles (radians) for spot lights.
	vec4 innerOuterAngles;		// vec2
	// Color.
	vec4 color;					// vec3
	// Attenuation: (constant, linear, quadratic).
	vec4 attenuation;			// vec3
};


// First elem is culling method, second is meta
// None=0
// BoundingSphere=1
// RasterSphere=2 [meta is light index]
// Tiled=3
// Clustered=4
uniform ivec2 cullingMethod;



const float PI = 3.14159265358979323;



// This is the data we're receiving from the vertex shader.
// "attribs" is the name of the data block (must match in vert shader).
// "fs_in" is a local name we give it in this shader file.
in attribs {
	vec3 position;
	vec2 uv;
	mat3 TBN;
	vec4 shadowMapCoords[MAX_SHADOW_MAPS];
} fs_in;







///////////////////////////////////
///// Random Sampling Helpers /////
///////////////////////////////////


// From: https://stackoverflow.com/a/17479300
// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}
// Compound versions of the hashing algorithm I whipped together.
uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }
// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}
// Pseudo-random value in half-open range [0:1].
float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
float random( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec4  v ) { return floatConstruct(hash(floatBitsToUint(v))); }


vec2 rand2(vec2 uv, float seed) {
	vec2 s1, s2;
	if (DETERMINISTIC_SAMPLES) {
		s1 = vec2(uv.x * 1000 + seed * 159);
		s2 = vec2(uv.y * 2000 + seed * 653);
	}
	else {
		s1 = gl_FragCoord.xy * 314 + vec2(uv.x * 1000 + seed * 159) + fs_in.position.x*10000;
		s2 = gl_FragCoord.xy * -92 + vec2(uv.y * 2000 + seed * 653) + fs_in.position.y*20000;
	}
	return vec2(random(s1), random(s2));
}




///////////////////////////////////
///// Shadow Sampling Methods /////
///////////////////////////////////

float sampleShadow(int idx, vec2 uv, float depth, vec3 normal, vec3 dirToLight) {
	float shadowZ = texture(shadowMaps[idx], uv).r;
	shadowZ += SHADOW_BIAS / (dot(normal, dirToLight) + 0.1);
	return (depth > shadowZ) ? 0.0 : 1.0;
}

vec2 PCSS_SAMPLES[PCSS_NUM_SAMPLES];
void PCSS_GenSamples();
float PCSS_PenumbraSize(float zReceiver, float zBlocker);
vec2 PCSS_FindBlocker(float radius, int idx, vec2 uv, float zReceiver);
float PCSS_PCF_Filter(vec2 uv, int idx, vec3 normal, vec3 dirToLight, float zReceiver, float filterRadiusUV);



float computeShadowBasic(int idx, vec3 UVZ, float r, vec3 n, vec3 d) {
	return sampleShadow(idx, UVZ.xy, UVZ.z, n, d);
}


float computeShadowPCF(int idx, vec3 UVZ, float r, vec3 n, vec3 d) {
	int rad = 1;

	vec2 texelStep = 1.0 / vec2(textureSize(shadowMaps[idx], 0).xy);
	float total = 0.0;
	for (int x = -rad; x <= rad; x++) {
		for (int y = -rad; y <= rad; y++) {
			vec2 samplePos = UVZ.xy + vec2(x,y) * texelStep;
			total +=  sampleShadow(idx, samplePos, UVZ.z, n, d);
		}
	}
	return total / float((2*rad+1)*(2*rad+1));
}


float computeShadowFilteredPCF(int idx, vec3 UVZ, float r, vec3 n, vec3 d) {
	float radius = 0.01 / length(shadowScales[idx]);
	PCSS_GenSamples();
	float total = 0.0;
	for (int i = 0; i < PCSS_NUM_SAMPLES; i++) {
		total += sampleShadow(idx, UVZ.xy + radius * PCSS_SAMPLES[i], UVZ.z, n, d);
	}
	return total / PCSS_NUM_SAMPLES;
}


float computeShadowPCSS(int idx, vec3 UVZ, float r, vec3 n, vec3 d) {
	PCSS_GenSamples();

	// Convert radians to the depth map's texture space (assumes uniform light scale)
	// Multiply this by a depth value to get a lateral displacement
	// TODO: Find a better method than manually scaling to match the other methods :/
	r = tan(0.5 * r) / length(shadowScales[idx]);

	// STEP 1: blocker search
	vec2 ret = PCSS_FindBlocker(r * UVZ.z, idx, UVZ.xy, UVZ.z);
	float avgBlockerDepth = ret.x;
	float numBlockers = ret.y;
	if (numBlockers < 1)
		//There are no occluders so early out (this saves filtering)
		return 1.0f;
	// STEP 2: penumbra size
	float filterRadiusUV = (UVZ.z - avgBlockerDepth) * r / (avgBlockerDepth * UVZ.z);

	// STEP 3: filtering
	return PCSS_PCF_Filter(UVZ.xy, idx, n, d, UVZ.z, filterRadiusUV);
}



float ParallaxRaycast(int idx, vec3 source, vec3 outDir);

float computeShadowRayMarching(int idx, vec3 UVZ, float r, vec3 n, vec3 d) {
	//float init_sample = sampleShadow(idx, UVZ.xy, UVZ.z, n, d);
	//if (init_sample == 0.0)
	//	return init_sample;//computeShadowPCSS(idx, UVZ, r, n, d);
	r = tan(0.5 * r);

	PCSS_GenSamples();

	float total = 0.0;

	for (int i = 0; i < PCSS_NUM_SAMPLES; i++) {
		vec3 dir = vec3(r * PCSS_SAMPLES[i], 1.0);
		total += ParallaxRaycast(idx, UVZ, dir);
	}
	return total /= PCSS_NUM_SAMPLES;
}

// Modified from:
// https://learnopengl.com/Advanced-Lighting/Parallax-Mapping
float ParallaxRaycast(int idx, vec3 source, vec3 outDir)
{
	// Returns 0 if obstructed, or 1 otherwise.
	// Assumes source is not obstructed (makes bias unnecessary).

	const int num_steps = RAYCAST_STEPS;
	vec2 texCoords = source.xy;
	
	outDir = normalize(outDir);
	outDir *= source.z / (num_steps * outDir.z);
	outDir.z *= -1.0;

	vec3 p = vec3(texCoords, source.z);
	
	for (int i = 0; i < num_steps; i++) {
		p += outDir;
		if (p.z <= 0.0)
			return 1.0;
		float shadowZ = texture(shadowMaps[idx], p.xy).r;
		if (p.z > shadowZ && p.z - shadowZ < RAYCAST_SURFACE_THICKNESS)
			return 0.0;
	}
	return 1.0;

} 





// The following functions are adapted from:
// https://developer.download.nvidia.com/whitepapers/2008/PCSS_Integration.pdf

vec2 poissonDisk[16] = {
	vec2( -0.94201624, -0.39906216 ),
	vec2( 0.94558609, -0.76890725 ),
	vec2( -0.094184101, -0.92938870 ),
	vec2( 0.34495938, 0.29387760 ),
	vec2( -0.91588581, 0.45771432 ),
	vec2( -0.81544232, -0.87912464 ),
	vec2( -0.38277543, 0.27676845 ),
	vec2( 0.97484398, 0.75648379 ),
	vec2( 0.44323325, -0.97511554 ),
	vec2( 0.53742981, -0.47373420 ),
	vec2( -0.26496911, -0.41893023 ),
	vec2( 0.79197514, 0.19090188 ),
	vec2( -0.24188840, 0.99706507 ),
	vec2( -0.81409955, 0.91437590 ),
	vec2( 0.19984126, 0.78641367 ),
	vec2( 0.14383161, -0.14100790 )
};


void PCSS_GenSamples() {
	int n = 0;
	for (int i = 0; i < PCSS_NUM_SAMPLES_BASE; i++) {
		for (int j = 0; j < PCSS_NUM_SAMPLES_BASE; j++) {
			vec2 p = (vec2(i,j) + 0.5) / float(PCSS_NUM_SAMPLES_BASE);
			p += rand2(p, n);
			PCSS_SAMPLES[n] = sqrt(p.y) * vec2(cos(2*PI*p.x), sin(2*PI*p.x));
			n++;
		}
	}
}

// Returns (avgBlockerDepth, numBlockers)
vec2 PCSS_FindBlocker(float radius, int idx, vec2 uv, float zReceiver) {
	// This uses similar triangles to compute what
	// area of the shadow map we should search
	float blockerSum = 0.0;
	float numBlockers = 0.0;

	for( int i = 0; i < PCSS_NUM_SAMPLES; ++i ) {	// num samples
		// Don't use shadow sampling helper because this isn't for a shadow (we don't want bias)
		float shadowMapDepth = texture(shadowMaps[idx], uv + PCSS_SAMPLES[i] * radius).r;
		if (shadowMapDepth < zReceiver) {
			blockerSum += shadowMapDepth;
			numBlockers++;
		}
	}
	// Returns (avgBlockerDepth, numBlockers)
	return vec2(blockerSum / numBlockers, numBlockers);
}

float PCSS_PCF_Filter(vec2 uv, int idx, vec3 normal, vec3 dirToLight, float zReceiver, float filterRadiusUV) {
	float sum = 0.0f;
	for (int i = 0; i < PCSS_NUM_SAMPLES; ++i )
	{
		vec2 offset = PCSS_SAMPLES[i] * filterRadiusUV;
		sum += sampleShadow(idx, uv + offset, zReceiver, normal, dirToLight);
	}
	return sum / PCSS_NUM_SAMPLES;
}












float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / max(denom, 0.001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / max(denom, 0.0001);
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
	float type = round(light.typeShadowIndexRadius.x);
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
		vec3 diff = light.position.xyz - position;
		dirToLight = normalize(diff);
		lightColor *= computeAttenuation(length(diff), vec3(light.attenuation));
	}
	else if (type == 3.0) {
		// Spot.
	}

	float shadowFac = 1.0;
	float shadowType = round(light.typeShadowIndexRadius.y);
	if (shadowType != 0.0) {
		int shadowIdx = int(round(light.typeShadowIndexRadius.z));
		float radius = light.typeShadowIndexRadius.w;
		vec4 c = fs_in.shadowMapCoords[shadowIdx];
		vec3 UVZ = (c.xyz / c.w) * 0.5 + 0.5;
		if (UVZ.x < 0.0 || UVZ.x > 1.0 ||
			UVZ.y < 0.0 || UVZ.y > 1.0 ||
			UVZ.z < 0.0 || UVZ.z > 1.0) {
			/* Outside the shadow map */
			shadowFac = SHADOW_OOB_LIT ? 1.0 : 0.0;
			return vec3(0.0);
		}
		else if (shadowType == 1.0) {	// Basic
			shadowFac = computeShadowBasic(shadowIdx, UVZ, radius, normal, dirToLight);
		}
		else if (shadowType == 2.0) {	// PCF
			shadowFac = computeShadowPCF(shadowIdx, UVZ, radius, normal, dirToLight);
		}
		else if (shadowType == 3.0) {	// FilteredPCF
			shadowFac = computeShadowFilteredPCF(shadowIdx, UVZ, radius, normal, dirToLight);
		}
		else if (shadowType == 4.0) {	// PCSS
			shadowFac = computeShadowPCSS(shadowIdx, UVZ, radius, normal, dirToLight);
		}
		else if (shadowType == 5.0) {	// Ray Marching
			shadowFac = computeShadowRayMarching(shadowIdx, UVZ, radius, normal, dirToLight);
		}
		else if (shadowType == 6.0) {	// DisabledClip
			shadowFac = 1.0;
		}
	}

	// Optimization only works when nearby pixels are also fully in shadow
	// (otherwise instructions after the branch are still run.)
	if (shadowFac == 0.0)
		return vec3(0.0);

	return shadowFac * computeLightFromDir(
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
	int offset = idx * 6;
	l.typeShadowIndexRadius = lightData[offset + 0];
	l.position = lightData[offset + 1];
	l.direction = lightData[offset + 2];
	l.innerOuterAngles = lightData[offset + 3];
	l.color = lightData[offset + 4];
	l.attenuation = lightData[offset + 5];
	return l;
}

// Binding must align with rp_deferred_opengl.h
layout(std430, binding = 1) buffer tileLightMappingSSBO
{
	int tileLightMapping[];
};

// Binding must align with rp_deferred_opengl.h
layout(std430, binding = 2) buffer lightsIndexSSBO
{
	int lightsIndex[];
};



// pos: vec3, radius float
vec4 getBoundingSphere(Light light) {
	const float thresh = 0.02f;
	// See computation in go_light.cpp
	float color = max(max(light.color.r, light.color.g), light.color.b);
	float atten = light.attenuation.z;
	float rad = sqrt(color / (thresh * atten));
	return vec4(light.position.xyz, rad);
}





void main() {

	// MATERIALS

	// Sample the diffuse color.
	vec3 albedo = mix(
		texture(textureDiffuse, fs_in.uv).rgb,
		colorDiffuse.rgb,
		colorDiffuse.a
	);
	albedo = pow(albedo, vec3(2.2));

	// Sample the metalness.
	float metalness = mix(
		texture(textureMetalness, fs_in.uv)[metalRoughChannels.x],
		metalnessFac.r,
		metalnessFac.g
	);

	// Sample the roughness.
	float roughness = mix(
		texture(textureRoughness, fs_in.uv)[metalRoughChannels.y],	// TODO: Don't know why it's inverted :/
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
		normal = normalize(fs_in.TBN[2]);		// TBN[2] == normal
	}

	
	// LIGHTING


	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

	if (cullingMethod.x == 0) {
		// None
		for (int i = 0; i < numLights.x; i++) {
			color += vec4(processLight(
				getLightData(i),
				fs_in.position,
				albedo,
				metalness,
				roughness,
				normal
			), 0.0);
		}
	}
	else if (cullingMethod.x == 1) {
		// BoundingSphere
		for (int i = 0; i < numLights.x; i++) {
			Light l = getLightData(i);
			vec4 boundingSphere = getBoundingSphere(l);
			if (l.typeShadowIndexRadius.x == 2.0 &&
				distance(fs_in.position, boundingSphere.xyz) >= boundingSphere.w) {
				// Outside the sphere, cull the light
				continue;
			}
			color += vec4(processLight(
				getLightData(i),
				fs_in.position,
				albedo,
				metalness,
				roughness,
				normal
			), 0.0);
		}
	}
	else if (cullingMethod.x == 2) {
		// RasterSphere
		int lightIdx = cullingMethod.y;
		color += vec4(processLight(
			getLightData(lightIdx),
			fs_in.position,
			albedo,
			metalness,
				roughness,
			normal
		), 0.0);
	}
	else if (cullingMethod.x == 3) {
		// Tiled
		ivec2 tileCoord = ivec2(floor(fs_in.uv * numTiles.xy));
		int startIdx = tileLightMapping[2 * (tileCoord.y * int(numTiles.x) + tileCoord.x)];
		int numIdxs =  tileLightMapping[2 * (tileCoord.y * int(numTiles.x) + tileCoord.x) + 1];
		for (int i = 0; i < numIdxs; i++) {
			int lightIdx = lightsIndex[startIdx + i];
			Light light = getLightData(lightIdx);
			color += vec4(processLight(
				light,
				fs_in.position,
				albedo,
				metalness,
				roughness,
				normal
			), 0.0);
			if (light.typeShadowIndexRadius.x == 2.0)
				color += 0.01 * vec4(light.color.rgb, 0.0);
		}
	}
	else if (cullingMethod.x == 6) {
		// Clustered
		float scale = numTiles.z / log2(zFar / zNear);
		float bias = -(numTiles.z * log2(zNear) / log2(zFar / zNear));
		uint zTile     = uint(max(log2(-fs_in.position.z) * scale + bias, 0.0));
	    uvec3 tiles    = uvec3( uvec2( numTiles.xy * gl_FragCoord.xy / viewportSize.xy ), zTile);
		uint tileIndex = tiles.x +
                     uint(numTiles.x) * tiles.y +
                     uint(numTiles.x * numTiles.y) * tiles.z;
		int lightCount       = tileLightMapping[2*tileIndex+1];
		int lightIndexOffset = tileLightMapping[2*tileIndex];
		for (int i = 0; i < lightCount; i++) {
			//color += vec4(0.1, 0.0, 0.0, 0.0);
			int lightIdx = lightsIndex[lightIndexOffset + i];
			Light light = getLightData(lightIdx);
			color += vec4(processLight(
				light,
				fs_in.position,
				albedo,
				metalness,
				roughness,
				normal
			), 0.0);
		}
	}
	else {
		color += vec4(1.0, 0.0, 1.0, 0.0);
	}
	
	outColor = color;

}
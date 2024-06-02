#version 430 core
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;



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


struct LightGrid {
    int offset;
    int count;
};

struct VolumeTileAABB {
    vec4 minPoint;
    vec4 maxPoint;
};

layout(std430, binding = 3) readonly buffer clusterAABB {
    VolumeTileAABB cluster[];
};


// Binding must align with rp_deferred_opengl.h
layout(std430, binding = 0) readonly buffer lightBuffer
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


// Binding must align with rp_deferred_opengl.h
layout(std430, binding = 1) writeonly buffer tileLightMappingSSBO
{
    LightGrid tileLightMapping[];
};

// Binding must align with rp_deferred_opengl.h
layout(std430, binding = 2) writeonly buffer lightsIndexSSBO
{
    int lightsIndex[];
};


layout(std430, binding = 4) buffer globalIndexCountSSBO {
    uint globalIndexCount;
};

//Shared variables 
//shared Light sharedLights[16 * 9 * 4];

bool testSphereAABB(uint light, uint tile);
float sqDistPointAABB(vec3 point, uint tile);

const uint MAX_LIGHTS_PER_TILE = 64;


void main() {
    globalIndexCount = 0;
    //uint threadCount = gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z;
    //uint lightCount = uint(numLights);
    //uint numBatches = (lightCount + threadCount - 1) / threadCount;
    //
    //uint tileIndex = gl_LocalInvocationIndex + gl_WorkGroupSize.x * gl_WorkGroupSize.y * gl_WorkGroupSize.z * gl_WorkGroupID.z;
    uint tileIndex = gl_WorkGroupID.x +
        gl_WorkGroupID.y * gl_NumWorkGroups.x +
        gl_WorkGroupID.z * (gl_NumWorkGroups.x * gl_NumWorkGroups.y);
    //
    uint visibleLightCount = 0;
    uint visibleLightIndices[MAX_LIGHTS_PER_TILE];
    //
    //for (uint batch = 0; batch < numBatches; ++batch) {
    //    uint lightIndex = batch * threadCount + gl_LocalInvocationIndex;
    //
    //    //Prevent overflow by clamping to last light which is always null
    //    lightIndex = min(lightIndex, lightCount);
    //
    //    //Populating shared light array
    //    sharedLights[gl_LocalInvocationIndex] = getLightData(int(lightIndex));
    //    barrier();
    //
    //    //Iterating within the current batch of lights
    //    for (uint light = 0; light < lightCount && light < threadCount; ++light) {
    //        Light l = getLightData(int(light));
    //        if (l.positionType.w != 2.0 || testSphereAABB(light, tileIndex)) {
    //            visibleLightIndices[visibleLightCount] = batch * threadCount + light;
    //            visibleLightCount += 1;
    //        }
    //    }
    //}

    for (uint lightIdx = 0; lightIdx < uint(numLights) && visibleLightCount < MAX_LIGHTS_PER_TILE; ++lightIdx) {
        if (testSphereAABB(lightIdx, tileIndex)) {
            visibleLightIndices[visibleLightCount] = lightIdx;
            visibleLightCount += 1;
        }
    }



    //We want all thread groups to have completed the light tests before continuing
    barrier();

    //uint offset = atomicAdd(globalIndexCount, visibleLightCount);

    // Just used evenly spaced lists for now ig
    uint offset = tileIndex * MAX_LIGHTS_PER_TILE;

    for (uint i = 0; i < visibleLightCount; ++i) {
        lightsIndex[offset + i] = int(visibleLightIndices[i]);
    }

    tileLightMapping[tileIndex].offset = int(offset);
    tileLightMapping[tileIndex].count = int(visibleLightCount);
}



// pos: vec3, radius float
vec4 getBoundingSphere(Light light) {
    const float thresh = 0.02f;
    // See computation in go_light.cpp
    float color = max(max(light.color.r, light.color.g), light.color.b);
    float atten = light.attenuation.z;
    float rad = sqrt(color / (thresh * atten));
    return vec4(light.positionType.xyz, rad);
}


bool testSphereAABB(uint l, uint tile) {
    Light light = getLightData(int(l));
    if (light.positionType.w != 2.0)
        return true;
    vec4 bs = getBoundingSphere(light);
    float radius = bs.w;
    vec3 center = bs.xyz;
    float squaredDistance = sqDistPointAABB(center, tile);

    return squaredDistance <= (radius * radius);
}

float sqDistPointAABB(vec3 point, uint tile) {
    float sqDist = 0.0;
    VolumeTileAABB currentCell = cluster[tile];
    for (int i = 0; i < 3; ++i) {
        float v = point[i];
        if (v < currentCell.minPoint[i]) {
            sqDist += (currentCell.minPoint[i] - v) * (currentCell.minPoint[i] - v);
        }
        if (v > currentCell.maxPoint[i]) {
            sqDist += (v - currentCell.maxPoint[i]) * (v - currentCell.maxPoint[i]);
        }
    }

    return sqDist;
}
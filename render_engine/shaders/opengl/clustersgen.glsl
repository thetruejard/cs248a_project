#version 430 core
layout(local_size_x = 1, local_size_y = 1) in;

//ssbo initialization
struct VolumeTileAABB {
    vec4 minPoint;
    vec4 maxPoint;
};
layout(std430, binding = 3) buffer clusterAABB {
    VolumeTileAABB cluster[];
};

//Shared between all clusters
uniform float zNear;
uniform float zFar;

uniform mat4 inverseProjection;


//Function prototypes
vec4 clipToView(vec4 clip);
vec4 screen2View(vec4 screen);
vec3 lineIntersectionToZPlane(vec3 A, vec3 B, float zDistance);

void main() {
    //Eye position is zero in view space
    const vec3 eyePos = vec3(0.0);

    //Per Tile variables
    uint tileIndex = gl_WorkGroupID.x +
        gl_WorkGroupID.y * gl_NumWorkGroups.x +
        gl_WorkGroupID.z * (gl_NumWorkGroups.x * gl_NumWorkGroups.y);

    //Calculating the min and max point in screen space in range [0,1]
    vec4 maxPoint_sS = vec4(vec2(gl_WorkGroupID.x + 1, gl_WorkGroupID.y + 1) / gl_NumWorkGroups.xy, -1.0, 1.0); // Top Right
    vec4 minPoint_sS = vec4(gl_WorkGroupID.xy / gl_NumWorkGroups.xy, -1.0, 1.0); // Bottom left

    //Pass min and max to view space
    vec3 maxPoint_vS = screen2View(maxPoint_sS).xyz;
    vec3 minPoint_vS = screen2View(minPoint_sS).xyz;

    //Near and far values of the cluster in view space
    float tileNear = -zNear * pow(zFar / zNear, gl_WorkGroupID.z / float(gl_NumWorkGroups.z));
    float tileFar = -zNear * pow(zFar / zNear, (gl_WorkGroupID.z + 1) / float(gl_NumWorkGroups.z));

    //Finding the 4 intersection points made from the maxPoint to the cluster near/far plane
    vec3 minPointNear = lineIntersectionToZPlane(eyePos, minPoint_vS, tileNear);
    vec3 minPointFar = lineIntersectionToZPlane(eyePos, minPoint_vS, tileFar);
    vec3 maxPointNear = lineIntersectionToZPlane(eyePos, maxPoint_vS, tileNear);
    vec3 maxPointFar = lineIntersectionToZPlane(eyePos, maxPoint_vS, tileFar);

    vec3 minPointAABB = min(min(minPointNear, minPointFar), min(maxPointNear, maxPointFar));
    vec3 maxPointAABB = max(max(minPointNear, minPointFar), max(maxPointNear, maxPointFar));

    //Getting the 
    cluster[tileIndex].minPoint = vec4(minPointAABB, 0.0);
    cluster[tileIndex].maxPoint = vec4(maxPointAABB, 0.0);
}

//Creates a line from the eye to the screenpoint, then finds its intersection
//With a z oriented plane located at the given distance to the origin
vec3 lineIntersectionToZPlane(vec3 A, vec3 B, float zDistance) {
    //Because this is a Z based normal this is fixed
    vec3 normal = vec3(0.0, 0.0, 1.0);

    vec3 ab = B - A;

    //Computing the intersection length for the line and the plane
    float t = (zDistance - dot(normal, A)) / dot(normal, ab);

    //Computing the actual xyz position of the point along the line
    vec3 result = A + t * ab;

    return result;
}

vec4 clipToView(vec4 clip) {
    //View space transform
    vec4 view = inverseProjection * clip;

    //Perspective projection
    view = view / view.w;

    return view;
}

vec4 screen2View(vec4 screen01) {
    //Convert to NDC [0,1]
    vec2 texCoord = screen01.xy;

    //Convert to clipSpace
    // vec4 clip = vec4(vec2(texCoord.x, 1.0 - texCoord.y)* 2.0 - 1.0, screen.z, screen.w);
    vec4 clip = vec4(vec2(texCoord.x, texCoord.y) * 2.0 - 1.0, screen01.z, screen01.w);
    //Not sure which of the two it is just yet

    return clipToView(clip);
}

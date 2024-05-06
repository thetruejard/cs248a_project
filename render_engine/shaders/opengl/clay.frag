#version 330 core

layout (location = 0) out vec4 outColor;

// Position of the camera.
uniform vec3 cameraPos;
// The (normalized) direction the camera is facing.
uniform vec3 cameraDir;

// The color of the clay.
uniform vec3 clayColor;
// The shininess of the specular component of the material.
uniform float claySpecularShininess;
// How much the specular component contributes to the final color.
uniform float claySpecular;

// This is the data we're receiving from the vertex shader..
// "attribs" is the name of the data block (must match in vert shader).
// "fs_in" is a local name we give it in this shader file.
in attribs {
	vec3 position;
	vec3 normal;
} fs_in;


void main() {

	// Kindasorta Phong shading.
	vec3 fragToCamera = cameraPos - fs_in.position;
	vec3 lightingDir = normalize(-cameraDir + vec3(0.4, -0.2, -0.3));
	
	// Ambient.
	float light = 0.1;
	
	// Diffuse.
	light += max(0, dot(fs_in.normal, lightingDir));
	
	// Specular.
	float specular = max(0, dot(-lightingDir, reflect(normalize(fragToCamera), fs_in.normal)));
	light += claySpecular * pow(specular, claySpecularShininess);
	
	outColor = vec4(clayColor * light, 1.0);
}
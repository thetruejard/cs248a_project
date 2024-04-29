#version 330 core

layout (location = 0) out vec4 outColor;

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
	vec2 uv;
} fs_in;


void main() {

	// Sample the texture.
	vec3 diffuseColor = mix(
		texture(textureDiffuse, fs_in.uv).rgb,
		max(colorDiffuse.rgb, vec3(0.1)),
		colorDiffuse.a);

	// Kindasorta Phong shading.
	vec3 fragToCamera = cameraPos - fs_in.position;
	vec3 lightingDir = normalize(-cameraDir + vec3(0.4, -0.2, -0.3));
	
	// Ambient.
	float light = 0.1;
	
	// Diffuse.
	light += max(0, dot(fs_in.normal, lightingDir));
	
	// Specular.
	float specularComponent = max(0, dot(-lightingDir, reflect(normalize(fragToCamera), fs_in.normal)));
	light += specular * pow(specularComponent, specularShininess);

	// Lighten, for easier preview.
	light = mix(light, 1.0, 0.4);
	
	outColor = vec4(diffuseColor * light, 1.0);
}
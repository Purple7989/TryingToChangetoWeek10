#version 430

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

uniform layout(binding = 0) sampler2D s_Image;
uniform layout(binding = 1) sampler1D s_ToonTerm;

uniform float u_Discard;

// https://learnopengl.com/Advanced-Lighting/Advanced-Lighting

void main() {	
	// Get albedo from the material
	vec4 albedoColor = texture(s_Image, inUV);

    // Using a LUT to allow artists to tweak toon shading settings
    albedoColor.r = texture(s_ToonTerm, albedoColor.r).r;
    albedoColor.g = texture(s_ToonTerm, albedoColor.g).g;
    albedoColor.b = texture(s_ToonTerm, albedoColor.b).b;
	albedoColor.a = 1.0;

	if (albedoColor.a < u_Discard) {
		discard;
	}

	outColor = albedoColor;

}
#version 440

// Include our common vertex shader attributes and uniforms
#include "../fragments/vs_common.glsl"



void main() {
	

	vec3 Position = mix(inPosition, inPosition2, u_DeltaTime);
	vec3 Normal = mix(inNormal, inNormal2, u_DeltaTime);

	gl_Position = u_ModelViewProjection * vec4(Position, 1);


	// Lecture 5
	// Pass vertex pos in world space to frag shader
	outWorldPos = (u_Model * vec4(inPosition, 1.0)).xyz;

	// Normals
	outNormal = mat3(u_NormalMatrix) * Normal;

    // We use a TBN matrix for tangent space normal mapping
    vec3 T = normalize(vec3(mat3(u_NormalMatrix) * inTangent));
    vec3 B = normalize(vec3(mat3(u_NormalMatrix) * inBiTangent));
    vec3 N = normalize(vec3(mat3(u_NormalMatrix) * inNormal));
    mat3 TBN = mat3(T, B, N);

    // We can pass the TBN matrix to the fragment shader to save computation
    outTBN = TBN;

	// Pass our UV coords to the fragment shader
	outUV = inUV;

	///////////
	outColor = inColor;

}


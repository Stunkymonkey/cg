#version 440 core

layout(location = 0) out float out_depth;

layout(binding = 0) uniform PerFrame
{
	mat4 view_proj;                // not used in this shader
	mat4 inv_view_proj;            // not used in this shader
	mat4 shadowmap_view_proj;      // not used in this shader
	mat4 clip_coords_to_shadowmap; // not used in this shader
	vec3 light_dir;                // not used in this shader
	int esm_multiplier; // multiplier for exponential shadow mapping (ESM): 0 means ESM is disabled
	ivec2 resolution;   // not used in this shader
	ivec2 shadowmap_resolution; // not used in this shader
}
per_frame;

float CalcDepth()
{
	/*
	 * LOESUNG
	 */
	return gl_FragCoord.z * 2.0 - 1.0;
}

float CalcEsmDepth()
{
	float depth = CalcDepth();
	return exp(depth * per_frame.esm_multiplier);
}

void main()
{
	if (per_frame.esm_multiplier == 0)
		out_depth = CalcDepth();
	else
		out_depth = CalcEsmDepth();
}

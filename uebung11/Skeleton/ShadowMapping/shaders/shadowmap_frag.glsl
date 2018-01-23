#version 440 core

layout(location = 0) out float out_depth;

layout(std140, binding = 0) uniform PerFrame
{
	mat4 view;		               // not used in this shader
	mat4 proj;					   // not used in this shader
	mat4 inv_view_proj;            // not used in this shader
	mat4 shadowmap_view_proj;      // not used in this shader
	mat4 clip_coords_to_shadowmap; // not used in this shader
	vec4 light_dir;                // not used in this shader
	ivec2 resolution;			   // not used in this shader
	ivec2 shadowmap_resolution;	   // not used in this shader
	int pcf_switch;				   // not used in this shader
}
per_frame;

float CalcDepth()
{
	/*
	 * TODO: Aufgabe 2.2
	 * Geben Sie die z-Komponente der normalisierten Gerätekoordinaten zurück.
	 */
	return normalize(gl_FragCoord/gl_FragCoord.w).z;
}

void main()
{
	out_depth = CalcDepth();
}

#version 420 core

layout(std140, binding = 0) uniform PerFrame
{
	mat4 view;		               // view
	mat4 proj;					   // projection
	mat4 inv_view_proj;            // not used in this shader
	mat4 shadowmap_view_proj;      // not used in this shader
	mat4 clip_coords_to_shadowmap; // not used in this shader
	vec4 light_dir;                // not used in this shader
	ivec2 resolution;              // not used in this shader
	ivec2 shadowmap_resolution;    // not used in this shader
	int pcf_switch;		           // not used in this shader
}
per_frame;

layout(binding = 1) uniform PerObject
{
	mat4 model; // object coords -> world coords
}
per_object;

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec3 in_normal;

layout(location = 0) out vec3 frag_color;
layout(location = 1) out vec3 frag_normal;

void main()
{
	gl_Position = per_frame.proj * per_frame.view * per_object.model * vec4(in_pos, 1.0);
	frag_color = in_color;
	frag_normal = mat3(per_object.model) * in_normal;
}

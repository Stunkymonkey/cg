#version 440 core

// Input color, normal and depth
layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec3 frag_pos;
layout(location = 2) in vec3 frag_normal;

// Output color, normal and depth
layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_normal_depth;

void main()
{
	// Just pass through input to output
	out_color = vec4(frag_color, 1.0);

	out_normal_depth.xyz = frag_normal;
	out_normal_depth.w = length(frag_pos);
}

#version 440 core

// Per-frame transformation and information
layout(std140, binding = 0) uniform PerFrame
{
	mat4 view;
	mat4 proj;
	ivec2 resolution;
	float aspect;
	float fov;
	int render_mode;
}
per_frame;

// Per-object transformation
layout(binding = 1) uniform PerObject {
	mat4 model;
}
per_object;

// Input position, color and normal
layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec3 in_normal;

// Output color, normal and depth
layout(location = 0) out vec3 frag_color;
layout(location = 1) out vec3 frag_pos;
layout(location = 2) out vec3 frag_normal;

void main()
{
	// Transform vertex into clip coordinates
	vec4 pos = per_frame.view * per_object.model * vec4(in_pos, 1.0);

	// Transform normal
	vec3 normal = normalize(transpose(inverse(mat3(per_frame.view * per_object.model))) * in_normal);

	// Transform vertex to screen space
	gl_Position = per_frame.proj * pos;

	// Save output color, normal and depth
	frag_color = clamp(in_color, 0.0, 1.0);
	frag_pos = vec3(pos);
	frag_normal = normal;
}

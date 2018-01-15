#version 440 core

// Textures for color, normal and depth
layout(binding = 0) uniform sampler2D tex_color;
layout(binding = 1) uniform sampler2D tex_normal_depth;

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

// Constant information
layout(binding = 2) uniform ConstantBuffer
{
	vec4 reflections[4][4];
	vec4 kernel[8];
}
constants;

// Output ambient occlusion
layout(location = 0) out float out_ao;

void main()
{
	ivec2 coord = ivec2(gl_FragCoord.xy);

	// Get normal and depth at fragment position
	vec4 normal_depth = texelFetch(tex_normal_depth, coord, 0);

	vec3 normal = normal_depth.xyz;
	float depth = normal_depth.w;

	// Calculate position from shooting a ray
	vec2 screen_coords = vec2(coord) / per_frame.resolution * 2.0 - 1.0;
	vec4 pos = vec4(normalize(vec3(vec2(tan(per_frame.fov / 2.0) * per_frame.aspect,
		tan(per_frame.fov / 2.0)) * screen_coords, -1.0)) * depth, 1.0);

	// Read the reflection vector corresponding to this pixel
	coord = ivec2(mod(coord, 4));
	vec3 reflection = constants.reflections[coord.x][coord.y].xyz;

	// Calculate ambient occlusion value, higher value of ao means more occlusion
	float ao = 0;
	int num_samples = 0;

	for (int i = 0; i < 8; ++i)
	{
		// Get the sample position in camera space
		vec3 offset = 2 * constants.kernel[i].xyz;
		offset = reflect(offset, reflection);

		// Discard samples too close to the plane
		float angle = dot(offset, normal);
		if (angle < 0.05 && angle > -0.05)
			continue;

		// Invert offset if direction is contrary to the normal
		if (angle < 0.0)
			offset = -offset;
		vec4 sample_pos = pos + vec4(offset, 0.0);

		///////
		// TODO
		// Transform the sample position using the projection matrix and use it to access
		// the depth of the sample in the corresponding texture. Then calculate the
		// difference between the depth retreived from the texture and the sample's
		// distance to the camera.
		// Caution: Keep the sign of the difference in mind!
		float diff = -(offset.z - texture(tex_normal_depth, (per_frame.proj * sample_pos).xy).r);

		// Calculate the contribution to the occlusion
		if (diff > 0.0)
			ao += clamp(2.0 - diff, 0.0, 1.0);

		++num_samples;
	}

	// We have to subtract from 1.0 so that occluded areas are darker
	out_ao = 1.0;

	if (num_samples > 0)
		out_ao -= ao / float(num_samples);
}

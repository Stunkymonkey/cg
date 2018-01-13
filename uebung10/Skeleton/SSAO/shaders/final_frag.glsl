#version 440 core

// Textures for color and ambient occlusion
layout(binding = 0) uniform sampler2D tex_color;
layout(binding = 1) uniform sampler2D tex_normal_depth;
layout(binding = 2) uniform sampler2D tex_ao;

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

// Output color
layout(location = 0) out vec4 out_color;

// Get smoothed ambient occlusion
float GetAo(in ivec2 coord)
{
	float ao = 0.0;

	// Over a 3x3 stencil
	for (int x = -1; x <= 1; x += 2)
	{
		for (int y = -1; y <= 1; y += 2)
		{
			// Calculate ambient occlusion
			vec2 sample_coord = (coord + vec2(x, y)) / per_frame.resolution;
			ao += dot(textureGather(tex_ao, sample_coord), vec4(1));
		}
	}

	// Divide by 16
	return ao * 0.0625;
}

// Get Blinn-Phong
vec2 GetBlinnPhong(in ivec2 coord)
{
	// Get normal, light and depth
	vec3 normal = texelFetch(tex_normal_depth, coord, 0).xyz;
	float depth = texelFetch(tex_normal_depth, coord, 0).w;

	vec3 light_dir = -normalize(vec3(1.0, -1.0, -1.0));

	// Calculate position from shooting a ray
	vec2 screen_coords = vec2(coord) / per_frame.resolution * 2.0 - 1.0;
	vec3 pos = normalize(vec3(vec2(tan(per_frame.fov / 2.0) * per_frame.aspect, tan(per_frame.fov / 2.0)) * screen_coords, -1.0)) * depth;

	vec3 view_dir = -pos.xyz;

	///////
	// TODO
	// Calculate the diffuse and specular part of the Blinn-Phong model.
	float lambertian = 0.0;
	float specular = 0.0;

	return vec2(lambertian, specular);
}

void main()
{
	ivec2 coord = ivec2(gl_FragCoord.xy);
	
	// Get object color
	vec4 obj_color = texelFetch(tex_color, coord, 0);

	// Get Blinn-Phong illumination
	vec2 local_lighting = GetBlinnPhong(coord);
	float diffuse = local_lighting.x;
	float specular = local_lighting.y;

	float light_intensity = 1.2;

	// Weights for ambient, diffuse and specular parts
	float ka = 0.5;
	float kd = 0.25;
	float ks = 0.25;

	// Show ambient term only
	out_color = ka * obj_color * light_intensity;

	// Multiply ambient term with ambient occlusion factor
    if (per_frame.render_mode == 2 || per_frame.render_mode > 3)
        out_color *= GetAo(coord);

	// Add diffuse and specular local lighting
    if (per_frame.render_mode > 2)
        out_color += obj_color * light_intensity * (kd * diffuse + ks * specular);

	// Show only ambient occlusion factor
	if (per_frame.render_mode == 5)
		out_color = vec4(GetAo(coord));
}

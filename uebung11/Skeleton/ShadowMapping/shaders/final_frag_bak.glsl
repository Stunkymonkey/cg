#version 440 core

layout(binding = 0) uniform sampler2D tex_shadowmap;

layout(binding = 0) uniform PerFrame
{
	mat4 view_proj;                // not used in this shader
	mat4 inv_view_proj;            // clip coords -> world coords
	mat4 shadowmap_view_proj;      // not used in this shader
	mat4 clip_coords_to_shadowmap; // clip coords -> shadow map's clip coords
	vec3 light_dir;                // light direction
	int esm_multiplier; // multiplier for exponential shadow mapping (ESM): 0 means ESM is disabled
	ivec2 resolution;   // window resolution
	ivec2 shadowmap_resolution; // shadow map resolution
}
per_frame;

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec3 frag_normal;

layout(location = 0) out vec4 out_color;

float CalcNaiveSunlight(vec3 shadowmap_ndc)
{
	/*
	 * LOESUNG
	 */
	vec2 sample_tex_coord = shadowmap_ndc.xy * 0.5 + 0.5;

	float sample_depth = texture(tex_shadowmap, sample_tex_coord).x;
	float actual_depth = shadowmap_ndc.z;

	actual_depth -= 0.005;

	return (sample_depth > actual_depth) ? 1.0 : 0.0;
}

float CalcEsmSunlight(vec3 shadowmap_ndc)
{
	vec2 shadowmap_pixel = (shadowmap_ndc.xy * 0.5 + 0.5) * per_frame.shadowmap_resolution;

	float multiplier = exp(shadowmap_ndc.z * -per_frame.esm_multiplier);

	vec2 rb = fract(shadowmap_pixel);
	vec2 lt = 1.0 - rb;
	vec4 contrib_left = vec4(lt.x, 1, 1, lt.x);
	vec4 contrib_right = vec4(1, rb.xx, 1);
	vec4 contrib_bottom = vec4(rb.yy, 1, 1);
	vec4 contrib_top = vec4(1, 1, lt.yy);

	float shadow = 0.0;
	for (int x = -3; x <= 3; x += 2)
	{
		for (int y = -3; y <= 3; y += 2)
		{
			vec2 sample_pixel = floor(shadowmap_pixel + vec2(x, y));
			vec2 sample_tex_coord = sample_pixel / per_frame.shadowmap_resolution;

			vec4 depths = textureGather(tex_shadowmap, sample_tex_coord);
			depths *= multiplier;
			depths = min(depths, 1.0);

			if (x == -3)
				depths *= contrib_left;
			else if (x == 3)
				depths *= contrib_right;
			if (y == -3)
				depths *= contrib_top;
			else if (y == 3)
				depths *= contrib_bottom;

			shadow += dot(depths, vec4(1.0));
		}
	}
	shadow /= 40.0;
	return min(shadow, 1.0);
}

bool IsOnShadowmap(vec3 sm_clip_pos)
{
	vec2 xy = sm_clip_pos.xy;
	return clamp(xy, -1.0, 1.0) == xy;
}

vec3 GetClipCoordPos()
{
	vec3 pos = gl_FragCoord.xyz;
	pos.xy /= per_frame.resolution;
	pos *= 2.0;
	pos -= 1.0;
	return pos;
}

vec3 GetShadowmapNdc()
{
	vec4 pos = vec4(GetClipCoordPos(), 1.0);
	pos = per_frame.clip_coords_to_shadowmap * pos;
	return pos.xyz / pos.w;
}

float GetSunlight()
{
	vec3 shadowmap_ndc = GetShadowmapNdc();

	if (!IsOnShadowmap(shadowmap_ndc))
		return 1.0;
	else if (per_frame.esm_multiplier == 0)
		return CalcNaiveSunlight(shadowmap_ndc);
	else
		return CalcEsmSunlight(shadowmap_ndc);
}

float GetDiffuseLight(vec3 normal)
{
	/*
	 * LOESUNG
	 */
	float diffuse = dot(normal, -per_frame.light_dir);
	return max(diffuse, 0.0);
}

float GetSpecularLight(vec3 normal)
{
	/*
	 * LOESUNG
	 */
	vec4 view_dir_h = inverse(per_frame.view_proj) * vec4(0.0, 0.0, -1.0, 1.0);
	vec3 view_dir = normalize(view_dir_h.xyz);
	vec3 half_dir = normalize(-per_frame.light_dir + view_dir);
	float angle = max(dot(normal, half_dir), 0.0);
	return pow(angle, 16.0);
}

void main()
{
	float sunlight = GetSunlight();
	vec3 normal = normalize(frag_normal);
	float diffuse = sunlight * GetDiffuseLight(normal);
	float specular = sunlight * GetSpecularLight(normal);

	/*
	 * LOESUNG: ANFANG
	 */
	const float ka = 0.4;
	const float kd = 0.35;
	const float ks = 0.25;
	float light = ka + kd * diffuse + ks * specular;
	/*
	 * LOESUNG: ENDE
	 */

	out_color = vec4(frag_color * light, 1.0);
}

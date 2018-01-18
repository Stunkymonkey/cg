#version 440 core

layout(binding = 0) uniform sampler2D tex_shadowmap;

layout(std140, binding = 0) uniform PerFrame
{
	mat4 view;		                // view matrix
	mat4 proj;						// projection matrix
	mat4 inv_view_proj;             // clip coords -> world coords
	mat4 shadowmap_view_proj;       // not used in this shader
	mat4 clip_coords_to_shadowmap;  // clip coords -> shadow map's clip coords
	vec4 light_dir;                 // light direction
	ivec2 resolution;				// window resolution
	ivec2 shadowmap_resolution;		// shadow map resolution
	int pcf_switch;					// switch between naive shadow maps and PCF, 0 means PCF is disabled
}
per_frame;

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec3 frag_normal;

layout(location = 0) out vec4 out_color;

float CalcShadow(vec3 shadowmap_ndc)
{
	/*
	 * TODO: Aufgabe 2.2:
	 * Implementieren Sie naives Shadow Mapping.
	 */

	// Transform shadowmap_ndc.xy to the proper interval for sampling the texture

	// Sample the texture tex_shadowmap

	// Apply a bias and compare

	return 1.0;
}

float CalcShadowPCF(vec3 shadowmap_ndc)
{
	/*
	 * TODO: Aufgabe 2.4:
	 * Implementieren Sie Shadow Mapping mit PCF.
	 */

	 return 1.0;
}

bool IsOnShadowmap(vec3 sm_clip_pos)
{
	vec2 xy = sm_clip_pos.xy;
	return clamp(xy, -1.f, 1.f) == xy;
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
	float res = 1.f;

	if (!IsOnShadowmap(shadowmap_ndc))
		res = 1.f;
	else if (per_frame.pcf_switch == 0)
		res = CalcShadow(shadowmap_ndc);
	else
		res = CalcShadowPCF(shadowmap_ndc);

	return res;
}

float GetDiffuseLight(vec3 normal)
{
	/*
	 * TODO: Aufgabe 2.1
	 * Berechnen Sie die diffuse Beleuchtung nach dem Blinn-Phong-Beleuchtungsmodell.
	 */
	return 1.0;
}

float GetSpecularLight(vec3 normal)
{
	/*
	 * TODO: Aufgabe 2.1
	 * Berechnen Sie die spekulare Beleuchtung nach dem Blinn-Phong-Beleuchtungsmodell.
	 */
	return 1.0;
}

void main()
{
	float sunlight = GetSunlight();
	vec3 normal = normalize(frag_normal);
	float diffuse = sunlight * GetDiffuseLight(normal);
	float specular = sunlight * GetSpecularLight(normal);

	/*
	 * TODO: Aufgabe 2.1
	 * Berechnen Sie Blinn-Phong shading. Das Licht ist weiﬂ.
	 */

	float light = sunlight;

	out_color = vec4(frag_color * light, 1.0);
}

#version 440 core

layout(binding = 0) uniform sampler2D tex_shadowmap;

// tiling material textures for low elevation
layout(binding = 2) uniform sampler2D tex_low_material_y;
layout(binding = 3) uniform sampler2D tex_low_material_xz;

// tiling material textures for medium elevation
layout(binding = 4) uniform sampler2D tex_mid_material_y;
layout(binding = 5) uniform sampler2D tex_mid_material_xz;

// tiling material textures for high elevation
layout(binding = 6) uniform sampler2D tex_high_material_y;
layout(binding = 7) uniform sampler2D tex_high_material_xz;


layout(std140, binding = 0) uniform PerFrame
{
    mat4 view;                     // view matrix
    mat4 proj;                     // projection matrix
    mat4 inv_view_proj;            // clip coords -> world coords
    mat4 shadowmap_view_proj;      // not used in this shader
    mat4 clip_coords_to_shadowmap; // clip coords -> shadow map's clip coords
    vec4 light_dir;                // light direction
    ivec2 resolution;              // window resolution
    ivec2 shadowmap_resolution;    // shadow map resolution
}
per_frame;

layout( binding = 1) uniform PerObject
{
    mat4 model;       // not used in this shader
    int use_texture; // true if tex_ground shall be sampled for obtaining the color
}
per_object;

layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec3 frag_tex_coord;

layout(location = 0) out vec4 out_color;

float CalcShadow(vec3 shadowmap_ndc)
{
    vec2 shadowmap_pixel = (shadowmap_ndc.xy * 0.5 + 0.5) * per_frame.shadowmap_resolution;

    float multiplier = exp(shadowmap_ndc.z * -96);

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
    shadow /= 36.0;
    return clamp(shadow, 0.0, 1.0);
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
    else
        return CalcShadow(shadowmap_ndc);
}

float GetDiffuseLight(vec3 normal)
{
    float diffuse = dot(normal, -per_frame.light_dir.xyz);
    return max(diffuse, 0.0);
}

float GetSpecularLight(vec3 normal)
{
    vec4 ndcPos;
    ndcPos.xy = ((gl_FragCoord.xy) / (per_frame.resolution.xy)) * 2.0 - 1.0;
    ndcPos.z = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
    ndcPos.w = 1.0;
    vec4 clipPos = ndcPos / gl_FragCoord.w;
    vec4 worldPos = per_frame.inv_view_proj * clipPos;
    vec4 camPos = inverse(per_frame.view) * vec4(0.0,0.0,0.0,1.0);

    vec3 view_dir = normalize( camPos.xyz - worldPos.xyz );
    vec3 half_dir = normalize(-per_frame.light_dir.xyz + view_dir);
    float angle = max(dot(normal, half_dir), 0.0);
    return pow(angle, 16.0);
}

vec4 triplanarTextureMapping(
    vec3 uvw_coords,
    vec3 normal,
    sampler2D texture_y,
    sampler2D texture_xz)
{
    /*
     * TODO (Exercise 2.2)
     * BEGIN
     */
        
    // TODO: Take three texture samples using planar mapping along the X-, Y- and Z-axis respectively
	vec4 xtex = texture(texture_xz, uvw_coords.yz);
	vec4 ytex = texture(texture_y, uvw_coords.xz);
	vec4 ztex = texture(texture_xz, uvw_coords.xy);
    // TODO: Compute the weights for the three samples from the given surface normal
    // Step 0: Use normal vector as weights
    vec3 weights = normal;
    // Step 1: Eliminate the sign
	weights = abs(weights);
    // Step 2: Compress the weights by removing small entries in the vector
	weights = max(weights, 0.001);
    // Step 3: Normalize to force weights to sum to 1.0
	weights = normalize(weights);
    // TODO: Sum up the weighted samples and return result
	return xtex * weights.x + ytex * weights.y + ztex * weights.z;
    /* REPLACE ME */ return texture(texture_y, uvw_coords.xz); // planar mapping along Y-axis /* REPLACE ME*/

    /*
     * TODO (Exercise 2.2)
     * END
     */
}

// Simplex 2D noise
// Source: https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83

vec3 permute(vec3 x) { return mod(((x*34.0)+1.0)*x, 289.0); }

float snoise(vec2 v){
  const vec4 C = vec4(0.211324865405187, 0.366025403784439,
           -0.577350269189626, 0.024390243902439);
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);
  vec2 i1;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;
  i = mod(i, 289.0);
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
  + i.x + vec3(0.0, i1.x, 1.0 ));
  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy),
    dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;
  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}

void main()
{
    float sunlight = GetSunlight();
    vec3 normal = normalize(frag_normal);
    float diffuse = sunlight * GetDiffuseLight(normal);
    float specular = sunlight * GetSpecularLight(normal);

    const float ka = 0.4;
    const float kd = 0.35;
    const float ks = 0.25;
    float light = ka + kd * diffuse + ks * specular;

    vec3 color = frag_color;

    if (per_object.use_texture == 1)
    {
        // get triplanar texture mapping for low elevation
        vec3 low_color = triplanarTextureMapping(
            frag_tex_coord * vec3(0.3,0.6,0.3),
            normal,
            tex_low_material_y,
            tex_low_material_xz).rgb;

        // get triplanar texture mapping for medium elevation
        vec3 mid_color = triplanarTextureMapping(
            frag_tex_coord * 0.3,
            normal,
            tex_mid_material_y,
            tex_mid_material_xz).rgb;

        // get triplanar texture mapping for high elevation
        vec3 high_color = triplanarTextureMapping(
            frag_tex_coord * 0.2,
            normal,
            tex_high_material_y,
            tex_high_material_xz).rgb;

        // set borders for material changes
        float elevation_mid_low_border = 2.0 + 0.5 * snoise(frag_tex_coord.xz * 0.1);
        float elevation_high_mid_border = 12.0 + 3.0 * snoise(frag_tex_coord.xz * 0.1);
        
        // blend between materials based on elevation
        if(frag_tex_coord.y < elevation_mid_low_border - 1.0)
        {
            color = low_color;
        }
        else if(frag_tex_coord.y < elevation_mid_low_border + 1.0)
        {
            color = mix(low_color,mid_color,smoothstep(elevation_mid_low_border - 1.0,elevation_mid_low_border + 1.0,frag_tex_coord.y));
        }
        else if(frag_tex_coord.y < elevation_high_mid_border - 2.0)
        {
            color = mid_color;
        }
        else if(frag_tex_coord.y < elevation_high_mid_border + 2.0)
        {
            color = mix(mid_color,high_color,smoothstep(elevation_high_mid_border - 2.0,elevation_high_mid_border + 2.0,frag_tex_coord.y));
        }
        else
        {
            color = high_color;
        }
    }

    out_color = vec4(color * light, 1.0);
}

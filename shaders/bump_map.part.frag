in vec3 eye_norm;
in vec4 epos; // not used in USE_TANGENT_VECTOR mode
in vec2 tc; // not used in USE_TANGENT_VECTOR mode

#ifdef USE_BUMP_MAP
uniform sampler2D bump_map;

vec3 get_bump_map_normal(); // to be defined later
mat3 get_tbn(in float bscale, in vec3 n); // to be defined later

vec3 bump_map_blend(in vec3 n1, in vec3 n2) {
	return normalize(vec3(n1.xy + n2.xy, n1.z*n2.z)); // Whiteout blending; drop the n2.z term for UDN blending
}

#ifdef USE_TANGENT_VECTOR
in vec4 tangent_v;

mat3 get_tbn_default(in float bscale, in vec3 n) {
	return transpose(mat3(tangent_v.xyz*tangent_v.w, bscale*cross(n, tangent_v.xyz), n));
}

#else // !USE_TANGENT_VECTOR
uniform float bump_tb_scale =  1.0;
uniform float bump_b_scale  = -1.0;

// http://www.thetenthplanet.de/archives/1180
mat3 cotangent_frame(in vec3 N, in vec3 p, in vec2 uv, in float bscale)
{
    // get edge vectors of the pixel triangle
    vec3 dp1  = dFdx(p);
    vec3 dp2  = dFdy(p);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);
 
    // solve the linear system
    vec3 dp2perp = cross(dp2, N);
    vec3 dp1perp = cross(N, dp1);
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x + vec3(0, 0, 1.0E-10);
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y + vec3(0, 1.0E-10, 0);
 
    // construct a scale-invariant frame 
    float invmax = bump_tb_scale * inversesqrt(max(dot(T,T), dot(B,B)));
    return mat3((T * invmax), (bump_b_scale * B * (bscale * invmax)), N);
}

mat3 get_tbn_default(in float bscale, in vec3 n) {
	// assume N, the interpolated vertex normal and V, the view vector (vertex to eye / camera pos - vertex pos) from VS
    return transpose(cotangent_frame(n, epos.xyz, tc, bscale));
}

#endif // USE_TANGENT_VECTOR


#ifdef ENABLE_PARALLAX_MAP
uniform sampler2D depth_map;
uniform float hole_depth = 1.0;

vec2 apply_parallax_map() {
    mat3 TBN    = get_tbn(-1.0, eye_norm); // FIXME: why is binormal inverted from bump map case?
#ifdef PARALLAX_MAP_OFFSET_ADJ
	vec2 offset = hole_depth * (TBN * -normalize(epos.xyz)).st;
	float depth_at_1 = texture(depth_map, tc+offset).w;
	//if (depth_at_1 < 0.96f) {offset *= texture(depth_map, tc).w;}
	if (depth_at_1 < 0.96f) {offset *= (depth_at_1 + texture(depth_map, tc).w) * 0.5;}
#else
	float depth = (texture(depth_map, tc).w) * hole_depth; // Get depth from the alpha (w) of the relief map
	vec2 offset = depth * (TBN * -normalize(epos.xyz)).st; // transform view vector to tangent space
#endif
    return tc + offset; // offset the uv
}
#endif // ENABLE_PARALLAX_MAP


// Note: we assume the bump map tex coords are the same as the object diffuse tex coords
// Note: light_dir and eye_pos and transformed from eye space to tangent space
vec3 apply_bump_map_for_tbn(inout vec3 light_dir, inout vec3 eye_pos, in mat3 TBN) {
	light_dir = normalize(TBN * light_dir);
	eye_pos   = TBN * eye_pos;
	return get_bump_map_normal(); // in tangent space
}

#ifndef BUMP_MAP_CUSTOM
mat3 get_tbn(in float bscale, in vec3 n) {
	return get_tbn_default(bscale, n);
}
vec3 get_bump_map_normal() {
#ifdef USE_TILE_BLEND_NMAP
	return normalize(ByExampleProceduralNoise(tc) * 2.0 - 1.0);
#else
	return normalize(texture(bump_map, tc).xyz * 2.0 - 1.0);
#endif
}
#endif // !BUMP_MAP_CUSTOM

vec3 apply_bump_map(inout vec3 light_dir, inout vec3 eye_pos) {
	return apply_bump_map_for_tbn(light_dir, eye_pos, get_tbn(1.0, eye_norm));
}
#endif // USE_BUMP_MAP

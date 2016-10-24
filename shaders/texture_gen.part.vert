uniform float tex_coord_weight = 0.0;
uniform vec4 texgen_s, texgen_t;
out vec2 tc;

void setup_texgen_st() {
	tc = vec2(dot(fg_Vertex, texgen_s), dot(fg_Vertex, texgen_t));
}

uniform int tc_start_ix = 0;

void set_tc0_from_vert_id() { // 0,0 1,0 1,1 0,1
	int tix = (gl_VertexID + tc_start_ix) & 3;
	if      (tix == 0) {tc = vec2(0,0);}
	else if (tix == 1) {tc = vec2(1,0);}
	else if (tix == 2) {tc = vec2(1,1);}
	else               {tc = vec2(0,1);}
}

void set_bent_quad_tc0_from_vert_id() {
	int tix = (gl_VertexID + tc_start_ix) & 7;
	if      (tix == 0) {tc = vec2(.5,1);}
	else if (tix == 1) {tc = vec2( 0,1);}
	else if (tix == 2) {tc = vec2( 0,0);}
	else if (tix == 3) {tc = vec2(.5,0);}
	else if (tix == 4) {tc = vec2( 1,1);}
	else if (tix == 5) {tc = vec2(.5,1);}
	else if (tix == 6) {tc = vec2(.5,0);}
	else               {tc = vec2( 1,0);}
}

void set_tc0_blend_from_tc_vert_id() {
	set_tc0_from_vert_id();
#ifdef ENABLE_TEX_COORD_WEIGHT
	tc = mix(tc, fg_TexCoord, tex_coord_weight);
#endif
}

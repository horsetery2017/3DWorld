void main() {
	set_tc0_blend_from_tc_vert_id();
	vec4 vpos = fg_Vertex;
	add_leaf_wind(vpos);
	gl_Position = fg_ModelViewProjectionMatrix * vpos;
	calc_leaf_lighting();
} 

const char *neoVehicle_vert_src =
"uniform vec3 u_eye;\n"
"uniform vec4 u_reflProps;\n"
"uniform vec4 u_specDir[5];\n"
"uniform vec4 u_specColor[5];\n"

"#define fresnel (u_reflProps.x)\n"
"#define lightStrength (u_reflProps.y)	// speclight alpha\n"
"#define shininess (u_reflProps.z)\n"
"#define specularity (u_reflProps.w)\n"

"VSIN(ATTRIB_POS)	vec3 in_pos;\n"

"VSOUT vec4 v_color;\n"
"VSOUT vec4 v_reflcolor;\n"
"VSOUT vec2 v_tex0;\n"
"VSOUT vec2 v_tex1;\n"
"VSOUT float v_fog;\n"

"vec3 DoDirLightSpec(vec3 Ldir, vec3 Lcol, vec3 N, vec3 V, float power)\n"
"{\n"
"	return pow(clamp(dot(N, normalize(V + -Ldir)), 0.0, 1.0), power)*Lcol;\n"
"}\n"

"void\n"
"main(void)\n"
"{\n"
"	vec4 Vertex = u_world * vec4(in_pos, 1.0);\n"
"	gl_Position = u_proj * u_view * Vertex;\n"
"	vec3 Normal = mat3(u_world) * in_normal;\n"
"	vec3 viewVec = normalize(u_eye - Vertex.xyz);\n"

"	v_tex0 = in_tex0;\n"

"	v_color = in_color;\n"
"	v_color.rgb += u_ambLight.rgb*surfAmbient;\n"
"	v_color.rgb += DoDynamicLight(Vertex.xyz, Normal)*surfDiffuse*lightStrength;\n"
"	v_color = clamp(v_color, 0.0, 1.0);\n"
"	v_color *= u_matColor;\n"

"	// reflect V along Normal\n"
"	vec3 uv2 = Normal*dot(viewVec, Normal)*2.0 - viewVec;\n"
"	v_tex1 = uv2.xy*0.5 + 0.5;\n"
"	float b = 1.0 - clamp(dot(viewVec, Normal), 0.0, 1.0);\n"
"	v_reflcolor = vec4(0.0, 0.0, 0.0, 1.0);\n"
"	v_reflcolor.a = mix(b*b*b*b*b, 1.0, fresnel)*shininess;\n"

"	for(int i = 0; i < 5; i++)\n"
"		v_reflcolor.rgb += DoDirLightSpec(u_specDir[i].xyz, u_specColor[i].rgb, Normal, viewVec, u_specDir[i].w)*specularity*lightStrength;\n"

"	v_fog = DoFog(gl_Position.w);\n"
"}\n"
;

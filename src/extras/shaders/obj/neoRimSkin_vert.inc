const char *neoRimSkin_vert_src =
"uniform mat4 u_boneMatrices[64];\n"

"uniform vec3 u_viewVec;\n"
"uniform vec4 u_rampStart;\n"
"uniform vec4 u_rampEnd;\n"
"uniform vec3 u_rimData;\n"

"VSIN(ATTRIB_POS)	vec3 in_pos;\n"

"VSOUT vec4 v_color;\n"
"VSOUT vec2 v_tex0;\n"
"VSOUT float v_fog;\n"

"void\n"
"main(void)\n"
"{\n"
"	vec3 SkinVertex = vec3(0.0, 0.0, 0.0);\n"
"	vec3 SkinNormal = vec3(0.0, 0.0, 0.0);\n"
"	for(int i = 0; i < 4; i++){\n"
"		SkinVertex += (u_boneMatrices[int(in_indices[i])] * vec4(in_pos, 1.0)).xyz * in_weights[i];\n"
"		SkinNormal += (mat3(u_boneMatrices[int(in_indices[i])]) * in_normal) * in_weights[i];\n"
"	}\n"

"	vec4 Vertex = u_world * vec4(SkinVertex, 1.0);\n"
"	gl_Position = u_proj * u_view * Vertex;\n"
"	vec3 Normal = mat3(u_world) * SkinNormal;\n"

"	v_tex0 = in_tex0;\n"

"	v_color = in_color;\n"
"	v_color.rgb += u_ambLight.rgb*surfAmbient;\n"
"	v_color.rgb += DoDynamicLight(Vertex.xyz, Normal)*surfDiffuse;\n"

"	// rim light\n"
"	float f = u_rimData.x - u_rimData.y*dot(Normal, u_viewVec);\n"
"	vec4 rimlight = clamp(mix(u_rampEnd, u_rampStart, f)*u_rimData.z, 0.0, 1.0);\n"
"	v_color.rgb += rimlight.rgb;\n"

"	v_color = clamp(v_color, 0.0, 1.0);\n"
"	v_color *= u_matColor;\n"

"	v_fog = DoFog(gl_Position.z);\n"
"}\n"
;

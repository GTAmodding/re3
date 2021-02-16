const char *colourfilterIII_frag_src =
"uniform sampler2D tex0;\n"
"uniform vec4 u_blurcolor;\n"

"FSIN vec4 v_color;\n"
"FSIN vec2 v_tex0;\n"
"FSIN float v_fog;\n"

"void\n"
"main(void)\n"
"{\n"
"	float a = u_blurcolor.a;\n"
"	vec4 dst = texture(tex0, vec2(v_tex0.x, 1.0-v_tex0.y));\n"
"	vec4 prev = dst;\n"
"	for(int i = 0; i < 5; i++){\n"
"		vec4 tmp = dst*(1.0-a) + prev*u_blurcolor*a;\n"
"		prev = clamp(tmp, 0.0, 1.0);\n"
"	}\n"
"	vec4 color;\n"
"	color.rgb = prev.rgb;\n"
"	color.a = 1.0;\n"

"	FRAGCOLOR(color);\n"
"}\n"

;

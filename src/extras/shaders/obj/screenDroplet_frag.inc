const char *screenDroplet_frag_src =
"uniform sampler2D tex0;\n"
"uniform sampler2D tex1;\n"

"FSIN vec4 v_color;\n"
"FSIN vec2 v_tex0;\n"
"FSIN vec2 v_tex1;\n"
"FSIN float v_fog;\n"

"void\n"
"main(void)\n"
"{\n"
"	vec4 color;\n"
"	color = v_color*texture(tex0, vec2(v_tex0.x, 1.0-v_tex0.y));\n"
"	color *= texture(tex1, vec2(v_tex1.x, 1.0-v_tex1.y));\n"

"	FRAGCOLOR(color);\n"
"}\n"

;

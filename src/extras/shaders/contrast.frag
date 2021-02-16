uniform sampler2D tex0;
uniform vec3 u_contrastAdd;
uniform vec3 u_contrastMult;

FSIN vec4 v_color;
FSIN vec2 v_tex0;
FSIN float v_fog;

void
main(void)
{
	vec4 dst = texture(tex0, vec2(v_tex0.x, 1.0-v_tex0.y));
	vec4 color;
	color.rgb = dst.rgb*u_contrastMult + u_contrastAdd;
	color.a = 1.0;

	FRAGCOLOR(color);
}


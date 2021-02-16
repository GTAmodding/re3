uniform sampler2D tex0;
uniform vec4 u_blurcolor;

FSIN vec4 v_color;
FSIN vec2 v_tex0;
FSIN float v_fog;

void
main(void)
{
	float a = u_blurcolor.a;
	vec4 dst = texture(tex0, vec2(v_tex0.x, 1.0-v_tex0.y));
	vec4 prev = dst;
	for(int i = 0; i < 5; i++){
		vec4 tmp = dst*(1.0-a) + prev*u_blurcolor*a;
		prev = clamp(tmp, 0.0, 1.0);
	}
	vec4 color;
	color.rgb = prev.rgb;
	color.a = 1.0;

	FRAGCOLOR(color);
}


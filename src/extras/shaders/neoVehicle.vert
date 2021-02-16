uniform vec3 u_eye;
uniform vec4 u_reflProps;
uniform vec4 u_specDir[5];
uniform vec4 u_specColor[5];

#define fresnel (u_reflProps.x)
#define lightStrength (u_reflProps.y)	// speclight alpha
#define shininess (u_reflProps.z)
#define specularity (u_reflProps.w)

VSIN(ATTRIB_POS)	vec3 in_pos;

VSOUT vec4 v_color;
VSOUT vec4 v_reflcolor;
VSOUT vec2 v_tex0;
VSOUT vec2 v_tex1;
VSOUT float v_fog;

vec3 DoDirLightSpec(vec3 Ldir, vec3 Lcol, vec3 N, vec3 V, float power)
{
	return pow(clamp(dot(N, normalize(V + -Ldir)), 0.0, 1.0), power)*Lcol;
}

void
main(void)
{
	vec4 Vertex = u_world * vec4(in_pos, 1.0);
	gl_Position = u_proj * u_view * Vertex;
	vec3 Normal = mat3(u_world) * in_normal;
	vec3 viewVec = normalize(u_eye - Vertex.xyz);

	v_tex0 = in_tex0;

	v_color = in_color;
	v_color.rgb += u_ambLight.rgb*surfAmbient;
	v_color.rgb += DoDynamicLight(Vertex.xyz, Normal)*surfDiffuse*lightStrength;
	v_color = clamp(v_color, 0.0, 1.0);
	v_color *= u_matColor;

	// reflect V along Normal
	vec3 uv2 = Normal*dot(viewVec, Normal)*2.0 - viewVec;
	v_tex1 = uv2.xy*0.5 + 0.5;
	float b = 1.0 - clamp(dot(viewVec, Normal), 0.0, 1.0);
	v_reflcolor = vec4(0.0, 0.0, 0.0, 1.0);
	v_reflcolor.a = mix(b*b*b*b*b, 1.0, fresnel)*shininess;

	for(int i = 0; i < 5; i++)
		v_reflcolor.rgb += DoDirLightSpec(u_specDir[i].xyz, u_specColor[i].rgb, Normal, viewVec, u_specDir[i].w)*specularity*lightStrength;

	v_fog = DoFog(gl_Position.w);
}

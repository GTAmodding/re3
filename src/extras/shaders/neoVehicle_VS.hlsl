#include "standardConstants.h"

struct VS_in
{
	float4 Position		: POSITION;
	float3 Normal		: NORMAL;
	float2 TexCoord		: TEXCOORD0;
	float4 Prelight		: COLOR0;
};

struct VS_out {
	float4 Position		: POSITION;
	float3 TexCoord0	: TEXCOORD0;	// also fog
	float2 TexCoord1	: TEXCOORD1;
	float4 Color		: COLOR0;
	float4 ReflColor	: COLOR1;
};

float3 eye : register(c41);
float4 reflProps : register(c42);
Light specLights[5] : register(c43);


#define fresnel (reflProps.x)
#define lightStrength (reflProps.y)	// speclight alpha
#define shininess (reflProps.z)
#define specularity (reflProps.w)

VS_out main(in VS_in input)
{
	VS_out output;

	output.Position = mul(combinedMat, input.Position);
	float3 Vertex = mul(worldMat, input.Position).xyz;
	float3 Normal = mul(normalMat, input.Normal);
	float3 viewVec = normalize(eye - Vertex);

	output.TexCoord0.xy = input.TexCoord;

	output.Color = input.Prelight;
	output.Color.rgb += ambientLight.rgb * surfAmbient*lightStrength;

	int i;
	for(i = 0; i < numDirLights; i++)
		output.Color.xyz += DoDirLight(lights[i+firstDirLight], Normal)*surfDiffuse*lightStrength;
	// PS2 clamps before material color
	output.Color = clamp(output.Color, 0.0, 1.0);
	output.Color *= matCol;

	// reflect V along Normal
	float3 uv2 = Normal*dot(viewVec, Normal)*2.0 - viewVec;
	output.TexCoord1 = uv2.xy*0.5 + 0.5;
	float b = 1.0 - saturate(dot(viewVec, Normal));
	output.ReflColor = float4(0.0, 0.0, 0.0, 1.0);
	output.ReflColor.a = lerp(b*b*b*b*b, 1.0, fresnel)*shininess;

	//Light mainLight = lights[0];
	for(i = 0; i < 5; i++)
		output.ReflColor.xyz += DoDirLightSpec(specLights[i], Normal, viewVec, specLights[i].direction.w)*specularity*lightStrength;

	output.TexCoord0.z = clamp((output.Position.w - fogEnd)*fogRange, fogDisable, 1.0);

	return output;
}

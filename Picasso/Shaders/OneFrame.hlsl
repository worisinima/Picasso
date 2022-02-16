
#include "ToneMapping.hlsl"

Texture2D gTextureResource : register(t0);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

struct VertexIn
{
	float4 PosL : POSITION;
	float4 Coord : TEXCOORD;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float4 UV : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
	vout.PosH = vin.PosL;
	
	// Just pass vertex color into the pixel shader.
	vout.UV = vin.Coord;
    
	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	float4 Output = 1;
#if 1
	Output.rgba = gTextureResource.Sample(gsamLinearWrap, pin.UV.xy);
	
	Output.rgb = ACESToneMapping(Output.rgb, 1);
	//Output.rgb = LinearToSrgb(Output.rgb);
	Output.rgb = pow(Output.rgb, 1.0f / 2.2f);
#else
	Output.rgba = pow(gTextureResource.Sample(gsamLinearWrap, pin.UV.xy).rrrr * 0.99, 128);
#endif
	return Output;
}

float4 PSWithOutTone(VertexOut pin) : SV_Target
{
	float4 Output = 1;
	Output.rgba = gTextureResource.Sample(gsamLinearWrap, pin.UV.xy);
	return Output;
}
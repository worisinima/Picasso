
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
	float4 Output = 1.0f;
	Output.rgb = float3(pin.UV.xy, 0.0f);
	return Output;
}
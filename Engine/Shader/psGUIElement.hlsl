#include "GUIStructs.hlsli"

Texture2D<float4> ElementTex : register( t0 );
SamplerState ElementSampler : register( s0 );

float4 main( PInput pIn ) : SV_TARGET
{
	float4 color = ElementTex.Sample( ElementSampler, pIn.TexCoord );
	return float4( color * pIn.Color );
}
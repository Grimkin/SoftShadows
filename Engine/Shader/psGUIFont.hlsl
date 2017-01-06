#include "GUIStructs.hlsli"

Texture2D<float> FontAtlas : register( t0 );
SamplerState fontSampler : register( s0 );

float4 main( PInput pIn ) : SV_TARGET
{
	float val = FontAtlas.Sample( fontSampler, pIn.TexCoord );
	return float4( pIn.Color.rgb, val * pIn.Color.a );
}
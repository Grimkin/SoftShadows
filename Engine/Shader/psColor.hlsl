
cbuffer ObjectBuffer : register( b1 ) {
	float4x4 worldMat;
	float4 proberties;
	float4 color;
}

float4 main( float4 position : SV_POSITION ) : SV_TARGET
{
	return color;
}
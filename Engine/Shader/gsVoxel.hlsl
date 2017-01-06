struct GInput {
	float3 Position : POSITION;
	float3 Normal : NORMAL;
};

struct PSInput {
	float4 Position : SV_POSITION;
	float3 PositionWS : POSITON;
};

cbuffer GridBuffer : register( b0 ) {
	float4x4 GridViewProjMats[3];
}

static const float3 viewDirections[3] =
{
	float3( 0.0f, 0.0f, -1.0f ), // back to front
	float3( -1.0f, 0.0f, 0.0f ), // right to left
	float3( 0.0f, -1.0f, 0.0f )  // top to down 
};

uint GetViewIndex( in float3 normal ) {
	float3x3 directionMatrix;
	directionMatrix[0] = -viewDirections[0];
	directionMatrix[1] = -viewDirections[1];
	directionMatrix[2] = -viewDirections[2];
	float3 dotProducts = abs( mul( directionMatrix, normal ) );
	float maximum = max( max( dotProducts.x, dotProducts.y ), dotProducts.z );
	uint index;
	if( maximum == dotProducts.x )
		index = 0;
	else if( maximum == dotProducts.y )
		index = 1;
	else
		index = 2;
	return index;
}

[maxvertexcount( 3 )]
void main( triangle GInput input[3], inout TriangleStream<PSInput> outStream ) {
	uint viewIndex = GetViewIndex( normalize( input[0].Normal + input[1].Normal + input[2].Normal ) );

	PSInput output[3];
	[unroll]
	for( uint i = 0; i < 3; i++ ) {
		output[i].Position = mul( float4( input[i].Position, 1.0f ), GridViewProjMats[viewIndex] );
		output[i].PositionWS = input[i].Position; // position in world space
	}

	// Bloat triangle in normalized device space with the texel size of the currently bound 
	// render-target. In this way pixels, which would have been discarded due to the low 
	// resolution of the currently bound render-target, will still be rasterized. 
	float2 side0N = normalize( output[1].Position.xy - output[0].Position.xy );
	float2 side1N = normalize( output[2].Position.xy - output[1].Position.xy );
	float2 side2N = normalize( output[0].Position.xy - output[2].Position.xy );
	const float texelSize = 1.0f / 1024.0f;
	output[0].Position.xy += normalize( -side0N + side2N ) * texelSize;
	output[1].Position.xy += normalize( side0N - side1N ) * texelSize;
	output[2].Position.xy += normalize( side1N - side2N ) * texelSize;

	[unroll]
	for( uint j = 0; j < 3; j++ )
		outStream.Append( output[j] );

	outStream.RestartStrip();
}
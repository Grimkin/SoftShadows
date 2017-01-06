
static const float4 Positions[3] = {
	float4( -1.0f, -1.0f, 0.5f, 1.0f ),
	float4( -1.0f, 3.0f,0.5f,1.0f ),
	float4( 3.0f, -1.0f,0.5f,1.0f )
};

static const float2 TexCoords[3] = {
	float2( 0, 1 ),
	float2( 0, -1 ),
	float2( 2, 1 )
};

struct PSInput {
	float2 texCoord : TEXCOORD;
	float4 position : SV_POSITION;
};

PSInput main( uint uiVertexID : SV_VertexID ) {
	PSInput vOut = (PSInput)0;
	vOut.texCoord = TexCoords[uiVertexID];
	vOut.position = Positions[uiVertexID];
	return vOut;
}
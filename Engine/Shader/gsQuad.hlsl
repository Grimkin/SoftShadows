#include "GUIStructs.hlsli"

[maxvertexcount( 4 )]
void main( point GInput gIn[1], inout TriangleStream<PInput> outStream ) {
	PInput element = (PInput)0;
	float width = gIn[0].Size.x;
	float height = gIn[0].Size.y;
	element.Color = gIn[0].Color;
	float4 position = float4( gIn[0].Position, 0, 1 );
	float2 texPos = float2( gIn[0].TexPosX, 0 );
	float tWidth = gIn[0].TexSize.x;
	float tHeight = gIn[0].TexSize.y;
	element.Position = position;
	element.TexCoord = texPos;
	outStream.Append( element );
	element.Position = position + float4( width, 0, 0, 0 );
	element.TexCoord = texPos + float2( tWidth, 0 );
	outStream.Append( element );
	element.Position = position + float4( 0, -height, 0, 0 );
	element.TexCoord = texPos + float2( 0, tHeight );
	outStream.Append( element );
	element.Position = position + float4( width, -height, 0, 0 );
	element.TexCoord = texPos + float2( tWidth, tHeight );
	outStream.Append( element );

	outStream.RestartStrip();
}
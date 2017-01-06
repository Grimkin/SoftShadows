struct PInput {
	float4 Position : SV_POSITION;
};

struct FragmentElement {
	float Depth;
	uint Next;
};

RWStructuredBuffer<FragmentElement> FLBuffer : register( u1 );
RWByteAddressBuffer OffsetBuffer : register( u2 );

void main( PInput pIn ){
	uint uiFragmentCount = FLBuffer.IncrementCounter();

	uint uiScreenWidth = 1024;
	uint uiScreenHeight = 1024;

	uint uiPosX = ( pIn.Position.x + 1.0f ) * 0.5f * uiScreenWidth;
	uint uiPosY = ( pIn.Position.y + 1.0f ) * 0.5f * uiScreenHeight;
	uint uiOffsetAddress = uiPosX + uiPosY * uiScreenWidth;
	uint uiOldOffset;
	OffsetBuffer.InterlockedExchange( uiOffsetAddress, uiFragmentCount, uiOldOffset );

	FragmentElement element;
	element.Depth = pIn.Position.z;
	element.Next = uiOldOffset;
	FLBuffer[uiFragmentCount] = element;
}
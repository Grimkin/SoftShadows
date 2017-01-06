#include "GUIStructs.hlsli"

GInput main( VInput vIn ) {
	GInput vOut = (GInput) 0;
	vOut.Position = vIn.Position;
	vOut.TexPosX = vIn.TexPosX;
	vOut.TexSize = vIn.TexSize;
	vOut.Size = vIn.Size;
	vOut.Color = vIn.Color;
	return vOut;
}
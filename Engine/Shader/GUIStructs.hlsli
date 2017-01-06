struct VInput {
	float4 Color : COLOR;
	float2 Position : POSITION;
	float2 TexSize : TEXSIZE;
	float2 Size : SCALE;
	float TexPosX : TEXPOSX;
};

struct GInput {
	float4 Color : COLOR;
	float2 Position : POSITION;
	float2 TexSize : TEXSIZE;
	float2 Size : SCALE;
	float TexPosX : TEXPOSX;
};

struct PInput {
	float4 Color : COLOR;
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};
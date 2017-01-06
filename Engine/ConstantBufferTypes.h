#pragma once
#include "Types.h"

__declspec( align( 16 ) )
struct ObjectData {
	Matrix WorldMat;
	float4 Proberties;
	float4 Color;
};

__declspec( align( 16 ) )
struct CameraData {
	Matrix ViewMat;
	Matrix ProjMat;
	Matrix ViewProjMat;
	Matrix InvViewProjMat;
	float3 Position;
};
#pragma once

#include <vector>

#include "embree2\rtcore.h"
#include "embree2\rtcore_ray.h"

#include "Types.h"

class GameObject;
class Texture;
class Camera;

class Embree {
public:
	Embree();
	~Embree();

	bool Init();
	void SetGeometry( std::vector<GameObject*> objects );

	void Render( Texture* tex, Camera& camera, const float3& lightDir, float lightSize );
private:
	float GetShadowVal( const float3& pos, const float3& dir, float jitterRad );

	struct CamData {
		float3 Position;
		float3 FwdDir;
		float3 UpDir;
		float3 RightDir;
		float2 ScreenDelta;
	};

	void RenderTile( uint32_t idx, std::vector<uint32_t>& out, uint32_t width, uint32_t height, CamData& camData, const float3& lightDir, float lightSize, uint2 numTiles );

	RTCDevice m_RTCDevice = nullptr;
	RTCScene m_RTCScene = nullptr;

	uint32_t m_PictureID = 0;
	const uint32_t m_TileSize = 100;
};


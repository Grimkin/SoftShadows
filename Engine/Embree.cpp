#include "Embree.h"

#include <ppl.h>
#include <random>

#include "Game.h"
#include "Logger.h"
#include "GameObject.h"
#include "Math.h"
#include "Camera.h"
#include "Texture.h"
#include "RenderBackend.h"
#include "Logger.h"
#include "Makros.h"

Embree::Embree() {
}


Embree::~Embree() {
	if( m_RTCDevice )
		rtcDeleteDevice( m_RTCDevice );
}

bool Embree::Init() {
	m_RTCDevice = rtcNewDevice();
	if( !m_RTCDevice ) {
		Game::GetLogger().FatalError( L"Embree initialization failed" );
		return false;
	}
	return true;
}

void Embree::SetGeometry( std::vector<GameObject*> objects ) {
	if( m_RTCScene ) {
		rtcDeleteScene( m_RTCScene );
		m_RTCScene = nullptr;
	}

	m_RTCScene = rtcDeviceNewScene( m_RTCDevice, RTC_SCENE_STATIC | RTC_SCENE_COHERENT, RTC_INTERSECT1 | RTC_INTERSECT_STREAM );

	for( GameObject* object : objects ) {
		const std::vector<float3a>& positions = object->GetRenderable()->GetGeometry()->GetAlignedPositions();
		const std::vector<uint3>& indices = object->GetRenderable()->GetGeometry()->GetTriangleIndices();
		Matrix transform = object->GetTransform().GetWorldTransMat();

		uint32_t id = rtcNewTriangleMesh( m_RTCScene, RTC_GEOMETRY_STATIC, indices.size(), positions.size() );

		float4a* vertices = (float4a*)rtcMapBuffer( m_RTCScene, id, RTC_VERTEX_BUFFER );

		for( size_t i = 0; i < positions.size(); i++ ) {
			float3a aPos = positions[i];
			float4 pos = { aPos.x, aPos.y, aPos.z, 1 };
			pos = Mult( transform, pos );
			vertices[i] = make_float4a( pos );
		}

		rtcUnmapBuffer( m_RTCScene, id, RTC_VERTEX_BUFFER );

		uint3* newIndices = (uint3*)rtcMapBuffer( m_RTCScene, id, RTC_INDEX_BUFFER );

		for( size_t i = 0; i < indices.size(); i++ ) {
			newIndices[i] = indices[i];
		}

		rtcUnmapBuffer( m_RTCScene, id, RTC_INDEX_BUFFER );

	}

	rtcCommit( m_RTCScene );
}

float Embree::GetShadowVal( const float3& pos, const float3& dir, float jitterRad ) {
	std::default_random_engine e;
	std::uniform_real_distribution<float> distr( 0.f, 1.f );
	e.seed( *reinterpret_cast<const uint32_t*>( &pos.x ) ^ *reinterpret_cast<const uint32_t*>( &pos.y ) ^ *reinterpret_cast<const uint32_t*>( &pos.z ) );

	const int numSamples = 10;
	float2 p[numSamples * numSamples];

	float3 vDir1;
	float3 vDir2;
	if( abs( dir.x ) < 0.9 )
		vDir1 = Normalize( Cross( dir, float3( 1, 0, 0 ) ) );
	else
		vDir1 = Normalize( Cross( dir, float3( 0, 1, 0 ) ) );
	vDir2 = Normalize( Cross( dir, vDir1 ) );

	int iN = numSamples;
	float fN = static_cast<float>( numSamples );
	for( int j = 0; j < iN; ++j ) {
		for( int i = 0; i < iN; ++i ) {
			float randX = distr( e );
			float randY = distr( e );
			p[j * iN + i].x = ( i + ( j + randX ) / fN ) / fN;
			p[j * iN + i].y = ( j + ( i + randY ) / fN ) / fN;
		}
	}
	for( int j = 0; j < iN; ++j ) {
		float rand = distr( e );
		int k = static_cast<int>( j + rand * ( iN - j ) );
		for( int i = 0; i < iN; ++i ) {
			float temp = p[j * iN + i].x;
			p[j * iN + i].x = p[k * iN + i].x;
			p[k * iN + i].x = temp;
		}
	}
	for( int i = 0; i < iN; ++i ) {
		float rand = distr( e );
		int k = static_cast<int>( i + rand * ( iN - i ) );
		for( int j = 0; j < iN; ++j ) {
			float temp = p[j * iN + i].y;
			p[j * iN + i].y = p[j * iN + k].y;
			p[j * iN + k].y = temp;
		}
	}

	float shadowVal = 0.f;

	RTCRay rays[numSamples * numSamples];

	for( int i = 0; i < iN; ++i ) {
		for( int j = 0; j < iN; ++j ) {
			float randX = ( ( ( p[iN * j + i].x ) * 2.f ) - 1.f ) * jitterRad;
			float randY = ( ( ( p[iN * j + i].y ) * 2.f ) - 1.f ) * jitterRad;
			float3 testDir = dir + vDir1 * randX + vDir2 * randY;
			RTCRay ray;
			ray.org[0] = pos.x; ray.org[1] = pos.y; ray.org[2] = pos.z;
			ray.dir[0] = testDir.x; ray.dir[1] = testDir.y; ray.dir[2] = testDir.z;
			ray.tnear = 0.001f;
			ray.tfar = INFINITY;
			ray.geomID = 1;
			ray.primID = 0;
			ray.mask = -1;
			ray.time = 0;

			rays[i + j * iN] = ray;
		}
	}

	RTCIntersectContext context;
	context.flags = RTC_INTERSECT_COHERENT;

	rtcOccluded1M( m_RTCScene, &context, rays, iN * iN, sizeof( RTCRay ) );

	for( size_t i = 0; i < iN * iN; i++ ) {
		if( rays[i].geomID )
			shadowVal += 1.f;
	}

	shadowVal /= fN * fN;

	return shadowVal;
}

void Embree::RenderTile( uint32_t idx, std::vector<uint32_t>& out, uint32_t width, uint32_t height, CamData & camData, const float3 & lightDir, float lightSize, uint2 numTiles ) {
	uint32_t tileY = idx / numTiles.x;
	uint32_t tileX = idx - tileY * numTiles.x;
	uint32_t x0 = tileX * m_TileSize;
	uint32_t x1 = Min( x0 + m_TileSize, width );
	uint32_t y0 = tileY * m_TileSize;
	uint32_t y1 = Min( y0 + m_TileSize, height );

	float3 pos = camData.Position;
	float3 lookDir = camData.FwdDir;
	float3 lookUp = camData.UpDir;
	float3 lookRight = camData.RightDir;

	float2 screenDelta = camData.ScreenDelta;

	int hWidth = width / 2;
	int hHeight = height / 2;

	float jitterRad = tan( Deg2Rad( lightSize ) );

	std::vector<RTCRay> rays;
	rays.resize( width * height );

	for( uint32_t y = y0; y < y1; y++ ) {
		for( uint32_t x = x0; x < x1; x++ ) {
			float3 dir = lookDir + screenDelta.x * ( x - hWidth ) * lookRight + screenDelta.y * ( hHeight - y ) * lookUp;

			dir = Normalize( dir );

			RTCRay& ray = rays[x + y*width];
			ray.org[0] = pos.x; ray.org[1] = pos.y; ray.org[2] = pos.z;
			ray.dir[0] = dir.x; ray.dir[1] = dir.y; ray.dir[2] = dir.z;
			ray.tnear = 0.f;
			ray.tfar = INFINITY;
			ray.geomID = RTC_INVALID_GEOMETRY_ID;
			ray.primID = RTC_INVALID_GEOMETRY_ID;
			ray.mask = -1;
			ray.time = 0;
		}
	}

	RTCIntersectContext context;
	context.flags = RTC_INTERSECT_COHERENT;

	rtcIntersect1M( m_RTCScene, &context, rays.data(), rays.size(), sizeof( RTCRay ) );

	for( uint32_t y = y0; y < y1; y++ ) {
		for( uint32_t x = x0; x < x1; x++ ) {
			RTCRay& ray = rays[x + y*width];
			if( ray.geomID != RTC_INVALID_GEOMETRY_ID ) {
				float3 color = { .3f,.3f,.3f };
				float3 diffuse = 1.f - color;
				float3 normal = Normalize( { ray.Ng[0], ray.Ng[1], ray.Ng[2] } );

				float3 hitPos = pos;
				hitPos.x += ray.tfar * ray.dir[0];
				hitPos.y += ray.tfar * ray.dir[1];
				hitPos.z += ray.tfar * ray.dir[2];

				float shadowVal = GetShadowVal( hitPos, lightDir, jitterRad );
				float lightVal = clamp( -Dot( lightDir, normal ), 0.0f, 1.0f );

				color = color + diffuse * lightVal * shadowVal;

				unsigned int r = (unsigned int)( 255.0f * clamp( color.x, 0.0f, 1.0f ) );
				unsigned int g = (unsigned int)( 255.0f * clamp( color.y, 0.0f, 1.0f ) );
				unsigned int b = (unsigned int)( 255.0f * clamp( color.z, 0.0f, 1.0f ) );
				out[x + width*y] = 0xff000000 + ( b << 16 ) + ( g << 8 ) + r;
			}
			else
				out[x + width*y] = 0xffff0000;
		}
	}

}

void Embree::Render( Texture * tex, Camera & camera, const float3& lightDir, float lightSize ) {
	std::srand( static_cast<uint32_t>( time( 0 ) ) );
	float3 pos = camera.GetTransform().GetWorldPosition();
	float3 lookDir = camera.GetFwdVector();
	float3 lookUp = camera.GetUpVector();
	float3 lookRight = camera.GetRightVector();

	int width = static_cast<int>( tex->GetTextureWidth() );
	int height = static_cast<int>( tex->GetTextureHeight() );

	float2 screenDelta;
	screenDelta.y = 2.f * tan( camera.GetFoV() / 2.f );
	screenDelta.x = screenDelta.y * camera.GetAspectRatio();

	screenDelta.x /= width;
	screenDelta.y /= height;

	std::vector<uint32_t> out;
	out.resize( width * height );

	int hWidth = width / 2;
	int hHeight = height / 2;

	float jitterRad = tan( Deg2Rad( lightSize ) );

	std::vector<RTCRay> rays;
	rays.resize( width * height );

	for( int y = 0; y < height; y++ ) {
		for( int x = 0; x < width; x++ ) {
			float3 dir = lookDir + screenDelta.x * ( x - hWidth ) * lookRight + screenDelta.y * ( hHeight - y ) * lookUp;

			dir = Normalize( dir );

			RTCRay ray;
			ray.org[0] = pos.x; ray.org[1] = pos.y; ray.org[2] = pos.z;
			ray.dir[0] = dir.x; ray.dir[1] = dir.y; ray.dir[2] = dir.z;
			ray.tnear = 0.f;
			ray.tfar = INFINITY;
			ray.geomID = RTC_INVALID_GEOMETRY_ID;
			ray.primID = RTC_INVALID_GEOMETRY_ID;
			ray.mask = -1;
			ray.time = 0;

			rays[x + y*width] = ray;

		}
	}

	RTCIntersectContext context;
	context.flags = RTC_INTERSECT_COHERENT;

	rtcIntersect1M( m_RTCScene, &context, rays.data(), rays.size(), sizeof( RTCRay ) );

	for( int y = 0; y < height; y++ ) {
		for( int x = 0; x < width; x++ ) {
			RTCRay& ray = rays[x + y*width];
			if( ray.geomID != RTC_INVALID_GEOMETRY_ID ) {
				float3 color = { .3f,.3f,.3f };
				float3 diffuse = { 1.f, 1.f, 1.f };
				float3 normal = Normalize( { ray.Ng[0], ray.Ng[1], ray.Ng[2] } );

				float3 hitPos = pos;
				hitPos.x += ray.tfar * ray.dir[0];
				hitPos.y += ray.tfar * ray.dir[1];
				hitPos.z += ray.tfar * ray.dir[2];

				float shadowVal = GetShadowVal( hitPos, lightDir, jitterRad );
				float lightVal = clamp( -Dot( lightDir, normal ), 0.0f, 1.0f );
				/*lightVal = shadowVal < lightVal ? shadowVal : lightVal;
				if( lightVal < 0.3 )
					lightVal = 0.3f;

				color = diffuse * lightVal;*/

				color = color + diffuse * lightVal * shadowVal;

				unsigned int r = (unsigned int)( 255.0f * clamp( color.x, 0.0f, 1.0f ) );
				unsigned int g = (unsigned int)( 255.0f * clamp( color.y, 0.0f, 1.0f ) );
				unsigned int b = (unsigned int)( 255.0f * clamp( color.z, 0.0f, 1.0f ) );
				out[x + width*y] = 0xff000000 + ( b << 16 ) + ( g << 8 ) + r;
			}
			else
				out[x + width*y] = 0xffff0000;
		}
	}

	/*
	int width = static_cast<int>( tex->GetTextureWidth() );
	int height = static_cast<int>( tex->GetTextureHeight() );

	std::vector<uint32_t> out;
	out.resize( width * height );

	uint2 numTiles;
	numTiles.x = ( width + m_TileSize - 1 ) / m_TileSize;
	numTiles.y = ( height + m_TileSize - 1 ) / m_TileSize;

	CamData camData;
	camData.Position = camera.GetTransform().GetWorldPosition();
	camData.FwdDir = camera.GetFwdVector();
	camData.UpDir = camera.GetUpVector();
	camData.RightDir = camera.GetRightVector();
	camData.ScreenDelta.y = 2.f * tan( camera.GetFoV() / 2.f );
	camData.ScreenDelta.x = camData.ScreenDelta.y * camera.GetAspectRatio();

	camData.ScreenDelta.x /= width;
	camData.ScreenDelta.y /= height;


	concurrency::parallel_for( size_t( 0 ), size_t( numTiles.x*numTiles.y ), [&]( size_t idx ) {
		//Game::GetLogger().PrintLine( std::to_wstring( idx ) );
		RenderTile( idx, out, width, height, camData, lightDir, lightSize, numTiles );
	} );
	*/

	tex->UpdateData( out.data(), width * sizeof( uint32_t ), 0 );

	Game::GetRenderBackend().SaveTexture( tex->GetTextureResource(), L"EmbreeResult" + std::to_wstring( m_PictureID++ ) );
}

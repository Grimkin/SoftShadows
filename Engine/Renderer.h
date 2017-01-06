#pragma once
#include <map>
#include <vector>

#include "Types.h"
#include "ConstantBuffer.h"

class RenderPass;
class Camera;
class GUIText;

enum class RenderPassType;

__declspec( align( 16 ) )
struct LightData {
	Color LightColor;
	float3 InvLightDir;
	uint32_t NumSamples;
	float JitterRad;
	float LightAngle;
};

class Renderer {
public:
	static Renderer& Init();

	void Start();
	void Render();
	void Update();
	void AddRenderPass( RenderPass& renderPass, RenderPassType renderPassType, int8_t idx );

	void SetActiveCamera( Camera& camera );
	Camera* GetActiveCamera() {
		return m_ActiveCamera;
	}
	void OnResize();

	void SetLight( float3 invDirection, Color lightColor );
	void SetShadowParams( uint32_t numSamples, float jitterRad );
private:
	Renderer();
	virtual ~Renderer();

	void RenderOpaque();
	void RenderTransparent();
	void PostRender();

	void UpdateLightBuffer();

	std::multimap<int8_t, RenderPass*> m_OpaqueRenderPasses;
	std::multimap<int8_t, RenderPass*> m_TransparencyRenderPasses;
	std::multimap<int8_t, RenderPass*> m_PostRenderPasses;

	Camera* m_ActiveCamera;
	GUIText* m_FPSCounter;

	float3 m_LightDir;
	Color m_LightColor;
	uint32_t m_NumLightSamples = 4;
	float m_LightAngle = 0.25f;

	ConstantBuffer<LightData> m_LightBuffer;

	bool m_ClearRenderTarget = true;
	bool m_ClearDepthStencil = true;
};


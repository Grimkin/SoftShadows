#pragma once
#include <string>

#include "Types.h"
#include "ConstantBuffer.h"
#include "Material.h"

class Geometry;
class RenderPass;
class Material;
class MaterialInstance;
struct Proberties;


struct RenderableDesc {
	Geometry* Geometry = nullptr;
	Material* Material = nullptr;
	Proberties* Proberties = nullptr;
	const Color* Color = nullptr;
	std::vector<ShaderSamplerDesc> CustomSampler;
	std::vector<ShaderSRV> CustomSRVs;
};

class Renderable {
	friend class GameObject;
	friend class RenderPass;
public:
	void SetVisibility( bool visible ) {
		if( visible != m_IsVisible )
			Game::GetRenderBackend().NotifyChangeInVisibility();
		m_IsVisible = visible;
	}
	bool IsVisible() const {
		return m_IsVisible;
	}
	void AssignToRenderpass( RenderPass& pass );

	void PrepareForRender();
	void Render();
	void PostRender();

	Geometry* GetGeometry() {
		return m_Geometry;
	}

	void SetGeometry( Geometry* geometry );
	void SetColor( const Color& color );
private:
	Renderable( const RenderableDesc& desc, GameObject& owner );
	virtual ~Renderable();

	Geometry* m_Geometry;
	MaterialInstance* m_MaterialInstance;
	GameObject& m_Owner;
	std::vector<RenderPass*> m_AssignedRenderPasses;

	bool m_IsVisible = true;
};


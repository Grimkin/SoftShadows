#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <functional>

#include "D3DWrapper.h"

class RenderPass;
class Renderable;
enum class ShaderType;

struct Proberties {
	float Ambient = 0.1f;
	float Diffuse = 0.6f;
	float Specular = 0.4f;
	float Unused;
	operator float4() const {
		return float4( Ambient, Diffuse, Specular, Unused );
	}
};

struct MaterialDesc {
	std::wstring UniqueName;
	RenderPass* RenderPass;
	Proberties Proberties;
	std::vector<std::pair<ShaderFlag, SamplerDesc>> Samplers;
	std::vector<std::pair<ShaderFlag, ShaderResourceView*>> Textures;
	std::vector<UnorderedAccessView*> UAVs;
	std::vector<std::function<void()>> PreRenderFunctions;
};

struct ShaderSampler {
	ShaderType ShaderType;
	uint8_t Position;
	SamplerState* SamplerState;
};

struct ShaderSRV {
	ShaderType BindToShader;
	uint8_t Position;
	ShaderResourceView* Resource;
};

struct ShaderSamplerDesc {
	ShaderType BindToShader;
	uint8_t Position;
	SamplerDesc SamplerDesc;
};

class Material {
	friend class MaterialManager;
public:
	void BindSamplers();
	const std::vector<SamplerState*>& GetSampler( ShaderType shaderType ) const;

	void BindTextures();
	const std::vector<ShaderResourceView*>& GetTextures( ShaderType shaderType ) const;

	void BindUAVs();
	const std::vector<UnorderedAccessView*>& GetUAVs() const {
		return m_UAVs;
	}

	void PrepareRender();

	size_t GetID() {
		return m_MaterialID;
	}
	const std::wstring& GetName() const {
		return m_Name;
	}
	const Proberties& GetProberties() const {
		return m_Proberties;
	}
	const Color& GetColor() const {
		return m_Color;
	}
	RenderPass* GetRenderPass() const {
		return m_RenderPass;
	}

	static Material* Create( const MaterialDesc& desc );
	static Material* Get( const std::wstring& name );
	static Material* Get( size_t id );
private:
	Material( MaterialDesc desc, size_t id );
	virtual ~Material();

	size_t m_MaterialID;

	std::wstring m_Name;
	RenderPass* m_RenderPass;
	Proberties m_Proberties;
	Color m_Color;

	std::vector<SamplerState*> m_VSSamplers;
	std::vector<SamplerState*> m_PSSamplers;
	std::vector<SamplerState*> m_GSSamplers;
	std::vector<SamplerState*> m_HSSamplers;
	std::vector<SamplerState*> m_DSSamplers;

	std::vector<ShaderResourceView*> m_VSSRVs;
	std::vector<ShaderResourceView*> m_PSSRVs;
	std::vector<ShaderResourceView*> m_GSSRVs;
	std::vector<ShaderResourceView*> m_HSSRVs;
	std::vector<ShaderResourceView*> m_DSSRVs;

	std::vector<UnorderedAccessView*> m_UAVs;

	std::vector<std::function<void()>> m_PreRenderFunctions;

	static MaterialManager* s_MaterialManager;
};

class MaterialInstance {
	friend class MaterialManager;
public:
	MaterialInstance( Renderable& renderable, Material* material, Proberties* proberties, const Color* color, const std::vector<ShaderSamplerDesc>& customSampler, const std::vector<ShaderSRV>& customSRV );
	~MaterialInstance();

	void PrepareRender();
	void PostRender();

	const Proberties& GetProberties() const {
		return m_Proberties;
	}
	const Color& GetColor() const {
		return m_Color;
	}
	void SetColor( const Color& color ) {
		m_Color = color;
	}
private:
	void BindSamplers();
	void RestoreMaterialSampler();

	void BindTextures();
	void RestoreMaterialTextures();

	Material* m_Material;
	Renderable* m_Renderable;

	Proberties m_Proberties;
	Color m_Color = { 1.0f, 1.0f, 1.0f ,1.0f };

	std::vector<ShaderSampler> m_CustomSamplers;
	std::vector<ShaderSampler> m_DefaultSamplers;

	std::vector<ShaderSRV> m_CustomSRVs;
	std::vector<ShaderSRV> m_DefaultSRVs;

	static MaterialManager* s_MaterialManager;
};

class MaterialManager {
public:
	static MaterialManager& Init();
	Material* CreateMaterial( MaterialDesc desc );
	Material* GetDefaultMaterial() {
		return m_Materials[0];
	}
	Material* GetMaterial( std::wstring uniqueName );
	Material* GetMaterial( size_t id );

	size_t GetCurrentMaterialID() {
		return m_CurrentMaterialID;
	}
	// just sets the id no material is bound
	void SetCurrentMaterialID( size_t id ) {
		m_CurrentMaterialID = id;
	}
private:
	MaterialManager();
	virtual ~MaterialManager();

	void CreateDefaultMaterial();

	size_t m_CurrentMaterialID = 0;

	std::vector<Material*> m_Materials;
	std::unordered_map<std::wstring, size_t> m_MaterialNameToID;
	std::unordered_map<size_t, Material*> m_MaterialByDesc;
};


namespace std {
	template<>
	class hash<MaterialDesc> {
	public:
		size_t operator() ( const MaterialDesc& desc );
	};
}
#include "Material.h"

#include "Game.h"
#include "Logger.h"
#include "D3DWrapper.h"
#include "RenderBackend.h"
#include "Makros.h"
#include "RenderPass.h"
#include "Texture.h"
#include "Window.h"
#include "ConstantBuffer.h"
#include "Math.h"
#include "ConfigManager.h"

MaterialManager* Material::s_MaterialManager = nullptr;
MaterialManager* MaterialInstance::s_MaterialManager = nullptr;

void Material::BindSamplers() {
	RenderBackend* renderBackend = &Game::GetRenderBackend();
	renderBackend->SetSamplerVS( 0, m_VSSamplers );
	renderBackend->SetSamplerPS( 0, m_PSSamplers );
	renderBackend->SetSamplerGS( 0, m_GSSamplers );
	renderBackend->SetSamplerHS( 0, m_HSSamplers );
	renderBackend->SetSamplerDS( 0, m_DSSamplers );
}

const std::vector<SamplerState*>& Material::GetSampler( ShaderType shaderType ) const {
	static std::vector<SamplerState*> emptySampler;
	switch( shaderType ) {
		case ShaderType::VertexShader:
			return m_VSSamplers;
		case ShaderType::PixelShader:
			return m_PSSamplers;
		case ShaderType::GeometryShader:
			return m_GSSamplers;
		case ShaderType::HullShader:
			return m_HSSamplers;
		case ShaderType::DomainShader:
			return m_DSSamplers;
		default:
			return emptySampler;
	}
}

void Material::BindTextures() {
	RenderBackend* renderBackend = &Game::GetRenderBackend();
	renderBackend->SetSRVsVS( 0, m_VSSRVs );
	renderBackend->SetSRVsPS( 0, m_PSSRVs );
	renderBackend->SetSRVsGS( 0, m_GSSRVs );
	renderBackend->SetSRVsHS( 0, m_HSSRVs );
	renderBackend->SetSRVsDS( 0, m_DSSRVs );
}

void Material::BindUAVs() {
	std::vector<uint32_t> counts( m_UAVs.size() );
	Game::GetRenderBackend().SetUAVPS( 1, m_UAVs, counts );
}

void Material::PrepareRender() {
	BindSamplers();
	BindTextures();
	BindUAVs();

	for( auto function : m_PreRenderFunctions ) {
		function();
	}
}

const std::vector<ShaderResourceView*>& Material::GetTextures( ShaderType shaderType ) const {
	static std::vector<ShaderResourceView*> emptyTexture;
	switch( shaderType ) {
		case ShaderType::VertexShader:
			return m_VSSRVs;
		case ShaderType::PixelShader:
			return m_PSSRVs;
		case ShaderType::GeometryShader:
			return m_GSSRVs;
		case ShaderType::HullShader:
			return m_HSSRVs;
		case ShaderType::DomainShader:
			return m_DSSRVs;
		default:
			return emptyTexture;
	}
}

Material * Material::Create( const MaterialDesc & desc ) {
	return s_MaterialManager->CreateMaterial( desc );
}

Material * Material::Get( const std::wstring & name ) {
	return s_MaterialManager->GetMaterial( name );
}

Material * Material::Get( size_t id ) {
	return s_MaterialManager->GetMaterial( id );
}

Material::Material( MaterialDesc desc, size_t id )
	: m_Name( desc.UniqueName )
	, m_RenderPass( desc.RenderPass )
	, m_Proberties( desc.Proberties )
	, m_MaterialID( id )
	, m_UAVs( desc.UAVs )
	, m_PreRenderFunctions( desc.PreRenderFunctions ) {
	for( auto& sampler : desc.Samplers ) {
		SamplerState* samplerState = Game::GetRenderBackend().GetSamplerState( sampler.second );
		if( CheckFlag( sampler.first, ShaderFlag::VertexShader ) ) {
			m_VSSamplers.push_back( samplerState );
		}
		if( CheckFlag( sampler.first, ShaderFlag::PixelShader ) ) {
			m_PSSamplers.push_back( samplerState );
		}
		if( CheckFlag( sampler.first, ShaderFlag::GeometryShader ) ) {
			m_GSSamplers.push_back( samplerState );
		}
		if( CheckFlag( sampler.first, ShaderFlag::HullShader ) ) {
			m_HSSamplers.push_back( samplerState );
		}
		if( CheckFlag( sampler.first, ShaderFlag::DomainShader ) ) {
			m_DSSamplers.push_back( samplerState );
		}
	}

	for( auto& texture : desc.Textures ) {
		if( CheckFlag( texture.first, ShaderFlag::VertexShader ) ) {
			m_VSSRVs.push_back( texture.second );
		}
		if( CheckFlag( texture.first, ShaderFlag::PixelShader ) ) {
			m_PSSRVs.push_back( texture.second );
		}
		if( CheckFlag( texture.first, ShaderFlag::GeometryShader ) ) {
			m_GSSRVs.push_back( texture.second );
		}
		if( CheckFlag( texture.first, ShaderFlag::HullShader ) ) {
			m_HSSRVs.push_back( texture.second );
		}
		if( CheckFlag( texture.first, ShaderFlag::DomainShader ) ) {
			m_DSSRVs.push_back( texture.second );
		}
	}
}


Material::~Material() {
}

MaterialManager& MaterialManager::Init() {
	static MaterialManager materialManager;
	return materialManager;
}

Material* MaterialManager::CreateMaterial( MaterialDesc desc ) {
	// check for double unique name existance
	auto nameId = m_MaterialNameToID.find( desc.UniqueName );
	if( nameId != m_MaterialNameToID.end() ) {
		Game::GetLogger().Log( L"Material", L"Material with name \"" + desc.UniqueName + L"\" already exists. Choose different name or use GetMaterial() to use the material" );
		return m_Materials[nameId->second];
	}

	// check for same material type existance
	size_t hash = std::hash<MaterialDesc>()( desc );
	auto  mat = m_MaterialByDesc.find( hash );
	if( mat != m_MaterialByDesc.end() ) {
		Game::GetLogger().Log( L"Material", L"Attempt to create material with same type already existing. Existing name: \"" + mat->second->m_Name + L"\". Consider using this." );
		m_MaterialNameToID[desc.UniqueName] = nameId->second;
		return m_Materials[nameId->second];
	}

	// create new material
	size_t newId = m_Materials.size();
	Material* newMaterial = new Material( desc, newId );
	m_MaterialNameToID[desc.UniqueName] = newId;
	m_Materials.push_back( newMaterial );
	m_MaterialByDesc[hash] = newMaterial;
	return newMaterial;
}

Material * MaterialManager::GetMaterial( std::wstring uniqueName ) {
	auto nameId = m_MaterialNameToID.find( uniqueName );
	if( nameId != m_MaterialNameToID.end() ) {
		return m_Materials[nameId->second];
	}
	return nullptr;
}

Material * MaterialManager::GetMaterial( size_t id ) {
	if( id < m_Materials.size() ) {
		return m_Materials[id];
	}
	return nullptr;
}

MaterialManager::MaterialManager() {
	Game::SetMaterialManager( *this );
	Material::s_MaterialManager = this;
	MaterialInstance::s_MaterialManager = this;
	CreateDefaultMaterial();
}

MaterialManager::~MaterialManager() {
	SDeleteVec<Material>( m_Materials, []( Material* ptr ) { delete ptr; } );
}

void MaterialManager::CreateDefaultMaterial() {
	MaterialDesc desc;
	desc.UniqueName = L"DefaultMaterial";

	ShaderInit sInit;
	std::vector<InputDesc> inputDesc = {
		{ "POSITION", 0, Format::R32G32B32_Float, 0, 0, InputSpecification::PerVertexData, 0 },
		{ "Normal", 0, Format::R32G32B32_Float, 1, D3D11_APPEND_ALIGNED_ELEMENT, InputSpecification::PerVertexData, 0},
		{ "TEXCOORD", 0, Format::R32G32_Float, 2, D3D11_APPEND_ALIGNED_ELEMENT, InputSpecification::PerVertexData, 0}
	};
	sInit.file = L"vsTest";
	sInit.customInputDesc = inputDesc;
	sInit.shaderType = ShaderType::VertexShader;

	RenderPassInit rInit;
	rInit.Name = L"DefaultRenderPass";
	rInit.PixelShader = Shader::Get( L"psTest" );
	if( Game::GetConfig().GetBool( L"Renormalize", false ) )
		rInit.GeometryShader = Shader::Get( L"gsNormal" );
	rInit.VertexShader = Shader::Get( sInit );
	if( Game::GetConfig().GetBool( L"BackFaceCulling", true ) )
		rInit.RasterizerDesc.CullMode = CullMode::Back;
	else
		rInit.RasterizerDesc.CullMode = CullMode::None;

	rInit.PreFunction = [&]() {
		Game::GetRenderBackend().ClearDeferredRTV( { 0.f,0.f,1.f,1.f } );
		Game::GetRenderBackend().SetDeferredRTVs();
	};
	desc.RenderPass = RenderPass::Create( rInit );

	SamplerDesc sampDesc;
	desc.Samplers = { { ShaderFlag::PixelShader, sampDesc } };

	Texture* tex = Game::GetTextureManager().GetDummyTexture();

	desc.Textures = { { ShaderFlag::PixelShader, tex->GetSRV() } };

	m_Materials.push_back( new Material( desc, 0 ) );

	m_Materials[0]->BindSamplers();
	m_Materials[0]->BindTextures();
}


size_t std::hash<MaterialDesc>::operator() ( const MaterialDesc& desc ) {
	size_t out = *reinterpret_cast<const size_t*>( &desc.Proberties.Ambient );
	out ^= *reinterpret_cast<const size_t*>( &desc.Proberties.Diffuse );
	out ^= *reinterpret_cast<const size_t*>( &desc.Proberties.Specular );
	int i = 0;
	for( auto& sampler : desc.Samplers ) {
		out ^= static_cast<size_t>( sampler.first ) << ( i + 4 );
		out ^= std::hash<SamplerDesc>()( sampler.second ) << i;
		i++;
	}
	i = 0;
	for( auto tex : desc.Textures ) {
		out ^= reinterpret_cast<size_t>( tex.second ) << ( i + 8 );
	}
	return out;
}

MaterialInstance::MaterialInstance( Renderable& renderable, Material * material, Proberties * proberties, const Color * color,
									const std::vector<ShaderSamplerDesc>& customSampler, const std::vector<ShaderSRV>& customSRV )
	: m_Material( material )
	, m_Renderable( &renderable ) {
	if( !m_Material )
		m_Material = s_MaterialManager->GetDefaultMaterial();
	if( proberties )
		m_Proberties = *proberties;
	else
		m_Proberties = m_Material->GetProberties();
	if( color )
		m_Color = *color;
	else
		m_Color = m_Material->GetColor();

	if( !customSampler.empty() ) {
		for( auto sampler : customSampler ) {
			SamplerState* samplerState = Game::GetRenderBackend().GetSamplerState( sampler.SamplerDesc );
			m_CustomSamplers.push_back( { sampler.BindToShader, sampler.Position, samplerState } );
			const std::vector<SamplerState*>& defaultSamplers = m_Material->GetSampler( sampler.BindToShader );
			if( sampler.Position < defaultSamplers.size() )
				m_DefaultSamplers.push_back( { sampler.BindToShader, sampler.Position, defaultSamplers[sampler.Position] } );
		}
	}
	if( !customSRV.empty() ) {
		for( auto srv : customSRV ) {
			m_CustomSRVs.emplace_back( srv );
			const std::vector<ShaderResourceView*>& defaultSRVs = m_Material->GetTextures( srv.BindToShader );
			if( srv.Position < defaultSRVs.size() )
				m_DefaultSRVs.emplace_back( srv );
		}
	}

	m_Material->GetRenderPass()->Assign( m_Renderable );
}

MaterialInstance::~MaterialInstance() {
	m_Material->GetRenderPass()->Deassign( *m_Renderable );
}

void MaterialInstance::BindSamplers() {
	RenderBackend* renderBackend = &Game::GetRenderBackend();
	for( auto& sampler : m_CustomSamplers ) {
		switch( sampler.ShaderType ) {
			case ShaderType::VertexShader:
				renderBackend->SetSamplerVS( sampler.Position, { sampler.SamplerState } );
				break;
			case ShaderType::PixelShader:
				renderBackend->SetSamplerPS( sampler.Position, { sampler.SamplerState } );
				break;
			case ShaderType::GeometryShader:
				renderBackend->SetSamplerGS( sampler.Position, { sampler.SamplerState } );
				break;
			case ShaderType::HullShader:
				renderBackend->SetSamplerHS( sampler.Position, { sampler.SamplerState } );
				break;
			case ShaderType::DomainShader:
				renderBackend->SetSamplerDS( sampler.Position, { sampler.SamplerState } );
				break;
			default:
				break;
		}
	}
}

void MaterialInstance::RestoreMaterialSampler() {
	RenderBackend* renderBackend = &Game::GetRenderBackend();
	for( auto& sampler : m_DefaultSamplers ) {
		switch( sampler.ShaderType ) {
			case ShaderType::VertexShader:
				renderBackend->SetSamplerVS( sampler.Position, { sampler.SamplerState } );
				break;
			case ShaderType::PixelShader:
				renderBackend->SetSamplerPS( sampler.Position, { sampler.SamplerState } );
				break;
			case ShaderType::GeometryShader:
				renderBackend->SetSamplerGS( sampler.Position, { sampler.SamplerState } );
				break;
			case ShaderType::HullShader:
				renderBackend->SetSamplerHS( sampler.Position, { sampler.SamplerState } );
				break;
			case ShaderType::DomainShader:
				renderBackend->SetSamplerDS( sampler.Position, { sampler.SamplerState } );
				break;
			default:
				break;
		}
	}
}

void MaterialInstance::BindTextures() {
	RenderBackend* renderBackend = &Game::GetRenderBackend();
	for( auto& srv : m_CustomSRVs ) {
		switch( srv.BindToShader ) {
			case ShaderType::VertexShader:
				renderBackend->SetSRVsVS( srv.Position, { srv.Resource } );
				break;
			case ShaderType::PixelShader:
				renderBackend->SetSRVsPS( srv.Position, { srv.Resource } );
				break;
			case ShaderType::GeometryShader:
				renderBackend->SetSRVsGS( srv.Position, { srv.Resource } );
				break;
			case ShaderType::HullShader:
				renderBackend->SetSRVsHS( srv.Position, { srv.Resource } );
				break;
			case ShaderType::DomainShader:
				renderBackend->SetSRVsDS( srv.Position, { srv.Resource } );
				break;
			default:
				break;
		}
	}
}

void MaterialInstance::RestoreMaterialTextures() {
	RenderBackend* renderBackend = &Game::GetRenderBackend();
	for( auto& srv : m_DefaultSRVs ) {
		switch( srv.BindToShader ) {
			case ShaderType::VertexShader:
				renderBackend->SetSRVsVS( srv.Position, { srv.Resource } );
				break;
			case ShaderType::PixelShader:
				renderBackend->SetSRVsPS( srv.Position, { srv.Resource } );
				break;
			case ShaderType::GeometryShader:
				renderBackend->SetSRVsGS( srv.Position, { srv.Resource } );
				break;
			case ShaderType::HullShader:
				renderBackend->SetSRVsHS( srv.Position, { srv.Resource } );
				break;
			case ShaderType::DomainShader:
				renderBackend->SetSRVsDS( srv.Position, { srv.Resource } );
				break;
			default:
				break;
		}
	}
}

void MaterialInstance::PrepareRender() {
	if( m_Material->GetID() != Game::GetMaterialManager().GetCurrentMaterialID() ) {
		m_Material->PrepareRender();
		Game::GetMaterialManager().SetCurrentMaterialID( m_Material->GetID() );
	}
	BindSamplers();
	BindTextures();
}

void MaterialInstance::PostRender() {
	RestoreMaterialSampler();
	RestoreMaterialTextures();
}

#include "Shader.h"

#include <d3dcompiler.h>

#include "D3DRenderBackend.h"
#include "Makros.h"
#include "Game.h"

ShaderManager* Shader::s_ShaderManager = nullptr;

void Shader::SetShader() {
	ID3D11DeviceContext* context = &Game::GetContext();
	switch( m_ShaderType ) {
		case ShaderType::VertexShader:
			context->VSSetShader( m_ShaderFile, nullptr, 0 );
			context->IASetInputLayout( m_InputLayout );
			break;
		case ShaderType::PixelShader:
			context->PSSetShader( m_ShaderFile, nullptr, 0 );
			break;
		case ShaderType::GeometryShader:
			context->GSSetShader( m_ShaderFile, nullptr, 0 );
			break;
		case ShaderType::HullShader:
			context->HSSetShader( m_ShaderFile, nullptr, 0 );
			break;
		case ShaderType::ComputeShader:
			context->CSSetShader( m_ShaderFile, nullptr, 0 );
			break;
		default:
			break;
	}
}

void Shader::ResetShaders() {
	ID3D11DeviceContext* context = &Game::GetContext();
	context->VSSetShader( nullptr, nullptr, 0 );
	context->PSSetShader( nullptr, nullptr, 0 );
	context->GSSetShader( nullptr, nullptr, 0 );
	context->HSSetShader( nullptr, nullptr, 0 );
	context->CSSetShader( nullptr, nullptr, 0 );
}

Shader * Shader::Get( const ShaderInit& init ) {
	return s_ShaderManager->Get( init );
}

Shader * Shader::Get( const std::wstring & shaderName ) {
	return s_ShaderManager->Get( shaderName );
}

Shader::Shader() {
}


Shader::~Shader() {
	// Doesn't matter on which shader to call release
	SRelease( m_ShaderFile.vertexShader );
	// Only set for vertexshader, but tested in SRelease()
	SRelease( m_InputLayout );
}

bool Shader::Init( const ShaderInit& shaderInit, const std::vector<std::wstring>& shaderPaths ) {
	m_HashValue = std::hash<ShaderInit>()( shaderInit );

	ID3D11Device* device = &Game::GetDevice();
	ID3D11DeviceContext* context = &Game::GetContext();

	m_ShaderType = shaderInit.shaderType;
	ID3DBlob* byteCode = nullptr;
	HRESULT hr;
	for( size_t i = 0; i < shaderPaths.size(); ++i ) {
		std::wstring fullPath = shaderPaths[i] + shaderInit.file + L".cso";
		hr = D3DReadFileToBlob( fullPath.c_str(), &byteCode );
		if( SUCCEEDED( hr ) )
			break;
	}
	if( FAILED( hr ) ) {
		SRelease( byteCode );
		Game::GetLogger().FatalError( L"Couldn't read file " + shaderInit.file );
		return false;
	}

	switch( shaderInit.shaderType ) {
		case ShaderType::VertexShader:
			hr = device->CreateVertexShader( byteCode->GetBufferPointer(), byteCode->GetBufferSize(), nullptr, &m_ShaderFile.vertexShader );
			break;
		case ShaderType::PixelShader:
			hr = device->CreatePixelShader( byteCode->GetBufferPointer(), byteCode->GetBufferSize(), nullptr, &m_ShaderFile.pixelShader );
			break;
		case ShaderType::GeometryShader:
			hr = device->CreateGeometryShader( byteCode->GetBufferPointer(), byteCode->GetBufferSize(), nullptr, &m_ShaderFile.geometryShader );
			break;
		case ShaderType::HullShader:
			hr = device->CreateHullShader( byteCode->GetBufferPointer(), byteCode->GetBufferSize(), nullptr, &m_ShaderFile.hullShader );
			break;
		case ShaderType::ComputeShader:
			hr = device->CreateComputeShader( byteCode->GetBufferPointer(), byteCode->GetBufferSize(), nullptr, &m_ShaderFile.computeShader );
			break;
		default:
			break;
	}
	if( FAILED( hr ) ) {
		SRelease( byteCode );
		Game::GetLogger().FatalError( L"Couldn't create shader for " + shaderInit.file );
		return false;
	}

	if( m_ShaderType == ShaderType::VertexShader ) {
		CreateInputLayout( shaderInit, byteCode );
	}

	SRelease( byteCode );
	return true;
}

bool Shader::CreateInputLayout( const ShaderInit& shaderInit, ID3DBlob* byteCode ) {
	HRESULT hr;
	std::vector<D3D11_INPUT_ELEMENT_DESC> d3dDesc;
	if( !shaderInit.customInputDesc.empty() ) {
		d3dDesc = *reinterpret_cast<const std::vector<D3D11_INPUT_ELEMENT_DESC>*>( &shaderInit.customInputDesc );
		hr = Game::GetDevice().CreateInputLayout( d3dDesc.data(), static_cast<UINT>( d3dDesc.size() ), byteCode->GetBufferPointer(), byteCode->GetBufferSize(), &m_InputLayout );
	}
	else {
		// Create Input Layout out of the file
		ID3D11ShaderReflection* vertexShaderReflection;
		hr = D3DReflect( byteCode->GetBufferPointer(), byteCode->GetBufferSize(), IID_ID3D11ShaderReflection, reinterpret_cast<void**>( &vertexShaderReflection ) );
		if( FAILED( hr ) ) {
			Game::GetLogger().FatalError( L"Couldn't reflect Shader file " + shaderInit.file );
			return false;
		}
		D3D11_SHADER_DESC desc;
		vertexShaderReflection->GetDesc( &desc );

		// Read input layout description from shader info
		for( UINT i = 0; i < desc.InputParameters; ++i ) {
			D3D11_SIGNATURE_PARAMETER_DESC pDesc;
			hr = vertexShaderReflection->GetInputParameterDesc( i, &pDesc );
			if( FAILED( hr ) ) {
				Game::GetLogger().FatalError( L"Couldn't reflect Shader file " + shaderInit.file );
				return false;
			}
			// Fill out input element desc
			D3D11_INPUT_ELEMENT_DESC elementDesc;
			elementDesc.SemanticName = pDesc.SemanticName;
			elementDesc.SemanticIndex = pDesc.SemanticIndex;
			elementDesc.InputSlot = 0; // Can only handle one vertex buffer bound
			elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			elementDesc.InstanceDataStepRate = 0;
			elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

			// Determine DXGI format
			if( pDesc.Mask == 1 ) {
				if( pDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32 ) elementDesc.Format = DXGI_FORMAT_R32_UINT;
				else if( pDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32 ) elementDesc.Format = DXGI_FORMAT_R32_SINT;
				else if( pDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
			}
			else if( pDesc.Mask <= 3 ) {
				if( pDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
				else if( pDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
				else if( pDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
			}
			else if( pDesc.Mask <= 7 ) {
				if( pDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
				else if( pDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
				else if( pDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			}
			else if( pDesc.Mask <= 15 ) {
				if( pDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
				else if( pDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
				else if( pDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			}

			// Save element desc
			d3dDesc.push_back( elementDesc );
		}
		hr = Game::GetDevice().CreateInputLayout( d3dDesc.data(), static_cast<UINT>( d3dDesc.size() ), byteCode->GetBufferPointer(), byteCode->GetBufferSize(), &m_InputLayout );
		SRelease( vertexShaderReflection );
	}
	if( FAILED( hr ) ) {
		Game::GetLogger().FatalError( L"Failed creating InputManager Layout for file " + shaderInit.file );
		return false;
	}
	return true;
}



size_t std::hash<ShaderInit>::operator() ( const ShaderInit& shaderInit ) {
	size_t out = std::hash<std::wstring>()( shaderInit.file );
	out ^= std::hash<ShaderType>()( shaderInit.shaderType );
	if( !shaderInit.customInputDesc.empty() && shaderInit.shaderType == ShaderType::VertexShader )
		// Take the last component as hash since the likely hood
		// that it is different is greater than for the first element
		out ^= std::hash<InputDesc>()( shaderInit.customInputDesc.back() );
	return out;
}

ShaderManager::ShaderManager( ID3D11Device* device, const std::vector<std::wstring> shaderPaths )
	: m_Device( device ) {
	Game::SetShaderManager( *this );
	Shader::s_ShaderManager = this;
	m_Device = device;
	m_Device->GetImmediateContext( &m_Context );
	m_ShaderPaths = shaderPaths;
	// Also look into the working directory
	m_ShaderPaths.push_back( L"" );
}


ShaderManager::~ShaderManager() {
	SDeleteMap<size_t,Shader>( m_ExistingShader, []( Shader* shader ) {delete shader; } );
	SRelease( m_Context );
}

ShaderManager& ShaderManager::Init( ID3D11Device * device, const std::vector<std::wstring> shaderPaths ) {
	static ShaderManager shaderManager( device, shaderPaths );
	return shaderManager;
}

Shader * ShaderManager::Get( const ShaderInit& shaderInit ) {
	std::hash<ShaderInit> hs;
	size_t hashCode = hs( shaderInit );
	if( m_ExistingShader.find( hashCode ) != m_ExistingShader.end() )
		return m_ExistingShader.at( hashCode );

	Shader* newShader = new Shader();
	bool success = newShader->Init( shaderInit, m_ShaderPaths );
	if( !success ) {
		delete newShader;
		return nullptr;
	}
	m_ExistingShader[hashCode] = newShader;
	if( shaderInit.customInputDesc.size() == 0 || shaderInit.shaderType != ShaderType::VertexShader )
		m_ShaderByName[shaderInit.file] = newShader;

	return newShader;
}

Shader * ShaderManager::Get( const std::wstring & shaderName ) {
	if( m_ShaderByName.count( shaderName ) > 0 )
		return m_ShaderByName[shaderName];

	ShaderInit init;
	std::wstring type = shaderName.substr( 0, 2 );
	if( type[1] != L's' ) {
		Game::GetLogger().Log( L"Shader", L"Attempt to get shader \"" + shaderName + L"\" failed" );
		return nullptr;
	}
	init.file = shaderName;
	if( type[0] == L'v' )
		init.shaderType = ShaderType::VertexShader;
	else if( type[0] == L'p' )
		init.shaderType = ShaderType::PixelShader;
	else if( type[0] == L'g' )
		init.shaderType = ShaderType::GeometryShader;
	else if( type[0] == L'd' )
		init.shaderType = ShaderType::DomainShader;
	else if( type[0] == L'h' )
		init.shaderType = ShaderType::HullShader;
	else if( type[0] == L'c' )
		init.shaderType = ShaderType::ComputeShader;

	return Get( init );
}

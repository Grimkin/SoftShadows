#pragma once
#include "Logger.h"
#include "D3DWrapper.h"

#include <map>
#include <vector>


union ShaderFile {
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11GeometryShader* geometryShader;
	ID3D11HullShader* hullShader;
	ID3D11ComputeShader* computeShader;
	operator ID3D11VertexShader* ( ) const {
		return vertexShader;
	}
	operator ID3D11PixelShader* ( ) const {
		return pixelShader;
	}
	operator ID3D11GeometryShader* ( ) const {
		return geometryShader;
	}
	operator ID3D11HullShader* ( ) const {
		return hullShader;
	}
	operator ID3D11ComputeShader* ( ) const {
		return computeShader;
	}
};


struct ShaderInit {
	std::wstring file;
	ShaderType shaderType;
	std::vector<InputDesc> customInputDesc;
};


class Shader {
	friend class ShaderManager;
public:
	void SetShader();
	ShaderType GetShaderType() {
		return m_ShaderType;
	}
	size_t GetHashValue() const{
		return m_HashValue;
	}

	static void ResetShaders();
	static Shader* Get( const ShaderInit& init );
	static Shader* Get( const std::wstring& shaderName );
private:
	Shader();
	virtual ~Shader();

	bool Init( const ShaderInit& shaderInit, const std::vector<std::wstring>& shaderPaths );
	bool CreateInputLayout( const ShaderInit& shaderInit, ID3DBlob* byteCode );

	ShaderFile m_ShaderFile;
	ShaderType m_ShaderType;
	ID3D11InputLayout *m_InputLayout;

	size_t m_HashValue;

	static ShaderManager* s_ShaderManager;
};



class ShaderManager {
	friend class Game;
public:
	static ShaderManager& Init( ID3D11Device* device, const std::vector<std::wstring> shaderPaths );

	Shader* Get( const ShaderInit& shaderInit );
	Shader* Get( const std::wstring& shaderName );

	ID3D11Device* GetDevice() {
		return m_Device;
	}
	ID3D11DeviceContext* GetContext() {
		return m_Context;
	}
	const std::vector<std::wstring>& GetPaths() {
		return m_ShaderPaths;
	}
private:
	ShaderManager( ID3D11Device* device, const std::vector<std::wstring> shaderPaths );
	virtual ~ShaderManager();

	ID3D11Device* m_Device;
	ID3D11DeviceContext* m_Context;
	std::unordered_map<size_t, Shader*> m_ExistingShader;
	std::unordered_map<std::wstring, Shader*> m_ShaderByName;
	std::vector<std::wstring> m_ShaderPaths;
};

namespace std {
	template<>
	class hash<Shader> {
	public:
		size_t operator() ( const Shader& shader ) {
			return shader.GetHashValue();
		}
	};
	template<>
	class hash<ShaderInit> {
	public:
		size_t operator() ( const ShaderInit& shaderInit );
	};
}
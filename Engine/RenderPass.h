#pragma once
#include <vector>
#include <unordered_map>
#include <functional>

#include "D3DWrapper.h"
#include "Shader.h"

class Shader;
class RenderBackend;
class Logger;
class Renderable;

struct RasterizerDesc;
struct DepthStencilDesc;
struct BlendDesc;

enum class RenderFlag {
	None = 0
};

enum class RenderPassType {
	Opaque,
	Transparency,
	Post,
	Manual
};

inline RenderFlag operator|( RenderFlag flag1, RenderFlag flag2 ) {
	typedef std::underlying_type<RenderFlag>::type enum_type;
	return static_cast<RenderFlag>( static_cast<enum_type>( flag1 ) | static_cast<enum_type>( flag2 ) );
}

struct RenderPassInit {
	std::wstring Name = L"";
	Shader* VertexShader = nullptr;
	Shader* PixelShader = nullptr;
	Shader* GeometryShader = nullptr;
	Shader* HullShader = nullptr;
	Shader* DomainShader = nullptr;
	RasterizerDesc RasterizerDesc;
	DepthStencilDesc DepthStencilDesc;
	BlendDesc BlendDesc;
	std::function<void()> PreFunction = nullptr;
	RenderPassType RenderPassType = RenderPassType::Opaque;
	int8_t RenderPassOrderIndex = 0;
	bool ShowTiming = false;
	std::wstring TimingName = L"";
};


namespace std {
	template<>
	class hash<RenderPassInit> {
	public:
		size_t operator() ( const RenderPassInit& renderPassInit ) {
			size_t out = 0;
			out ^= std::hash<RasterizerDesc>()( renderPassInit.RasterizerDesc );
			out ^= std::hash<DepthStencilDesc>()( renderPassInit.DepthStencilDesc ) << 4;
			out ^= std::hash<BlendDesc>()( renderPassInit.BlendDesc ) << 8;
			if( renderPassInit.VertexShader )
				out ^= renderPassInit.VertexShader->GetHashValue() << 12;
			if( renderPassInit.PixelShader )
				out ^= renderPassInit.PixelShader->GetHashValue() << 16;
			if( renderPassInit.GeometryShader )
				out ^= renderPassInit.GeometryShader->GetHashValue() << 20;
			if( renderPassInit.HullShader )
				out ^= renderPassInit.HullShader->GetHashValue() << 24;
			if( renderPassInit.DomainShader )
				out ^= renderPassInit.DomainShader->GetHashValue() << 28;
			return out;
		}
	};
}

class RenderPass {
	friend class RenderPassManager;
public:
	void Execute( RenderFlag flags );
	void ExecuteDraw( RenderFlag flags, uint32_t numVertices );

	void Apply( RenderFlag flags );

	void Assign( Renderable* renderable );
	void Deassign( const Renderable& renderable );

	size_t GetHashValue() {
		return m_HashValue;
	}

	bool ShowTiming() {
		return m_ShowTiming;
	}

	std::wstring& GetTimingName() {
		return m_TimingName;
	}

	static RenderPass* Create( const RenderPassInit& init );
	static RenderPass* Get( const std::wstring& name );
private:
	RenderPass( RenderPassInit& init );
	virtual ~RenderPass();

	//needed for unique pointer
	struct Deleter {
		void operator()( RenderPass *pass ) const {
			delete pass;
		}
	};

	bool InitPipeline( RenderPassInit& init );
	void RenderObject( Renderable& renderable );

	std::wstring m_Name;
	std::vector<Shader*> m_Shaders;
	RasterizerState* m_RasterizerState;
	DepthStencilState* m_DepthStencilState;
	BlendState* m_BlendState;
	std::function<void()> m_PreFunction;

	std::vector<Renderable*> m_AssignedRenderables;
	std::unordered_map<const Renderable*, uint32_t> m_RenderablePosition;

	size_t m_HashValue;

	bool m_ShowTiming;
	std::wstring m_TimingName = L"";

	static RenderPassManager* s_RenderPassManager;
};

class RenderPassManager {
public:
	static RenderPassManager& Init();

	RenderPass* GetPass( std::wstring uniqueName );
	RenderPass* CreatePass( RenderPassInit renderPassInit );

private:
	RenderPassManager();
	~RenderPassManager();

	std::unordered_map<std::wstring, RenderPass*> m_RenderPasses;
	std::unordered_map<size_t, RenderPass*> m_UniqueRenderPasses;
};


#pragma once
#include "D3DWrapper.h"
#include "RenderBackend.h"
#include "ConstantBufferTypes.h"
#include "Game.h"

template<typename BufferType>
class ConstantBuffer {
public:
	ConstantBuffer() {
		BufferDesc desc;
		desc.Usage = Usage::Dynamic;
		desc.BindFlags = BindFlag::ConstantBuffer;
		desc.ByteWidth = sizeof( BufferType );
		desc.CPUAccessFlags = CPUAccessFlag::Write;

		m_Buffer = Game::GetRenderBackend().CreateBuffer( nullptr, desc );
	}
	~ConstantBuffer() {
	}

	void Update( BufferType& data ) {
		Game::GetRenderBackend().UpdateBuffer( m_Buffer, &data, sizeof( BufferType ), 0, MapType::WriteDiscard );
	}
	void Bind( ShaderFlag bindToShader, uint32_t position ) {
		Game::GetRenderBackend().BindConstantBuffer( m_Buffer, bindToShader, position );
	}
private:
	Buffer* m_Buffer;
};
#include "Renderable.h"
#include "Geometry.h"
#include "RenderPass.h"
#include "GameObject.h"
#include "GameObjectManager.h"
#include "Material.h"

Renderable::Renderable( const RenderableDesc& desc, GameObject& owner )
	: m_Geometry( desc.Geometry )
	, m_Owner( owner) {
	m_MaterialInstance = new MaterialInstance( *this, desc.Material, desc.Proberties, desc.Color, desc.CustomSampler, desc.CustomSRVs );
}

Renderable::~Renderable() {
	for( RenderPass* pass : m_AssignedRenderPasses ) {
		pass->Deassign( *this );
	}
	m_AssignedRenderPasses.clear();
	delete m_MaterialInstance;
}

void Renderable::AssignToRenderpass( RenderPass & pass ) {
	pass.Assign( this );
	m_AssignedRenderPasses.push_back( &pass );
}

void Renderable::PrepareForRender() {
	m_Geometry->PrepareForRender();
	m_MaterialInstance->PrepareRender();
	ObjectData data;
	data.WorldMat = m_Owner.GetTransform().GetWorldTransMat();
	data.Proberties = m_MaterialInstance->GetProberties();
	data.Color = m_MaterialInstance->GetColor();
	Game::GetGOManager().SetObjectData( data );
}

void Renderable::Render() {
	m_Geometry->Draw();
}

void Renderable::PostRender() {
	m_MaterialInstance->PostRender();
}

void Renderable::SetGeometry( Geometry * geometry ) {
}

void Renderable::SetColor( const Color & color ) {
	m_MaterialInstance->SetColor( color );
}

#pragma once

#include <vector>
#include "Types.h"

class Geometry;
class DynamicGeometry;
class Material;
class GameObject;
class RenderBackend;

class DebugElement {
public:
	virtual void Delete() = 0;
protected:
	GameObject* m_GameObject;
};

class DebugCube : DebugElement{
	friend class DebugElementsManager;
public:
	static DebugCube* Create( const float3& position, const float3& size, const Color& color = { .0f, 1.0f, .0f,1.0f } );
	void Delete() override;
private:
	DebugCube( const float3& position, const float3& size, const Color& color = { .0f, 1.0f, .0f, 1.0f } );
	virtual ~DebugCube();

	static void CreateGeometry();

	GameObject* m_GameObject;

	static Geometry* s_Geometry;

	static DebugElementsManager* s_Manager;
};

class DebugLine : DebugElement{
	friend class DebugElementsManager;
public:
	static DebugLine* Create( const float3& startPosition, const float3& endPosition, const Color& color = { 0.f,1.f,0.f,1.0f } );
	void Delete() override;

	const float3& GetStartPosition() {
		return m_StartPosition;
	}
	const float3& GetEndPosition() {
		return m_EndPosition;
	}
private:
	DebugLine( const float3& startPosition, const float3& endPosition, const Color& color = { 0.f,1.f,0.f,1.0f } );
	virtual ~DebugLine();

	float3 m_StartPosition;
	float3 m_EndPosition;

	static Geometry* s_Geometry;

	static DebugElementsManager* s_Manager;
};

class DebugElementsManager {
public:
	static DebugElementsManager& Init( RenderBackend& renderBackend );

	void RegisterCube( DebugCube& cube );
	void RegisterLine( DebugLine& line );

	void DeleteCube( DebugCube& cube );
	void DeleteLine( DebugLine& line );

	void UpdateLineBuffer();

	Material* GetDebugMaterial() {
		return m_Material;
	}
	void SetLineColor( const Color& color );
private:
	DebugElementsManager( RenderBackend& renderBackend );
	~DebugElementsManager();

	const uint32_t m_MaximumLines = 100;
	Buffer* m_LinePointBuffer;

	std::vector<DebugCube*> m_DebugCubes;
	std::vector<DebugLine*> m_DebugLines;

	RenderBackend* m_RenderBackend;
	Material* m_Material;
	DynamicGeometry* m_LineGeometry;
	GameObject* m_LineGameObject;
};


#pragma once

#include "Component.h"
#include "Mesh.h"
#include <DirectXMath.h>
#include "Shader.h"
#include "Texture.h"
#include "Resources.h"
#include "Material.h"

class RendererManager;

class RendererBase : public Component
{
#ifdef EDITOR
	friend class EditorEntityWindow;
#endif
	friend RendererManager;
	friend class Engine;
	friend Material;
public:
	virtual void PrepareForRendering() = 0;

	//This functions are used by Render Queues. DON'T CALL FROM ELSEWEHERE.
	//Binded material will be used.
	virtual void Render(unsigned int subMeshIndex) = 0;

	//Custom material needed.
	virtual void Render(unsigned int subMeshIndex, const Material* material) = 0;

	RendererBase* Clone() = 0;

	//materialIndex = index of the sub mesh, first index is 0.
	void SetMaterial(Material* material, unsigned int materialIndex);

	const std::vector<Material*>& GetMaterials() const;
public:
	bool castShadows = true;
protected:
	RendererManager* rendererManager;
	std::vector<Material*> materials;
};

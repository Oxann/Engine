#pragma once
#include "Component.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ResourceBase.h"

#include <vector>
#include <unordered_map>
#include <DirectXCollision.h>

//Loaded meshes can be accessible through model objects.
//Look Model::FindMesh
class Mesh : public ResourceBase
{
	friend class Model;
	friend class Material;
public:
	class SubMesh
	{
	public:
		void SetVertexCount(unsigned int count);
		void SetIndexBuffer(unsigned short* data, unsigned int indexCount);
		void AddNewVertexElement(const void* data, VertexBuffer::ElementType type);
		
		//If the mesh has no element of the specified type, function will return nullptr.
		const VertexBuffer* GetVertexElement(VertexBuffer::ElementType) const;
		const IndexBuffer* GetIndexBuffer() const;

		unsigned int GetIndexCount() const;
		unsigned int GetVertexCount() const;
		size_t GetElementCount() const;

		bool HasVertexElement(VertexBuffer::ElementType type) const;
	public:		
		DirectX::BoundingBox AABB;
		std::vector<DirectX::XMFLOAT3> positions;
		std::vector<unsigned short> indices;
	private:
		std::unordered_map<VertexBuffer::ElementType,VertexBuffer> vertexElements;
		std::unique_ptr<IndexBuffer> indexBuffer = nullptr;
		unsigned int indexCount = 0u;
		unsigned int vertexCount = 0u;
	};
public:
	size_t GetSubMeshCount() const;
	const std::vector<SubMesh>& GetSubMeshes() const;
	SubMesh& AddNewSubMesh();
	static void Init();
private:
	static void GenerateCube();
	static void GenerateSphere();
	static void GenerateQuad();
public:
	inline static Mesh* Cube;
	inline static Mesh* Sphere;
	inline static Mesh* Quad;
public:
	DirectX::BoundingBox AABB;
private:
	std::vector<SubMesh> subMeshes;
};
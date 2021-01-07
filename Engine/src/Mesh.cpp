#include "Mesh.h"
#include <DirectXMath.h>
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "EngineAssert.h"


void Mesh::SubMesh::SetVertexCount(unsigned int count)
{
    vertexCount = count;
}

void Mesh::SubMesh::SetIndexBuffer(unsigned short* data, unsigned int indexCount)
{
    this->indexCount = indexCount;
    indexBuffer = std::make_unique<IndexBuffer>(data, indexCount);
}

void Mesh::SubMesh::AddNewVertexElement(const void* data, VertexBuffer::ElementType type)
{
    ENGINEASSERT(vertexCount != 0, "Vertex count must be set before adding new vertex elements.");
    vertexElements.insert({ type, { data, vertexCount, type } });
}

const VertexBuffer* Mesh::SubMesh::GetVertexElement(VertexBuffer::ElementType type) const
{
    auto result = vertexElements.find(type);
    return (result == vertexElements.end()) ? nullptr : &result->second;
}

const IndexBuffer* Mesh::SubMesh::GetIndexBuffer() const
{
    return indexBuffer.get();
}

unsigned int Mesh::SubMesh::GetIndexCount() const
{
    return indexCount;
}

unsigned int Mesh::SubMesh::GetVertexCount() const
{
    return vertexCount;
}

size_t Mesh::SubMesh::GetElementCount() const
{
    return vertexElements.size();
}

bool Mesh::SubMesh::HasPositions() const
{
    return vertexElements.find(VertexBuffer::ElementType::Position3D) != vertexElements.end();
}

bool Mesh::SubMesh::HasNormals() const
{
    return vertexElements.find(VertexBuffer::ElementType::Normal) != vertexElements.end();
}

bool Mesh::SubMesh::HasTextureCoords() const
{
    return vertexElements.find(VertexBuffer::ElementType::TexCoord) != vertexElements.end();
}

size_t Mesh::GetSubMeshCount() const
{
    return submeshes.size();
}

const std::vector<Mesh::SubMesh>& Mesh::GetSubMeshes() const
{
    return submeshes;
}

Mesh::SubMesh& Mesh::AddNewSubMesh()
{
    submeshes.push_back({});
    return submeshes.back();
}

void Mesh::Init()
{
    GenerateCube();
    GenerateSphere();
    GenerateQuad();
}

void Mesh::GenerateCube()
{
    std::vector<DirectX::XMFLOAT3> cube_Positions = {
        //Front
        { -0.5f,  0.5f, -0.5f },
        {  0.5f,  0.5f, -0.5f },
        {  0.5f, -0.5f, -0.5f },
        { -0.5f, -0.5f, -0.5f },
        //Back
        {  0.5f,  0.5f,  0.5f },
        { -0.5f,  0.5f,  0.5f },
        { -0.5f, -0.5f,  0.5f },
        {  0.5f, -0.5f,  0.5f },
        //Right
        {  0.5f,  0.5f, -0.5f },
        {  0.5f,  0.5f,  0.5f },
        {  0.5f, -0.5f,  0.5f },
        {  0.5f, -0.5f, -0.5f },
        //Left
        {  -0.5f,  0.5f,  0.5f },
        {  -0.5f,  0.5f, -0.5f },
        {  -0.5f, -0.5f, -0.5f },
        {  -0.5f, -0.5f,  0.5f },
        //Top
        {  -0.5f,  0.5f,  0.5f },
        {   0.5f,  0.5f,  0.5f },
        {   0.5f,  0.5f, -0.5f },
        {  -0.5f,  0.5f, -0.5f },
        //Bottom
        {  -0.5f, -0.5f, -0.5f },
        {   0.5f, -0.5f, -0.5f },
        {   0.5f, -0.5f,  0.5f },
        {  -0.5f, -0.5f,  0.5f }
    };

    std::vector<DirectX::XMFLOAT2> cube_texCoords = {
        //Front
        { 0.0f,0.0f },
        { 1.0f,0.0f },
        { 1.0f,1.0f },
        { 0.0f,1.0f },
        //Back
        { 0.0f,0.0f },
        { 1.0f,0.0f },
        { 1.0f,1.0f },
        { 0.0f,1.0f },
        //Right
        { 0.0f,0.0f },
        { 1.0f,0.0f },
        { 1.0f,1.0f },
        { 0.0f,1.0f },
        //Left
        { 0.0f,0.0f },
        { 1.0f,0.0f },
        { 1.0f,1.0f },
        { 0.0f,1.0f },
        //Top
        { 0.0f,0.0f },
        { 1.0f,0.0f },
        { 1.0f,1.0f },
        { 0.0f,1.0f },
        //Bottom
        { 0.0f,0.0f },
        { 1.0f,0.0f },
        { 1.0f,1.0f },
        { 0.0f,1.0f }
    };

    std::vector<DirectX::XMFLOAT3> cube_Normals = {
        //Front
        { 0.0f,0.0f,-1.0f },
        { 0.0f,0.0f,-1.0f },
        { 0.0f,0.0f,-1.0f },
        { 0.0f,0.0f,-1.0f },
        //Back
        { 0.0f,0.0f,1.0f },
        { 0.0f,0.0f,1.0f },
        { 0.0f,0.0f,1.0f },
        { 0.0f,0.0f,1.0f },
        //Right
        { 1.0f,0.0f,0.0f },
        { 1.0f,0.0f,0.0f },
        { 1.0f,0.0f,0.0f },
        { 1.0f,0.0f,0.0f },
        //Left
        { -1.0f,0.0f,0.0f },
        { -1.0f,0.0f,0.0f },
        { -1.0f,0.0f,0.0f },
        { -1.0f,0.0f,0.0f },
        //Top
        { 0.0f,1.0f,0.0f },
        { 0.0f,1.0f,0.0f },
        { 0.0f,1.0f,0.0f },
        { 0.0f,1.0f,0.0f },
        //Bottom
        { 0.0f,-1.0f,0.0f },
        { 0.0f,-1.0f,0.0f },
        { 0.0f,-1.0f,0.0f },
        { 0.0f,-1.0f,0.0f },
    };

    std::vector<unsigned short> cube_indices = {
        //Front
        0,1,3, 1,2,3,
        //Back
        4,5,7, 5,6,7,
        //Right
        8,9,11, 9,10,11,
        //Left
        12,13,15, 13,14,15,
        //Top
        16,17,19, 17,18,19,
        //Bottom
        20,21,23, 21,22,23
    };  

    Mesh::Cube = new Mesh;
    SubMesh& newSubMesh = Cube->AddNewSubMesh();
    newSubMesh.SetVertexCount(cube_Positions.size());
    newSubMesh.AddNewVertexElement(cube_Positions.data(), VertexBuffer::ElementType::Position3D);
    newSubMesh.AddNewVertexElement(cube_texCoords.data(), VertexBuffer::ElementType::TexCoord);
    newSubMesh.AddNewVertexElement(cube_Normals.data(), VertexBuffer::ElementType::Normal);
    newSubMesh.SetIndexBuffer(cube_indices.data(), cube_indices.size());
}

void Mesh::GenerateSphere()
{
    //http://www.songho.ca/opengl/gl_sphere.html

    const unsigned int latitudeCount = 18u;
    const unsigned int longitudeCount = 36u;
    const float latiudeStep = DirectX::XM_PI / (float)latitudeCount;
    const float longitudeStep = 2.0f * DirectX::XM_PI / (float)longitudeCount;
    const float radius = 0.5f;

    std::vector<DirectX::XMFLOAT3> sphere_pos_vertices;
    std::vector<DirectX::XMFLOAT2> sphere_texCoord_vertices;
    std::vector<DirectX::XMFLOAT3> sphere_normal_vertices;


    for (int i = 0; i <= latitudeCount; i++)
    {
        DirectX::XMFLOAT3 vertex_position;

        const float latitudeAngle = DirectX::XM_PI / 2.0f - i * latiudeStep;
        vertex_position.y = radius * std::sinf(latitudeAngle);
        const float xz = radius * std::cosf(latitudeAngle);

        for (int j = 0; j <= longitudeCount; j++)
        {
            const float longitudeAngle = j * longitudeStep;

            //Position
            vertex_position.x = xz * std::cosf(longitudeAngle);
            vertex_position.z = xz * std::sinf(longitudeAngle);
            sphere_pos_vertices.push_back(vertex_position);

            //Texture coordinates
            sphere_texCoord_vertices.push_back({ (float)j / longitudeCount ,(float)i / latitudeCount });

            //Normals
            DirectX::XMFLOAT3 normal;
            normal.x = vertex_position.x * 1.0f / radius;
            normal.y = vertex_position.y * 1.0f / radius;
            normal.z = vertex_position.z * 1.0f / radius;
            DirectX::XMStoreFloat3(&normal,DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&normal)));
            sphere_normal_vertices.push_back(normal);
        }
    }

    std::vector<unsigned short> sphere_indices;

    for (int i = 0; i < latitudeCount; i++)
    {
        unsigned short topLeft = i * (longitudeCount + 1);
        unsigned short bottomLeft = topLeft + longitudeCount + 1;

        for (int j = 0; j < longitudeCount; j++,topLeft++,bottomLeft++)
        {
            if (i != 0)
            {
                sphere_indices.push_back(topLeft);
                sphere_indices.push_back(topLeft + 1);
                sphere_indices.push_back(bottomLeft);
            }

            if (i != (latitudeCount - 1))
            {
                sphere_indices.push_back(topLeft + 1);
                sphere_indices.push_back(bottomLeft + 1);
                sphere_indices.push_back(bottomLeft);
            }
        }
    }

    Mesh::Sphere = new Mesh;
    SubMesh& newSubMesh = Sphere->AddNewSubMesh();
    newSubMesh.SetVertexCount(sphere_pos_vertices.size());
    newSubMesh.AddNewVertexElement(sphere_pos_vertices.data(), VertexBuffer::ElementType::Position3D);
    newSubMesh.AddNewVertexElement(sphere_texCoord_vertices.data(), VertexBuffer::ElementType::TexCoord);
    newSubMesh.AddNewVertexElement(sphere_normal_vertices.data(), VertexBuffer::ElementType::Normal);
    newSubMesh.SetIndexBuffer(sphere_indices.data(), sphere_indices.size());
}

void Mesh::GenerateQuad()
{
    std::vector<DirectX::XMFLOAT3> positions = {
        {-0.5f,0.5f,0.0f},
        {0.5f,0.5f,0.0f},
        {-0.5f,-0.5f,0.0f},
        {0.5f,-0.5f,0.0f}
    };

    std::vector<DirectX::XMFLOAT2> textureCoords = {
        {0.0f,0.0f},
        {1.0f,0.0f},
        {0.0f,1.0f},
        {1.0f,1.0f}
    };

    std::vector<DirectX::XMFLOAT3> normals = {
    {0.0f,0.0f,-1.0f},
    {0.0f,0.0f,-1.0f},
    {0.0f,0.0f,-1.0f},
    {0.0f,0.0f,-1.0f}
    };

    std::vector<unsigned short> indices = {
        0u,1u,2u,
        1u,3u,2u
    };

    Mesh::Quad = new Mesh;
    SubMesh& newSubMesh = Quad->AddNewSubMesh();
    newSubMesh.SetVertexCount(positions.size());
    newSubMesh.AddNewVertexElement(positions.data(), VertexBuffer::ElementType::Position3D);
    newSubMesh.AddNewVertexElement(textureCoords.data(), VertexBuffer::ElementType::TexCoord);
    newSubMesh.AddNewVertexElement(normals.data(), VertexBuffer::ElementType::Normal);
    newSubMesh.SetIndexBuffer(indices.data(), indices.size());
}

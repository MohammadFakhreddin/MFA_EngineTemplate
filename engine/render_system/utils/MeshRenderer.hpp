#pragma once
#include <memory>

#include "pipeline/FlatShadingPipeline.hpp"
#include "RenderBackend.hpp"
#include "RenderTypes.hpp"
#include "ImportGLTF.hpp"
#include "Collision.hpp"

namespace MFA
{

    class MeshRenderer
    {
    public:

        explicit MeshRenderer(
            std::shared_ptr<FlatShadingPipeline> pipeline,
            std::shared_ptr<AS::GLTF::Model> const& model,
            std::shared_ptr<RT::GpuTexture> errorTexture
        );

        void Render(RT::CommandRecordState& recordState, std::vector<glm::mat4> const& models) const;

        [[nodiscard]]
        std::vector<CollisionTriangle> GetCollisionTriangles(glm::mat4 const& model) const noexcept;

        [[nodiscard]]
        std::vector<glm::vec3> GetVertices(glm::mat4 const& model) const noexcept;

        std::vector<std::vector<std::tuple<int, int>>> GetNeighbors() const noexcept;

    private:

        std::shared_ptr<RT::BufferGroup> GenerateVertexBuffer(VkCommandBuffer cb, AS::GLTF::Model const& model);

        std::shared_ptr<RT::BufferGroup> GenerateIndexBuffer(VkCommandBuffer cb, AS::GLTF::Model const& model);

        std::vector<std::shared_ptr<RT::BufferAndMemory>> GenerateTextures(VkCommandBuffer cb, AS::GLTF::Model const& model);

        std::vector<std::shared_ptr<RT::BufferGroup>> CreateMaterials(VkCommandBuffer cb);

        void CreateDescriptorSets();

        void GenerateCollisionTriangles(AS::GLTF::Model const& model);

        std::shared_ptr<FlatShadingPipeline> _pipeline{};

        std::shared_ptr<AS::GLTF::MeshData> _meshData{};

        std::shared_ptr<RT::GpuTexture> _errorTexture{};

        std::shared_ptr<RT::BufferAndMemory> _verticesBuffer{};
        std::shared_ptr<RT::BufferAndMemory> _indicesBuffer{};
        std::vector<std::shared_ptr<RT::GpuTexture>> _textures{};
        std::vector<std::shared_ptr<RT::BufferGroup>> _materials{};
        std::vector<RT::DescriptorSetGroup> _descriptorSets;

        std::vector<CollisionTriangle> _collisionTriangles{};

        int _vertexCount{};
        std::shared_ptr<Blob> _vertices{};

        int _indexCount{};
        std::shared_ptr<Blob> _indices{};
    };

}
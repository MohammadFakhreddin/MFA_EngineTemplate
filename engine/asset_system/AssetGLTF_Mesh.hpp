#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include "BedrockMemory.hpp"

#include <glm/glm.hpp>

namespace MFA::Asset::GLTF
{

	enum class AlphaMode : uint8_t
	{
		Opaque = 0,
		Mask = 1,
		Blend = 2,
		Invalid = 255
	};

	using Index = uint32_t;

	struct Vertex
	{
	public:
		glm::vec3 position{};
		glm::vec2 baseColorUV{};
		glm::vec2 normalMapUV{};
		glm::vec2 metallicUV{};
		glm::vec2 roughnessUV{};
		glm::vec2 emissionUV{};
		glm::vec2 occlusionUV{};
		glm::vec3 color{};
		glm::vec3 normal{};
		glm::vec3 tangent{};
		int hasSkin = 0;       // Duplicate data
		int jointIndices[4]{ 0, 0, 0, 0 };
		float jointWeights[4]{ 0, 0, 0, 0 };
	};

	struct Primitive
	{
		uint32_t uniqueId = 0;                      // Unique id in entire model
		uint32_t vertexCount = 0;
		uint32_t indicesCount = 0;
		uint64_t verticesOffset = 0;                // From start of buffer
		uint64_t indicesOffset = 0;
		uint32_t verticesStartingIndex = 0;
		uint32_t indicesStartingIndex = 0;          // From start of buffer

		// TODO Separate material
		int baseColorTextureIndex = 0;
		int metallicRoughnessTextureIndex = 0;
		int roughnessTextureIndex = 0;
		int normalTextureIndex = 0;
		int emissiveTextureIndex = 0;
		int occlusionTextureIndex = 0;              // Indicates darker area of texture 
		float baseColorFactor[4]{};
		float metallicFactor = 0;                   // Metallic color is stored inside blue
		float roughnessFactor = 0;                  // Roughness color is stored inside green
		float emissiveFactor[3]{};
		//float occlusionStrengthFactor = 0;        // I couldn't find field in tinygltf     
		bool hasBaseColorTexture = false;
		bool hasMetallicTexture = false;
		bool hasRoughnessTexture = false;
		bool hasEmissiveTexture = false;
		bool hasOcclusionTexture = false;
		bool hasMetallicRoughnessTexture = false;
		bool hasNormalBuffer = false;
		bool hasNormalTexture = false;
		bool hasTangentBuffer = false;
		bool hasSkin = false;

		// TODO Use these render variables to render objects in correct order.

		AlphaMode alphaMode = AlphaMode::Opaque;
		float alphaCutoff = 0.0f;
		bool doubleSided = false;                   // TODO How are we supposed to render double sided objects ?

		bool hasPositionMinMax = false;

		float positionMin[3]{
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max()
		};

		float positionMax[3]{
			std::numeric_limits<float>::min(),
			std::numeric_limits<float>::min(),
			std::numeric_limits<float>::min()
		};
	};

	struct SubMesh
	{
		std::vector<Primitive> primitives{};
		std::vector<Primitive*> blendPrimitives{};
		std::vector<Primitive*> maskPrimitives{};
		std::vector<Primitive*> opaquePrimitives{};

		bool hasPositionMinMax = false;

		float positionMin[3]{
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max()
		};

		float positionMax[3]{
			std::numeric_limits<float>::min(),
			std::numeric_limits<float>::min(),
			std::numeric_limits<float>::min()
		};

		[[nodiscard]]
		std::vector<Primitive*> const& FindPrimitives(AlphaMode alphaMode) const;
	};

	struct MeshData
	{
		std::vector<SubMesh> subMeshes{};

		[[nodiscard]]
		bool IsValid() const;
	};

	class Mesh
	{
	public:

		explicit Mesh(
			uint32_t vertexCount,
			uint32_t indexCount,
			std::shared_ptr<Blob> vertexBuffer,
			std::shared_ptr<Blob> indexBuffer
		);
		~Mesh();

		Mesh(Mesh const&) noexcept = delete;
		Mesh(Mesh&&) noexcept = delete;
		Mesh& operator= (Mesh const& rhs) noexcept = delete;
		Mesh& operator= (Mesh&& rhs) noexcept = delete;

		void FinalizeData();

		// Returns mesh index
		[[nodiscard]]
		uint32_t InsertSubMesh() const;

		void InsertPrimitive(
			uint32_t subMeshIndex,
			Primitive primitive,
			uint32_t vertexCount,
			Vertex* vertices,
			uint32_t indicesCount,
			Index * indices
		);

		[[nodiscard]]
		bool IsValid() const;

		[[nodiscard]]
		std::shared_ptr<MeshData> const& GetMeshData() const;

		[[nodiscard]]
		uint32_t GetVertexCount() const;

		[[nodiscard]]
		std::shared_ptr<Blob> const & GetVertexData() const;

		[[nodiscard]]
		uint32_t GetIndexCount() const;

		[[nodiscard]]
		std::shared_ptr<Blob> const& GetIndexData() const;
		
		// Moves the mesh to the origin
		void CenterMesh();

		// Removes duplicate vertices
		void Optimize();

		[[nodiscard]]
		bool IsCentered() const noexcept;

		[[nodiscard]]
		bool IsOptimized() const noexcept;

	private:

		std::shared_ptr<MeshData> mData{};

		uint64_t mNextVertexOffset{};
		uint64_t mNextIndexOffset{};
		uint32_t mIndicesStartingIndex{};
		uint32_t mVerticesStartingIndex{};

		uint32_t mVertexCount{};
		std::shared_ptr<Blob> mVertexData{}; // TODO: We should separate vertexBuffer and index buffer because most of the time we need them only to create gpu buffers

		uint32_t mIndexCount{};
		std::shared_ptr<Blob> mIndexData{};

		bool mIsCentered = false;
		bool mIsOptimized = false;
	};

}

namespace MFA
{
	namespace AS = Asset;
}
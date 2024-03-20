#include "MeshInstance.hpp"

#include "Transform.hpp"
#include "utils/MeshRenderer.hpp"

namespace MFA
{

	//-------------------------------------------------------------------------------------------------

	MeshInstance::MeshInstance() = default;

	//-------------------------------------------------------------------------------------------------

	MeshInstance::MeshInstance(MeshRenderer const& meshRenderer)
	{
		_nodes = meshRenderer.GetNodes();
	}

	//-------------------------------------------------------------------------------------------------

	MeshInstance::Node* MeshInstance::FindNode(std::string const& name)
	{
		for (auto & node : _nodes)
		{
			if (node.name == name)
			{
				return &node;
			}
		}
		return nullptr;
	}

	//-------------------------------------------------------------------------------------------------

	std::vector<Asset::GLTF::Node> & MeshInstance::GetNodes()
	{
		return _nodes;
	}

	//-------------------------------------------------------------------------------------------------

	Transform & MeshInstance::GetTransform()
	{
		return _transform;
	}

	//-------------------------------------------------------------------------------------------------

	void MeshInstance::SetTransform(const Transform& transform)
	{
		_transform = transform;
	}

	//-------------------------------------------------------------------------------------------------

}

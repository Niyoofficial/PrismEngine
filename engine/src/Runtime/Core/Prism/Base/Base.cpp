#include "pcpch.h"
#include "Base.h"

namespace Prism::Core
{
void InitCore()
{
	Log::InitLog();
}

void ShutdownCore()
{
	Log::ShutdownLog();
}
}

namespace YAML
{
Node convert<glm::vec<2, float>>::encode(const glm::float2& rhs)
{
	Node node;
	node.push_back(rhs.x);
	node.push_back(rhs.y);
	return node;
}

bool convert<glm::vec<2, float>>::decode(const Node& node, glm::float2& rhs)
{
	if (!node.IsSequence() || node.size() != 2)
		return false;
	rhs.x = node[0].as<float>();
	rhs.y = node[1].as<float>();    
	return true;
}

Node convert<glm::vec<3, float>>::encode(const glm::float3& rhs)
{
    Node node;
    node.push_back(rhs.x);
    node.push_back(rhs.y);
    node.push_back(rhs.z);
    return node;
}

bool convert<glm::vec<3, float>>::decode(const Node& node, glm::float3& rhs)
{
    if (!node.IsSequence() || node.size() != 3)
        return false;
    rhs.x = node[0].as<float>();
    rhs.y = node[1].as<float>();
    rhs.z = node[2].as<float>();
    return true;
}

Node convert<glm::vec<4, float>>::encode(const glm::float4& rhs)
{
    Node node;
    node.push_back(rhs.x);
    node.push_back(rhs.y);
    node.push_back(rhs.z);
    node.push_back(rhs.w);
    return node;
}

bool convert<glm::vec<4, float>>::decode(const Node& node, glm::float4& rhs)
{
    if (!node.IsSequence() || node.size() != 4)
        return false;
    rhs.x = node[0].as<float>();
    rhs.y = node[1].as<float>();
    rhs.z = node[2].as<float>();
    rhs.w = node[3].as<float>();
    return true;
}

Node convert<glm::qua<float>>::encode(const glm::quat& rhs)
{
    Node node;
    node.push_back(rhs.x);
    node.push_back(rhs.y);
    node.push_back(rhs.z);
    node.push_back(rhs.w);
    return node;
}

bool convert<glm::qua<float>>::decode(const Node& node, glm::quat& rhs)
{
    if (!node.IsSequence() || node.size() != 4)
        return false;
    rhs.x = node[0].as<float>();
    rhs.y = node[1].as<float>();
    rhs.z = node[2].as<float>();
    rhs.w = node[3].as<float>();
    return true;
}
}

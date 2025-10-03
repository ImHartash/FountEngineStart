#pragma once
#include <string>
#include <unordered_map>
#include <memory>

#include "../../render/mesh/CMesh.hpp"

class IResourceManager {
public:
	static IResourceManager& GetInstance();
	std::shared_ptr<CMesh> GetMesh(const std::string& strFilePath);

private:
	IResourceManager() = default;
	~IResourceManager() = default;

	std::unordered_map<std::string, std::weak_ptr<CMesh>> m_MeshCache;
};
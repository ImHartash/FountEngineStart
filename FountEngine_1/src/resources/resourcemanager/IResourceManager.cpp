#include "IResourceManager.hpp"

#include "../../system/logging/CLogSystem.hpp"

IResourceManager& IResourceManager::GetInstance() {
    static IResourceManager nInstance;
    return nInstance;
}

std::shared_ptr<CMesh> IResourceManager::GetMesh(const std::string& strFilePath) {
    auto it = m_MeshCache.find(strFilePath);
    if (it != m_MeshCache.end()) {
        if (auto mesh = it->second.lock()) {
            return mesh;
        }
    }

    auto newMesh = std::make_shared<CMesh>();
    if (newMesh->LoadFromOBJ(strFilePath)) {
        m_MeshCache[strFilePath] = newMesh;
        return newMesh;
    }

    LOG_WARNING("Failed to get mesh from %s", strFilePath.c_str());
    return nullptr;
}
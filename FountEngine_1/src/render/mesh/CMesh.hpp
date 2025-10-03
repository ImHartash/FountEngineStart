#pragma once
#include <vector>
#include <string>
#include <d3d11.h>

#include "../vertex/Vertex_t.hpp"

class CMesh {
public:
	CMesh();
	~CMesh();

	bool LoadFromOBJ(const std::string& szFilename);
	bool CreateBuffers(ID3D11Device* pDevice);
	void Render(ID3D11DeviceContext* pDeviceContext);

	const std::vector<Vertex_t>& GetVertices() const { return m_Vertices; }
	const std::vector<unsigned int>& GetIndices() const { return m_Indices; }

private:
	std::vector<Vertex_t> m_Vertices;
	std::vector<unsigned int> m_Indices;

	ID3D11Buffer* m_pVertexBuffer = nullptr;
	ID3D11Buffer* m_pIndexBuffer = nullptr;
};
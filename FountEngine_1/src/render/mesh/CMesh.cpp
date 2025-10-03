#include "CMesh.hpp"

#include <fstream>
#include <sstream>

#include "../../system/logging/CLogSystem.hpp"
#include "../../math/CMath.hpp"

CMesh::CMesh() = default;
CMesh::~CMesh() {
	if (m_pVertexBuffer) m_pVertexBuffer->Release();
	if (m_pIndexBuffer) m_pIndexBuffer->Release();
}

bool CMesh::LoadFromOBJ(const std::string& strFilePath) {
	std::ifstream sFile(strFilePath);
	if (!sFile.is_open()) {
		LOG_ERROR("Failed to open file: %s", strFilePath.c_str());
		return false;
	}

	std::vector<CMath::Vector3_t> Positions;
	std::vector<CMath::Vector3_t> Normals;
	std::vector<CMath::Vector2_t> Texcoords;

	std::string strLine;
	while (std::getline(sFile, strLine)) {
		std::istringstream iss(strLine);
		std::string strType;
		iss >> strType;

		if (strType == "v") { // Vertex Position
			float x, y, z;
			iss >> x >> y >> z;
			Positions.emplace_back(x, y, z);
		}
		else if (strType == "vn") { // Vertex Normal Position
			float x, y, z;
			iss >> x >> y >> z;
			Normals.emplace_back(x, y, z);
		}
		else if (strType == "vt") {
			float u, v;
			iss >> u >> v;
			Texcoords.emplace_back(u, v);
		}
		else if (strType == "f") {
			std::string strVertex;
			while (iss >> strVertex) {
				std::istringstream viss(strVertex);
				std::string strIndices;

				Vertex_t Vertex;
				std::getline(viss, strIndices, '/');
				if (!strIndices.empty()) {
					int nPositionIndex = std::stoi(strIndices) - 1;
					Vertex.vec3Position = Positions[nPositionIndex];
				}

				std::getline(viss, strIndices, '/');
				if (!strIndices.empty()) {
					int nTexIndex = std::stoi(strIndices) - 1;
					Vertex.vec2Texcoord = Texcoords[nTexIndex];
				}

				std::getline(viss, strIndices, '/');
				if (!strIndices.empty()) {
					int nNormIndex = std::stoi(strIndices) - 1;
					Vertex.vec3Normal = Normals[nNormIndex];
				}

				m_Vertices.push_back(Vertex);
				m_Indices.push_back(static_cast<unsigned int>(m_Indices.size()));
			}
		}
	}

	LOG_INFO("Successfully loaded mesh from %s", strFilePath.c_str());
	return true;
}

bool CMesh::CreateBuffers(ID3D11Device* pDevice) {
	D3D11_BUFFER_DESC vbd = {};
	vbd.ByteWidth = static_cast<UINT>(sizeof(Vertex_t) * m_Vertices.size());
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vInitData = { };
	vInitData.pSysMem = m_Vertices.data();

	HRESULT hResult = pDevice->CreateBuffer(&vbd, &vInitData, &m_pVertexBuffer);
	if (FAILED(hResult)) {
		LOG_ERROR("Failed to create vertex buffer.");
		return false;
	}

	D3D11_BUFFER_DESC ibd = { };
	ibd.ByteWidth = static_cast<UINT>(sizeof(unsigned int) * m_Indices.size());
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA iInitData = { };
	iInitData.pSysMem = m_Indices.data();

	hResult = pDevice->CreateBuffer(&ibd, &iInitData, &m_pIndexBuffer);
	if (FAILED(hResult)) {
		LOG_ERROR("Failed to create index buffer.");
		return false;
	}

	return true;
}

void CMesh::Render(ID3D11DeviceContext* pDeviceContext) {
	UINT stride = sizeof(Vertex_t);
	UINT offset = 0;

	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDeviceContext->DrawIndexed(static_cast<UINT>(m_Indices.size()), 0, 0);
}
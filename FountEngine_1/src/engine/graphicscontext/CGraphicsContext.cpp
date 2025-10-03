#include "CGraphicsContext.hpp"

#include "../../render/vertex/Vertex_t.hpp"
#include "../../system/logging/CLogSystem.hpp"
#include "../../resources/resourcemanager/IResourceManager.hpp"

CGraphicsContext::CGraphicsContext() = default;
CGraphicsContext::~CGraphicsContext() {
	if (m_pIndexBuffer) m_pIndexBuffer->Release();
	if (m_pConstantBuffer) m_pConstantBuffer->Release();

	if (m_pVertexBuffer) m_pVertexBuffer->Release();
	if (m_pInputLayout) m_pInputLayout->Release();
	if (m_pVertexShader) m_pVertexShader->Release();
	if (m_pPixelShader) m_pPixelShader->Release();

	if (m_pRenderTargetView) m_pRenderTargetView->Release();
	if (m_pSwapChain) m_pSwapChain->Release();
	if (m_pDeviceContext) m_pDeviceContext->Release();
	if (m_pDevice) m_pDevice->Release();
}

bool CGraphicsContext::Initialize(HWND hWnd, int nWidth, int nHeight) {
	m_nWidth = nWidth;
	m_nHeight = nHeight;

	DXGI_SWAP_CHAIN_DESC scd = { 0 };
	scd.BufferCount = 1;
	scd.BufferDesc.Width = nWidth;
	scd.BufferDesc.Height = nHeight;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.RefreshRate.Numerator = 60; // 60 Hz
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = 1;
	scd.Windowed = TRUE;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	UINT uCreateDeviceFlags = 0;

#ifdef _DEBUG
	uCreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL fLevel;
	HRESULT hResult = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		uCreateDeviceFlags,
		nullptr, 0,
		D3D11_SDK_VERSION,
		&scd,
		&m_pSwapChain,
		&m_pDevice,
		&fLevel,
		&m_pDeviceContext
	);

	if (FAILED(hResult)) {
		LOG_ERROR("Failed to create D3D11 device and swap chain!");
		return false;
	}

	ID3D11Texture2D* pBackBuffer = nullptr;
	hResult = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hResult)) {
		LOG_ERROR("Failed to get back buffer!");
		return false;
	}

	hResult = m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hResult)) {
		LOG_ERROR("Failed to create render target view!");
		return false;
	}

	D3D11_BUFFER_DESC dxConstantBufferDesc = {};
	dxConstantBufferDesc.ByteWidth = sizeof(CMath::Matrix4x4_t) * 3;
	dxConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	dxConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	dxConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hResult = m_pDevice->CreateBuffer(&dxConstantBufferDesc, nullptr, &m_pConstantBuffer);
	if (FAILED(hResult)) {
		LOG_ERROR("Failed to create constant buffer!");
		return false;
	}

	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(nWidth);
	viewport.Height = static_cast<float>(nHeight);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	m_pDeviceContext->RSSetViewports(1, &viewport);

	if (!CompileShaders()) {
		return false;
	}

	if (!CreateCubeBuffers()) {
		return false;
	}

	m_Camera.SetPosition(CMath::Vector3_t(0.0f, 0.0f, -3.0f));
	this->LoadMesh("game/models/cube.obj");

	return true;
}

void CGraphicsContext::OnMouseMove(int iDeltaX, int iDeltaY) {
	m_Camera.OnMouseMove(-iDeltaX, iDeltaY);
}

void CGraphicsContext::UpdateCamera(float flDeltaTime, const bool* pKeys) {
	m_Camera.Update(flDeltaTime, pKeys);
}

void CGraphicsContext::BeginFrame() {
	if (!m_pDeviceContext || !m_pRenderTargetView) {
		return;
	}

	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, nullptr);

	const float aClearColor[4] = { 0.2f, 0.4f, 0.7f, 1.0f };
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, aClearColor);
}

void CGraphicsContext::EndFrame() {
	if (!m_pSwapChain) {
		return;
	}

	m_pSwapChain->Present(1, 0);
}

void CGraphicsContext::DrawCube() {
	UpdateMatrices();

	m_pDeviceContext->VSSetShader(m_pVertexShader, nullptr, 0);
	m_pDeviceContext->PSSetShader(m_pPixelShader, nullptr, 0);
	m_pDeviceContext->IASetInputLayout(m_pInputLayout);

	m_pDeviceContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	UINT stride = sizeof(Vertex_t);
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	m_pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->DrawIndexed(m_nIndexCount, 0, 0);

	// RenderMesh(); WHY YOU CRUSHING MY VIDEOCARD??????
}

bool CGraphicsContext::LoadMesh(const std::string& strFilename) {
	m_pCurrentMesh = IResourceManager::GetInstance().GetMesh(strFilename);
	if (m_pCurrentMesh) {
		return CreateMeshBuffers();
	}
	return false;
}

bool CGraphicsContext::CreateMeshBuffers() {
	if (m_pCurrentMesh) {
		return m_pCurrentMesh->CreateBuffers(m_pDevice);
	}
	return false;
}

void CGraphicsContext::RenderMesh() {
	if (m_pCurrentMesh) {
		m_pCurrentMesh->Render(m_pDeviceContext);
	}
}

bool CGraphicsContext::CompileShaders() {
	ID3DBlob* pVsBlob = nullptr;
	HRESULT hResult = D3DCompileFromFile(
		L"game/shaders/shaders.hlsl",
		nullptr, nullptr,
		"VS", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0, &pVsBlob,
		nullptr
	);

	if (FAILED(hResult)) {
		LOG_ERROR("Failed to compile vertex shader!");
		return false;
	}

	hResult = m_pDevice->CreateVertexShader(pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), nullptr, &m_pVertexShader);
	if (FAILED(hResult)) {
		pVsBlob->Release();
		LOG_ERROR("Failed to create vertex shader!");
		return false;
	}

	ID3DBlob* pPsBlob = nullptr;
	hResult = D3DCompileFromFile(
		L"game/shaders/shaders.hlsl",
		nullptr, nullptr,
		"PS", "ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0, &pPsBlob,
		nullptr
	);

	if (FAILED(hResult)) {
		pVsBlob->Release();
		LOG_ERROR("Failed to compile pixel shader!");
		return false;
	}

	hResult = m_pDevice->CreatePixelShader(pPsBlob->GetBufferPointer(), pPsBlob->GetBufferSize(), nullptr, &m_pPixelShader);
	if (FAILED(hResult)) {
		pVsBlob->Release();
		pPsBlob->Release();
		LOG_ERROR("Failed to create pixel shader!");
		return false;
	}

	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12 , D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24 , D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	UINT nElements = ARRAYSIZE(layout);

	hResult = m_pDevice->CreateInputLayout(layout, nElements, pVsBlob->GetBufferPointer(), pVsBlob->GetBufferSize(), &m_pInputLayout);
	pVsBlob->Release();
	pPsBlob->Release();

	if (FAILED(hResult)) {
		LOG_ERROR("Failed to create input layout!");
		return false;
	}

	return true;
}

bool CGraphicsContext::CreateCubeBuffers() {
	Vertex_t vertices[] = {
		{ CMath::Vector3_t(-0.5f,  0.5f, -0.5f), CMath::Vector3_t(0.0f, 0.0f, -1.0f), CMath::Vector2_t(0.0f, 0.0f) },
		{ CMath::Vector3_t(0.5f,  0.5f, -0.5f), CMath::Vector3_t(0.0f, 0.0f, -1.0f), CMath::Vector2_t(1.0f, 0.0f) },
		{ CMath::Vector3_t(0.5f, -0.5f, -0.5f), CMath::Vector3_t(0.0f, 0.0f, -1.0f), CMath::Vector2_t(1.0f, 1.0f) },
		{ CMath::Vector3_t(-0.5f, -0.5f, -0.5f), CMath::Vector3_t(0.0f, 0.0f, -1.0f), CMath::Vector2_t(0.0f, 1.0f) },

		{ CMath::Vector3_t(-0.5f,  0.5f,  0.5f), CMath::Vector3_t(0.0f, 0.0f, 1.0f), CMath::Vector2_t(1.0f, 0.0f) },
		{ CMath::Vector3_t(0.5f,  0.5f,  0.5f), CMath::Vector3_t(0.0f, 0.0f, 1.0f), CMath::Vector2_t(0.0f, 0.0f) },
		{ CMath::Vector3_t(0.5f, -0.5f,  0.5f), CMath::Vector3_t(0.0f, 0.0f, 1.0f), CMath::Vector2_t(0.0f, 1.0f) },
		{ CMath::Vector3_t(-0.5f, -0.5f,  0.5f), CMath::Vector3_t(0.0f, 0.0f, 1.0f), CMath::Vector2_t(1.0f, 1.0f) },

		{ CMath::Vector3_t(-0.5f,  0.5f,  0.5f), CMath::Vector3_t(-1.0f, 0.0f, 0.0f), CMath::Vector2_t(0.0f, 0.0f) },
		{ CMath::Vector3_t(-0.5f,  0.5f, -0.5f), CMath::Vector3_t(-1.0f, 0.0f, 0.0f), CMath::Vector2_t(1.0f, 0.0f) },
		{ CMath::Vector3_t(-0.5f, -0.5f, -0.5f), CMath::Vector3_t(-1.0f, 0.0f, 0.0f), CMath::Vector2_t(1.0f, 1.0f) },
		{ CMath::Vector3_t(-0.5f, -0.5f,  0.5f), CMath::Vector3_t(-1.0f, 0.0f, 0.0f), CMath::Vector2_t(0.0f, 1.0f) },

		{ CMath::Vector3_t(0.5f,  0.5f, -0.5f), CMath::Vector3_t(1.0f, 0.0f, 0.0f), CMath::Vector2_t(0.0f, 0.0f) },
		{ CMath::Vector3_t(0.5f,  0.5f,  0.5f), CMath::Vector3_t(1.0f, 0.0f, 0.0f), CMath::Vector2_t(1.0f, 0.0f) },
		{ CMath::Vector3_t(0.5f, -0.5f,  0.5f), CMath::Vector3_t(1.0f, 0.0f, 0.0f), CMath::Vector2_t(1.0f, 1.0f) },
		{ CMath::Vector3_t(0.5f, -0.5f, -0.5f), CMath::Vector3_t(1.0f, 0.0f, 0.0f), CMath::Vector2_t(0.0f, 1.0f) },

		{ CMath::Vector3_t(-0.5f, 0.5f,  0.5f), CMath::Vector3_t(0.0f, 1.0f, 0.0f), CMath::Vector2_t(0.0f, 0.0f) },
		{ CMath::Vector3_t(0.5f, 0.5f,  0.5f), CMath::Vector3_t(0.0f, 1.0f, 0.0f), CMath::Vector2_t(1.0f, 0.0f) },
		{ CMath::Vector3_t(0.5f, 0.5f, -0.5f), CMath::Vector3_t(0.0f, 1.0f, 0.0f), CMath::Vector2_t(1.0f, 1.0f) },
		{ CMath::Vector3_t(-0.5f, 0.5f, -0.5f), CMath::Vector3_t(0.0f, 1.0f, 0.0f), CMath::Vector2_t(0.0f, 1.0f) },

		{ CMath::Vector3_t(-0.5f, -0.5f, -0.5f), CMath::Vector3_t(0.0f, -1.0f, 0.0f), CMath::Vector2_t(0.0f, 0.0f) },
		{ CMath::Vector3_t(0.5f, -0.5f, -0.5f), CMath::Vector3_t(0.0f, -1.0f, 0.0f), CMath::Vector2_t(1.0f, 0.0f) },
		{ CMath::Vector3_t(0.5f, -0.5f,  0.5f), CMath::Vector3_t(0.0f, -1.0f, 0.0f), CMath::Vector2_t(1.0f, 1.0f) },
		{ CMath::Vector3_t(-0.5f, -0.5f,  0.5f), CMath::Vector3_t(0.0f, -1.0f, 0.0f), CMath::Vector2_t(0.0f, 1.0f) }
	};

	unsigned int indices[] = {
		0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23
	};

	D3D11_BUFFER_DESC vbd = {};
	vbd.ByteWidth = sizeof(vertices);
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA dxVInitData = {};
	dxVInitData.pSysMem = vertices;

	HRESULT hResult = m_pDevice->CreateBuffer(&vbd, &dxVInitData, &m_pVertexBuffer);
	if (FAILED(hResult)) {
		MessageBox(nullptr, "Failed to create vertex buffer!", "Engine Error", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	D3D11_BUFFER_DESC ibd = {};
	ibd.ByteWidth = sizeof(indices);
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA dxIInitData = {};
	dxIInitData.pSysMem = indices;

	hResult = m_pDevice->CreateBuffer(&ibd, &dxIInitData, &m_pIndexBuffer);
	if (FAILED(hResult)) {
		MessageBox(nullptr, "Failed to create index buffer!", "Engine Error", MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	m_nIndexCount = 36;

	return true;
}

void CGraphicsContext::UpdateMatrices() {
	static float flRotationAngle = 0.0f;
	flRotationAngle += 0.005f;

	CMath::Matrix4x4_t mWorld = CMath::Matrix4x4_t::CreateRotationY(flRotationAngle);
	CMath::Matrix4x4_t mView = m_Camera.GetViewMatrix();
	CMath::Matrix4x4_t mProjection = CMath::Matrix4x4_t::CreatePerspectiveFieldOfView(
		3.14159f / 4.0f,
		static_cast<float>(m_nWidth) / static_cast<float>(m_nHeight),
		0.1f,
		100.0f
	);

	D3D11_MAPPED_SUBRESOURCE dxMappedResource;
	HRESULT hResult = m_pDeviceContext->Map(m_pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &dxMappedResource);

	if (FAILED(hResult) || !dxMappedResource.pData) {
		LOG_ERROR("Failed to map constant buffer");
		return;
	}

	CMath::Matrix4x4_t* pMatrices = static_cast<CMath::Matrix4x4_t*>(dxMappedResource.pData);
	pMatrices[0] = mWorld;
	pMatrices[1] = mView;
	pMatrices[2] = mProjection;

	m_pDeviceContext->Unmap(m_pConstantBuffer, 0);
}
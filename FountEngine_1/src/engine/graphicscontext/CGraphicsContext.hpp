#pragma once
#include <windows.h>
#include <string>
#include <memory>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "../../client/camera/CCamera.hpp"
#include "../../render/mesh/CMesh.hpp"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;

class CGraphicsContext {
public:
	CGraphicsContext();
	~CGraphicsContext();

	CGraphicsContext(const CGraphicsContext&) = delete;
	CGraphicsContext& operator=(const CGraphicsContext&) = delete;

	bool Initialize(HWND hWnd, int nWidth, int nHeight);
	void OnMouseMove(int iDeltaX, int iDeltaY);
	void UpdateCamera(float flDeltaTime, const bool* pKeys);

	void BeginFrame();
	void EndFrame();

	void DrawCube();

	bool LoadMesh(const std::string& strFilename);
	bool CreateMeshBuffers();
	void RenderMesh();

private:
	bool CompileShaders();
	bool CreateCubeBuffers();
	void UpdateMatrices();

	int m_nWidth;
	int m_nHeight;

	CCamera m_Camera;
	
	ID3D11Device* m_pDevice = nullptr;
	ID3D11DeviceContext* m_pDeviceContext = nullptr;
	IDXGISwapChain* m_pSwapChain = nullptr;
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;

	ID3D11VertexShader* m_pVertexShader = nullptr;
	ID3D11PixelShader* m_pPixelShader = nullptr;
	ID3D11InputLayout* m_pInputLayout = nullptr;
	ID3D11Buffer* m_pVertexBuffer = nullptr;
	ID3D11Buffer* m_pConstantBuffer = nullptr;

	ID3D11Buffer* m_pIndexBuffer = nullptr;
	UINT m_nIndexCount = 0;

	std::shared_ptr<CMesh> m_pCurrentMesh;
};
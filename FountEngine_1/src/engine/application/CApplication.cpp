#include "CApplication.hpp"

#include <windowsx.h>
#include <chrono>

#include "../../system/logging/CLogSystem.hpp"

static CApplication* g_pApplication = nullptr;

LRESULT CALLBACK OnWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	return g_pApplication->HandleMessage(hWnd, uMsg, wParam, lParam);
}

// Class Realization
CApplication::CApplication() : m_hInstance(nullptr), m_hWnd(nullptr), m_nWidth(0), m_nHeight(0) {
	g_pApplication = this;
}

CApplication::~CApplication() {
	ReleaseCapture();
}

bool CApplication::Initialize(HINSTANCE hInstance, const std::string& strWindowTitle, int nWidth, int nHeight) {
	m_hInstance = hInstance;
	m_strWindowTitle = strWindowTitle;
	m_nWidth = nWidth;
	m_nHeight = nHeight;

	WNDCLASSEX wndClass = { 0 };
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = OnWndProc;
	wndClass.hInstance = hInstance;
	wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndClass.lpszClassName = "EngineMainWindowClass";

	if (!RegisterClassExA(&wndClass)) {
		LOG_ERROR("Failed to register window class!");
		return false;
	}

	RECT rSize = { 0, 0, m_nWidth, m_nHeight };
	AdjustWindowRect(&rSize, WS_OVERLAPPEDWINDOW, FALSE);

	m_hWnd = CreateWindowEx(
		0,
		wndClass.lpszClassName,
		m_strWindowTitle.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rSize.right - rSize.left,
		rSize.bottom - rSize.top,
		nullptr, nullptr,
		m_hInstance, nullptr
	);

	if (!m_hWnd) {
		LOG_ERROR("Failed to create window!");
		return false;
	}

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	SetCapture(m_hWnd);
	
	m_pGraphicsContext = std::make_unique<CGraphicsContext>();
	if (!m_pGraphicsContext->Initialize(m_hWnd, m_nWidth, m_nHeight)) {
		LOG_FATAL("Failed to initialize graphics context!");
		return false;
	}

	LOG_INFO("Application successfully initialized!");
	return true;
}

int CApplication::Run() {
	MSG mMsg = { 0 };

	using clock = std::chrono::high_resolution_clock;
	auto aPrevTime = clock::now();

	while (mMsg.message != WM_QUIT) {
		if (PeekMessage(&mMsg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&mMsg);
			DispatchMessage(&mMsg);
		}
		else {
			auto aCurrentTime = clock::now();
			float flDeltaTime = std::chrono::duration<float>(aCurrentTime - aPrevTime).count();
			aPrevTime = aCurrentTime;

			m_pGraphicsContext->UpdateCamera(flDeltaTime, m_aKeys);

			m_pGraphicsContext->BeginFrame();
			m_pGraphicsContext->DrawCube();
			m_pGraphicsContext->EndFrame();
		}
	}

	return static_cast<int>(mMsg.wParam);
}

LRESULT CApplication::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_LBUTTONDOWN:
		SetCapture(m_hWnd);
		return 0;

	case WM_LBUTTONUP:
		ReleaseCapture();
		return 0;

	case WM_KILLFOCUS:
		ReleaseCapture();
		return 0;

	case WM_KEYDOWN:
		OnKeyDown(wParam);
		return 0;

	case WM_KEYUP:
		OnKeyUp(wParam);
		return 0;
	
	case WM_MOUSEMOVE: {
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);

		static int iPrevX = x;
		static int iPrevY = y;
		int iDeltaX = x - iPrevX;
		int iDeltaY = y - iPrevY;
		iPrevX = x;
		iPrevY = y;

		if (GetCapture() == m_hWnd) {
			m_pGraphicsContext->OnMouseMove(iDeltaX, iDeltaY);
		}

		return 0;
	}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void CApplication::OnKeyDown(WPARAM wKey) {
	m_aKeys[wKey] = true;
}

void CApplication::OnKeyUp(WPARAM wKey) {
	m_aKeys[wKey] = false;
}
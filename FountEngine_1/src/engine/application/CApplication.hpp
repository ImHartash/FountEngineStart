#pragma once
#include <Windows.h>

#include <string>
#include <memory>

#include "../../engine/graphicscontext/CGraphicsContext.hpp"

class CApplication {
public:
	CApplication();
	~CApplication();

	CApplication(const CApplication&) = delete;
	CApplication& operator=(const CApplication&) = delete;

	bool Initialize(HINSTANCE hInstance, const std::string& strWindowTitle, int nWidth, int nHeight);
	int Run();

	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void OnKeyDown(WPARAM wKey);
	void OnKeyUp(WPARAM wKey);

private:
	HINSTANCE m_hInstance;
	HWND m_hWnd;
	std::string m_strWindowTitle;
	int m_nWidth, m_nHeight;

	bool m_aKeys[256] = { false };

	std::unique_ptr<CGraphicsContext> m_pGraphicsContext;
};
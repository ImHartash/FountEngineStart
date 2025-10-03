#include "engine/application/CApplication.hpp"
#include "system/logging/CLogSystem.hpp"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR lpCmdLine, _In_ int nCmdShow) {
	auto& LogSystem = CLogSystem::GetInstance();
	if (!LogSystem.Initialize("engine.log")) {
		MessageBoxA(nullptr, "Failed to initialize log system!", "Initialization Error", MB_OK);
		return -1;
	}

	LOG_DEBUG("Starting Application...");
	LOG_INFO("CLogSystem initialized successfully!");

	CApplication appMain;

	if (!appMain.Initialize(hInstance, "Source Like Engine", 1280, 720)) {
		LOG_FATAL("Failed to initialize application!");
		return -1;
	}

	LOG_DEBUG("Starting application main loop...");
	int iResult = appMain.Run();
	LOG_DEBUG("Application shutdown with code: %d", iResult);
	LogSystem.Shutdown();

	return iResult;
}
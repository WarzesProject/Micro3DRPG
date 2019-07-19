#pragma once

#include "Framework/PlatformTypes.h"

class WindowSDL
{
public:
	bool Init(const char* windowTitle);
	void Close();

	bool ProcessMessages();

	void GetWindowSize(int &width, int &height) const;
	handle GetNativeWindowHandle() const;

	std::function<void()> onDrawRequest;
	std::function<void()> onResize;
	std::function<void()> onToggleFullscreenState;

private:
#ifdef RENDERER_RUNTIME_IMGUI
	void initializeImGuiKeyMap();
	void processImGuiSdl2Event(const SDL_Event &sdlEvent);
	void updateImGuiMousePositionAndButtons();
#endif

	char m_windowTitle[64];
	SDL_Window *m_sdlWindow = nullptr;
#ifdef RENDERER_RUNTIME_IMGUI
	bool m_imGuiMousePressed[3] = { false, false, false };
#endif
};
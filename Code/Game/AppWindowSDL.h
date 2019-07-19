#pragma once

#include "Framework/PlatformTypes.h"

class AppRenderer;

class AppWindowSDL
{
public:
	AppWindowSDL(AppRenderer& application, const char* windowTitle);

	void onInitialization();
	void onDeinitialization();
	[[nodiscard]] bool processMessages();
	void getWindowSize(int &width, int &height) const;
	[[nodiscard]] handle getNativeWindowHandle() const;
	void redraw();
	
private:
#ifdef RENDERER_RUNTIME_IMGUI
	void initializeImGuiKeyMap();
	void processImGuiSdl2Event(const SDL_Event& sdlEvent);
	void updateImGuiMousePositionAndButtons();
#endif

	AppRenderer &mApplication;
	char		  mWindowTitle[64];	
	SDL_Window*	  m_sdlWindow;
#ifdef RENDERER_RUNTIME_IMGUI
	bool m_imGuiMousePressed[3];
#endif
};
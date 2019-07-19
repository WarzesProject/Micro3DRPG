#pragma once

#include "Framework/IApplicationImpl.h"

class IApplication;

class ApplicationImplSdl2 final : public IApplicationImpl
{
public:
	ApplicationImplSdl2(IApplication& application, const char* windowTitle);
	inline virtual ~ApplicationImplSdl2() override
	{
	}
public:
	virtual void onInitialization() override;
	virtual void onDeinitialization() override;
	[[nodiscard]] virtual bool processMessages() override;
	virtual void getWindowSize(int &width, int &height) const override;
	[[nodiscard]] virtual handle getNativeWindowHandle() const override;
	virtual void redraw() override;
	virtual void showUrgentMessage(const char* message, const char* title = "Urgent Message") const override;


private:
	explicit ApplicationImplSdl2(const ApplicationImplSdl2& source) = delete;
	ApplicationImplSdl2& operator =(const ApplicationImplSdl2& source) = delete;
	#ifdef RENDERER_RUNTIME_IMGUI
		void initializeImGuiKeyMap();
		void processImGuiSdl2Event(const SDL_Event& sdlEvent);
		void updateImGuiMousePositionAndButtons();
	#endif

private:
	IApplication *mApplication;			///< The owner application instance, always valid
	char		  mWindowTitle[64];		///< ASCII window title
	SDL_Window*	  m_sdlWindow;			///< SDL2 handle, can be a null handler
	bool		  mFirstUpdate;
	#ifdef RENDERER_RUNTIME_IMGUI
		bool m_imGuiMousePressed[3];
	#endif
};
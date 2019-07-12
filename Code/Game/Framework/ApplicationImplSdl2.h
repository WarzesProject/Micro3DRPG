#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Framework/IApplicationImpl.h"

//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
class IApplication;


//[-------------------------------------------------------]
//[ Classes                                               ]
//[-------------------------------------------------------]
/**
*  @brief
*    Simple DirectMedia Layer" (SDL, https://www.libsdl.org/ ) application implementation class
*/
class ApplicationImplSdl2 final : public IApplicationImpl
{


//[-------------------------------------------------------]
//[ Public methods                                        ]
//[-------------------------------------------------------]
public:
	/**
	*  @brief
	*    Constructor
	*
	*  @param[in] application
	*    The owner application instance
	*  @param[in] windowTitle
	*    ASCII window title, can be a null pointer
	*/
	ApplicationImplSdl2(IApplication& application, const char* windowTitle);

	/**
	*  @brief
	*    Destructor
	*/
	inline virtual ~ApplicationImplSdl2() override
	{
		// Nothing here
		// mNativeWindowHandle is destroyed within onDeinitialization()
	}


//[-------------------------------------------------------]
//[ Public virtual IApplicationImpl methods               ]
//[-------------------------------------------------------]
public:
	virtual void onInitialization() override;
	virtual void onDeinitialization() override;
	[[nodiscard]] virtual bool processMessages() override;
	virtual void getWindowSize(int &width, int &height) const override;
	[[nodiscard]] virtual handle getNativeWindowHandle() const override;
	virtual void redraw() override;
	virtual void showUrgentMessage(const char* message, const char* title = "Urgent Message") const override;


//[-------------------------------------------------------]
//[ Private methods                                       ]
//[-------------------------------------------------------]
private:
	explicit ApplicationImplSdl2(const ApplicationImplSdl2& source) = delete;
	ApplicationImplSdl2& operator =(const ApplicationImplSdl2& source) = delete;
	#ifdef RENDERER_RUNTIME_IMGUI
		void initializeImGuiKeyMap();
		void processImGuiSdl2Event(const SDL_Event& sdlEvent);
		void updateImGuiMousePositionAndButtons();
	#endif


//[-------------------------------------------------------]
//[ Private data                                          ]
//[-------------------------------------------------------]
private:
	IApplication* mApplication;			///< The owner application instance, always valid
	char		  mWindowTitle[64];		///< ASCII window title
	SDL_Window*	  mSdlWindow;			///< SDL2 handle, can be a null handler
	bool		  mFirstUpdate;
	#ifdef RENDERER_RUNTIME_IMGUI
		bool mImGuiMousePressed[3];
	#endif


};

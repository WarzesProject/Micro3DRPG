#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Framework/IApplicationImpl.h"

#include <Engine/Core/Platform/WindowsHeader.h>


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
class IApplication;


//[-------------------------------------------------------]
//[ Classes                                               ]
//[-------------------------------------------------------]
/**
*  @brief
*    Windows application implementation class
*/
class ApplicationImplWindows final : public IApplicationImpl
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
	ApplicationImplWindows(IApplication& application, const char* windowTitle);

	/**
	*  @brief
	*    Destructor
	*/
	inline virtual ~ApplicationImplWindows() override
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

	[[nodiscard]] inline virtual handle getNativeWindowHandle() const override
	{
		return reinterpret_cast<handle>(mNativeWindowHandle);
	}

	virtual void redraw() override;
	virtual void showUrgentMessage(const char* message, const char* title = "Urgent Message") const override;


//[-------------------------------------------------------]
//[ Private static Microsoft Windows callback function    ]
//[-------------------------------------------------------]
private:
	[[nodiscard]] static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


//[-------------------------------------------------------]
//[ Private methods                                       ]
//[-------------------------------------------------------]
private:
	explicit ApplicationImplWindows(const ApplicationImplWindows& source) = delete;
	ApplicationImplWindows& operator =(const ApplicationImplWindows& source) = delete;


//[-------------------------------------------------------]
//[ Private data                                          ]
//[-------------------------------------------------------]
private:
	IApplication* mApplication;			///< The owner application instance, always valid
	char		  mWindowTitle[64];		///< ASCII window title
	HWND		  mNativeWindowHandle;	///< OS window handle, can be a null handler
	bool		  mFirstUpdate;


};

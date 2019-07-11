#pragma once

#include "IApplicationImpl.h"

class IApplication
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
public:
	/**
	*  @brief
	*    Destructor
	*/
	virtual ~IApplication();

	/**
	*  @brief
	*    Run the application
	*
	*  @return
	*    Program return code, 0 to indicate that no error has occurred
	*/
	[[nodiscard]] int run();

	/**
	*  @brief
	*    Ask the application politely to shut down as soon as possible
	*/
	inline void exit()
	{
		mExit = true;
	}

	/**
	*  @brief
	*    Return the window size
	*
	*  @param[out] width
	*    Receives the window width
	*  @param[out] height
	*    Receives the window height
	*/
	inline void getWindowSize(int& width, int& height) const
	{
		return mApplicationImpl->getWindowSize(width, height);
	}

	/**
	*  @brief
	*    Return the OS dependent window handle
	*
	*  @remarks
	*    The OS dependent window handle, can be a null handle
	*/
	[[nodiscard]] inline handle getNativeWindowHandle() const
	{
		return mApplicationImpl->getNativeWindowHandle();
	}

	/**
	*  @brief
	*    Redraw request
	*/
	inline void redraw()
	{
		mApplicationImpl->redraw();
	}

	/**
	*  @brief
	*    Primitive way (e.g. by using a message box) to be able to tell the user that something went terrible wrong
	*
	*  @param[in] message
	*    UTF-8 message to show
	*  @param[in] title
	*    UTF-8 title to show
	*
	*  @remarks
	*    Do not misuse this method in order to communicate with the user on a regular basis. This method does only
	*    exist to be able to tell the user that something went terrible wrong. There are situations were one can't
	*    use a log file, command line or something like this. Even when using e.g. a log file to write out error
	*    information - an application may e.g. just close directly after it's start without any further information
	*    and the user may even think that the application didn't start in the first place for an unknown reason.
	*    In such a situation, it's polite to inform the user that something went terrible wrong and providing
	*    a short hint how the issue may be solved. This method wasn't named "MessageBox()" by intend - because
	*    such a feature may not be available on the used platform or is handled in another way as a normal MS Windows
	*    message box.
	*/
	inline void showUrgentMessage(const char* message, const char* title = "Urgent Message") const
	{
		mApplicationImpl->showUrgentMessage(message, title);
	}

#ifdef LINUX
	/**
	*  @brief
	*    Return the X11 display connection object
	*
	*  @note
	*    - The X11 display connection object, can be a null pointer
	*/
	[[nodiscard]] inline Display* getX11Display() const
	{
		return mApplicationImpl->getX11Display();
	}
#endif


	//[-------------------------------------------------------]
	//[ Public virtual IApplication methods                   ]
	//[-------------------------------------------------------]
public:
	/**
	*  @brief
	*    Called on application initialization
	*
	*  @return
	*    - "true" if all went fine, else "false"
	*
	*  @note
	*    - The base implementation is empty
	*/
	[[nodiscard]] inline virtual bool onInitialization()
	{
		// Done
		return true;
	}

	/**
	*  @brief
	*    Called on application de-initialization
	*
	*  @note
	*    - The base implementation is empty
	*/
	virtual void onDeinitialization();

	/**
	*  @brief
	*    Called in case the window size changed
	*
	*  @note
	*    - The base implementation is empty
	*/
	inline virtual void onResize()
	{
		// Nothing here
	}

	/**
	*  @brief
	*    Called in case the fullscreen state should be toggled
	*
	*  @note
	*    - The base implementation is empty
	*/
	inline virtual void onToggleFullscreenState()
	{
		// Nothing here
	}

	/**
	*  @brief
	*    Update the logic
	*
	*  @note
	*    - The base implementation is empty
	*/
	inline virtual void onUpdate()
	{
		// Nothing here
	}

	/**
	*  @brief
	*    Draw request method
	*
	*  @note
	*    - The base implementation is empty
	*/
	inline virtual void onDrawRequest()
	{
		// Nothing here
	}

	/**
	*  @brief
	*    Draw method
	*
	*  @note
	*    - The base implementation is empty
	*/
	inline virtual void onDraw()
	{
		// Nothing here
	}

	/**
	*  @brief
	*    Called when the escape key has been pressed
	*
	*  @note
	*    - The base implementation is empty
	*/
	virtual void onEscapeKey()
	{
		// Nothing here
	}


	//[-------------------------------------------------------]
	//[ Protected methods                                     ]
	//[-------------------------------------------------------]
protected:
	/**
	*  @brief
	*    Constructor
	*
	*  @param[in] windowTitle
	*    ASCII window title, can be a null pointer
	*/
	explicit IApplication(const char* windowTitle);

	explicit IApplication(const IApplication& source) = delete;
	IApplication& operator =(const IApplication& source) = delete;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
private:
	IApplicationImpl* mApplicationImpl;	///< Application implementation instance, always valid
	bool			  mExit;			///< If "true", the application has been asked politely to shut down as soon as possible, else "false"


};

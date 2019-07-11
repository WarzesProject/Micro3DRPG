#pragma once

class IApplication;

#ifdef LINUX
// Copied from Xlib.h
struct _XDisplay;
typedef struct _XDisplay Display;
#endif


//[-------------------------------------------------------]
//[ Classes                                               ]
//[-------------------------------------------------------]
/**
*  @brief
*    Abstract application implementation interface
*/
class IApplicationImpl
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
public:
	/**
	*  @brief
	*    Destructor
	*/
	inline virtual ~IApplicationImpl()
	{
		// Nothing here
	}

	/**
	*  @brief
	*    Return the owner application
	*
	*  @return
	*    The owner application
	*/
	[[nodiscard]] inline IApplication& getApplication() const
	{
		return mApplication;
	}


	//[-------------------------------------------------------]
	//[ Public virtual IApplicationImpl methods               ]
	//[-------------------------------------------------------]
public:
	/**
	*  @brief
	*    Called on application initialization
	*/
	virtual void onInitialization() = 0;

	/**
	*  @brief
	*    Called on application de-initialization
	*/
	virtual void onDeinitialization() = 0;

	/**
	*  @brief
	*    Process OS messages (non-blocking)
	*
	*  @return
	*    "true" in case the application should shut down, else "false"
	*/
	[[nodiscard]] virtual bool processMessages() = 0;

	/**
	*  @brief
	*    Return the window size
	*
	*  @param[out] width
	*    Receives the window width
	*  @param[out] height
	*    Receives the window height
	*/
	virtual void getWindowSize(int& width, int& height) const = 0;

	/**
	*  @brief
	*    Return the OS dependent window handle
	*
	*  @remarks
	*    The OS dependent window handle, can be a null handle
	*/
	[[nodiscard]] virtual handle getNativeWindowHandle() const = 0;

	/**
	*  @brief
	*    Redraw request
	*/
	virtual void redraw() = 0;

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
	virtual void showUrgentMessage(const char* message, const char* title = "Urgent Message") const = 0;

#ifdef LINUX
	/**
	*  @brief
	*    Return the X11 display connection object
	*
	*  @note
	*    - The X11 display connection object, can be a null pointer
	*/
	[[nodiscard]] virtual Display* getX11Display() const = 0;
#endif


	//[-------------------------------------------------------]
	//[ Protected methods                                     ]
	//[-------------------------------------------------------]
protected:
	/**
	*  @brief
	*    Constructor
	*
	*  @param[in] application
	*    Owner application
	*/
	inline explicit IApplicationImpl(IApplication& application) :
		mApplication(application)
	{
		// Nothing here
	}

	explicit IApplicationImpl(const IApplicationImpl& source) = delete;
	IApplicationImpl& operator =(const IApplicationImpl& source) = delete;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
private:
	IApplication& mApplication;


};

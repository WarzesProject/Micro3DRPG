#pragma once

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
PRAGMA_WARNING_DISABLE_MSVC(4574)	// warning C4574: '_HAS_ITERATOR_DEBUGGING' is defined to be '0': did you mean to use '#if _HAS_ITERATOR_DEBUGGING'?
#include <cassert>
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace Renderer
{
	class ILog;
	class IRenderer;
	class IRenderTarget;
}
namespace RendererRuntime
{
	class IRendererRuntime;
}
namespace RendererToolkit
{
	class IRendererToolkit;
}
class ExampleRunner;
class IApplicationFrontend;


//[-------------------------------------------------------]
//[ Classes                                               ]
//[-------------------------------------------------------]
/**
*  @brief
*    Example base class
*/
class ExampleBase
{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
	friend class ExampleRunner;


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
public:
	/**
	*  @brief
	*    Constructor
	*/
	inline explicit ExampleBase(ExampleRunner& exampleRunner) :
		mExampleRunner(&exampleRunner),
		mCustomLog(nullptr),
		mApplicationFrontend(nullptr)
	{
		// Nothing here
	}

	/**
	*  @brief
	*    Destructor
	*/
	inline virtual ~ExampleBase()
	{
		// Nothing here
	}

	/**
	*  @brief
	*    Return the example runner instance
	*
	*  @return
	*    The example runner instance
	*/
	[[nodiscard]] inline ExampleRunner& getExampleRunner()
	{
		assert((nullptr != mExampleRunner) && "Don't call this method inside constructors");
		return *mExampleRunner;
	}

	/**
	*  @brief
	*    Return the custom log instance
	*
	*  @return
	*    Custom log instance, can be a null pointer, don't destroy the instance
	*/
	[[nodiscard]] inline Renderer::ILog* getCustomLog() const
	{
		return mCustomLog;
	}

	/**
	*  @brief
	*    Let the example draw one frame
	*/
	inline void draw()
	{
		onDraw();
	}

	/**
	*  @brief
	*    Set the application frontend to be used by the example
	*/
	inline void setApplicationFrontend(IApplicationFrontend* applicationFrontend)
	{
		mApplicationFrontend = applicationFrontend;
	}

	/**
	*  @brief
	*    Return the renderer instance
	*
	*  @return
	*    The renderer instance, can be a null pointer, do not release the returned instance unless you added an own reference to it
	*/
	[[nodiscard]] Renderer::IRenderer* getRenderer() const;

	/**
	*  @brief
	*    Return the main renderer target
	*
	*  @return
	*    The main renderer target instance, can be a null pointer, do not release the returned instance unless you added an own reference to it
	*/
	[[nodiscard]] Renderer::IRenderTarget* getMainRenderTarget() const;

	/**
	*  @brief
	*    Return the renderer runtime instance
	*
	*  @return
	*    The renderer runtime instance, can be a null pointer
	*/
	[[nodiscard]] RendererRuntime::IRendererRuntime* getRendererRuntime() const;

	/**
	*  @brief
	*    Return the renderer runtime instance
	*
	*  @return
	*    The renderer runtime instance
	*/
	[[nodiscard]] RendererRuntime::IRendererRuntime& getRendererRuntimeSafe() const;

	/**
	*  @brief
	*    Return the renderer toolkit instance
	*
	*  @return
	*    The renderer toolkit instance, can be a null pointer
	*
	*  @remarks
	*    During runtime, the renderer toolkit can optionally be used to enable asset hot-reloading. Meaning,
	*    as soon as an source asset gets changed, the asset is recompiled in a background thread and the compiled
	*    runtime-ready asset is reloaded. One can see the change in realtime without the need to restart the application.
	*
	*    This feature links during runtime the renderer toolkit as soon as this method is accessed the first time. If
	*    the renderer toolkit shared library is not there, this method will return a null pointer. This is a developer-feature
	*    and as such, it's not available in static builds which are meant for the end-user who e.g. just want to "play the game".
	*/
	[[nodiscard]] RendererToolkit::IRendererToolkit* getRendererToolkit();

	/**
	*  @brief
	*    Ask the application politely to switch to another example as soon as possible
	*
	*  @param[in] exampleName
	*    Example name, must be valid
	*  @param[in] rendererName
	*    Renderer name, if null pointer the default renderer will be used
	*/
	void switchExample(const char* exampleName, const char* rendererName = nullptr);

	/**
	*  @brief
	*    Ask the application politely to shut down as soon as possible
	*/
	void exit();


	//[-------------------------------------------------------]
	//[ Public virtual ExampleBase methods                    ]
	//[-------------------------------------------------------]
public:
	inline virtual void onInitialization()
	{
		// Base does nothing
	}

	inline virtual void onDeinitialization()
	{
		// Base does nothing
	}

	inline virtual void onUpdate()
	{
		// Base does nothing
	}

	inline virtual void onDraw()
	{
		// Base does nothing
	}

	/**
	*  @brief
	*    Return if the examples does the drawing completely on its own; thus no draw handling in frontend (aka draw request handling in IApplicationRenderer)
	*
	*  @return
	*    "true" if the example does it's complete draw handling, otherwise "false"
	*/
	[[nodiscard]] inline virtual bool doesCompleteOwnDrawing() const
	{
		// Default implementation does not complete own drawing
		return false;
	}


	//[-------------------------------------------------------]
	//[ Protected methods                                     ]
	//[-------------------------------------------------------]
protected:
	/**
	*  @brief
	*    Constructor
	*/
	inline ExampleBase() :
		mExampleRunner(nullptr),
		mCustomLog(nullptr),
		mApplicationFrontend(nullptr)
	{
		// Nothing here
	}

	/**
	*  @brief
	*    Set custom log instance
	*
	*  @param[in] customLog
	*    Optional custom log instance, can be a null pointer, the instance must be valid as long as the example base instance exists
	*/
	inline void setCustomLog(Renderer::ILog* customLog = nullptr)
	{
		mCustomLog = customLog;
	}


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
private:
	ExampleRunner*		  mExampleRunner;
	Renderer::ILog*		  mCustomLog;			///< Optional custom log instance, can be a null pointer, don't destroy the instance
	IApplicationFrontend* mApplicationFrontend;	///< Renderer instance, can be a null pointer, do not destroy the instance


};

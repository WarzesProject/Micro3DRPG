#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Framework/PlatformTypes.h"


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace Renderer
{
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


//[-------------------------------------------------------]
//[ Classes                                               ]
//[-------------------------------------------------------]
/**
*  @brief
*    Abstract application frontend interface
*/
class IApplicationFrontend
{


//[-------------------------------------------------------]
//[ Public virtual IApplicationFrontend methods           ]
//[-------------------------------------------------------]
public:
	/**
	*  @brief
	*    Destructor
	*/
	inline virtual ~IApplicationFrontend()
	{
		// Nothing here
	}

	/**
	*  @brief
	*    Ask the application politely to switch to another example as soon as possible
	*
	*  @param[in] exampleName
	*    Example name, must be valid
	*  @param[in] rendererName
	*    Renderer name, if null pointer the default renderer will be used
	*/
	virtual void switchExample(const char* exampleName, const char* rendererName = nullptr) = 0;

	/**
	*  @brief
	*    Ask the application politely to shut down as soon as possible
	*/
	virtual void exit() = 0;

	/**
	*  @brief
	*    Return the renderer instance
	*
	*  @return
	*    The renderer instance, can be a null pointer, do not release the returned instance unless you added an own reference to it
	*/
	[[nodiscard]] virtual Renderer::IRenderer* getRenderer() const = 0;

	/**
	*  @brief
	*    Return the main renderer target
	*
	*  @return
	*    The main renderer target instance, can be a null pointer, do not release the returned instance unless you added an own reference to it
	*/
	[[nodiscard]] virtual Renderer::IRenderTarget* getMainRenderTarget() const = 0;

	/**
	*  @brief
	*    Return the renderer runtime instance
	*
	*  @return
	*    The renderer runtime instance, can be a null pointer
	*/
	[[nodiscard]] inline virtual RendererRuntime::IRendererRuntime* getRendererRuntime() const
	{
		// Base implementation returns always a null pointer
		return nullptr;
	}

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
	[[nodiscard]] inline virtual RendererToolkit::IRendererToolkit* getRendererToolkit()
	{
		// Base implementation returns always a null pointer
		return nullptr;
	}


};

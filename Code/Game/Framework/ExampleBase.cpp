#include "stdafx.h"
#include "Framework/ExampleBase.h"
#include "Framework/IApplicationFrontend.h"


//[-------------------------------------------------------]
//[ Public methods                                        ]
//[-------------------------------------------------------]
Renderer::IRenderer* ExampleBase::getRenderer() const
{
	return (nullptr != mApplicationFrontend) ? mApplicationFrontend->getRenderer() : nullptr;
}

Renderer::IRenderTarget* ExampleBase::getMainRenderTarget() const
{
	return (nullptr != mApplicationFrontend) ? mApplicationFrontend->getMainRenderTarget() : nullptr;
}

RendererRuntime::IRendererRuntime* ExampleBase::getRendererRuntime() const
{
	return (nullptr != mApplicationFrontend) ? mApplicationFrontend->getRendererRuntime() : nullptr;
}

RendererRuntime::IRendererRuntime& ExampleBase::getRendererRuntimeSafe() const
{
	assert(nullptr != mApplicationFrontend);
	RendererRuntime::IRendererRuntime* rendererRuntime = mApplicationFrontend->getRendererRuntime();
	assert(nullptr != rendererRuntime);
	return *rendererRuntime;
}

RendererToolkit::IRendererToolkit* ExampleBase::getRendererToolkit()
{
	return (nullptr != mApplicationFrontend) ? mApplicationFrontend->getRendererToolkit() : nullptr;
}

void ExampleBase::switchExample(const char* exampleName, const char* rendererName)
{
	assert(nullptr != exampleName);
	if (nullptr != mApplicationFrontend)
	{
		mApplicationFrontend->switchExample(exampleName, rendererName);
	}
}

void ExampleBase::exit()
{
	if (nullptr != mApplicationFrontend)
	{
		mApplicationFrontend->exit();
	}
}

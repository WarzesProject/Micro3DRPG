#pragma once

#include "Framework/PlatformTypes.h"

class GameRunner;
class AppRenderer;
namespace RendererRuntime
{
	class IRendererRuntime;
}
namespace RendererToolkit
{
	class IRendererToolkit;
}

class BaseExample
{
	friend class GameRunner;
public:	
	void setApplicationFrontend(AppRenderer* applicationFrontend)
	{
		mApplicationFrontend = applicationFrontend;
	}

	[[nodiscard]] Renderer::IRenderer* getRenderer() const;
	[[nodiscard]] Renderer::IRenderTarget* getMainRenderTarget() const;
	
	void exit();

	void onInitialization();
	void onDeinitialization();
	void onUpdate(){}
	void onDraw();

	[[nodiscard]] inline virtual bool doesCompleteOwnDrawing() const
	{
		return false;
	}


private:
	void fillCommandBuffer();

	Renderer::IBufferManagerPtr			mBufferManager;			///< Buffer manager, can be a null pointer
	Renderer::CommandBuffer				mCommandBuffer;			///< Command buffer
	Renderer::IRootSignaturePtr			mRootSignature;			///< Root signature, can be a null pointer
	Renderer::IGraphicsPipelineStatePtr	mGraphicsPipelineState;	///< Graphics pipeline state object (PSO), can be a null pointer
	Renderer::IVertexArrayPtr			mVertexArray;			///< Vertex array object (VAO), can be a null pointer

	AppRenderer* mApplicationFrontend = nullptr;
};
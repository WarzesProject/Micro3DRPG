#pragma once

#include <Engine/Renderer.h>

class GameApp
{
public:
	bool Init();
	void Close();
	void Update();
	void Draw();
	bool DoesCompleteOwnDrawing();

private:
	void fillCommandBuffer();

	Renderer::IBufferManagerPtr			mBufferManager;			///< Buffer manager, can be a null pointer
	Renderer::CommandBuffer				mCommandBuffer;			///< Command buffer
	Renderer::IRootSignaturePtr			mRootSignature;			///< Root signature, can be a null pointer
	Renderer::IGraphicsPipelineStatePtr	mGraphicsPipelineState;	///< Graphics pipeline state object (PSO), can be a null pointer
	Renderer::IVertexArrayPtr			mVertexArray;			///< Vertex array object (VAO), can be a null pointer
};
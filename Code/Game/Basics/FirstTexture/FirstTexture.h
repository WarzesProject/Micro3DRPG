#pragma once

#include "Framework/ExampleBase.h"

//[-------------------------------------------------------]
//[ Classes                                               ]
//[-------------------------------------------------------]
/**
*  @brief
*    A first example showing how to work with textures
*
*  @remarks
*    Demonstrates:
*    - Vertex buffer object (VBO)
*    - Vertex array object (VAO)
*    - 1D and 2D texture
*    - Sampler state object
*    - Vertex shader (VS) and fragment shader (FS)
*    - Root signature
*    - Graphics pipeline state object (PSO)
*/
class FirstTexture final : public ExampleBase
{


//[-------------------------------------------------------]
//[ Public methods                                        ]
//[-------------------------------------------------------]
public:
	/**
	*  @brief
	*    Constructor
	*/
	inline FirstTexture()
	{
		// Nothing here
	}

	/**
	*  @brief
	*    Destructor
	*/
	inline virtual ~FirstTexture() override
	{
		// The resources are released within "onDeinitialization()"
		// Nothing here
	}


//[-------------------------------------------------------]
//[ Public virtual IApplication methods                   ]
//[-------------------------------------------------------]
public:
	virtual void onInitialization() override;
	virtual void onDeinitialization() override;
	virtual void onDraw() override;


//[-------------------------------------------------------]
//[ Private methods                                       ]
//[-------------------------------------------------------]
private:
	void fillCommandBuffer();


//[-------------------------------------------------------]
//[ Private data                                          ]
//[-------------------------------------------------------]
private:
	Renderer::IBufferManagerPtr			mBufferManager;			///< Buffer manager, can be a null pointer
	Renderer::ITextureManagerPtr		mTextureManager;		///< Texture manager, can be a null pointer
	Renderer::CommandBuffer				mCommandBuffer;			///< Command buffer
	Renderer::IRootSignaturePtr			mRootSignature;			///< Root signature, can be a null pointer
	Renderer::IResourceGroupPtr			mTextureGroup;			///< Texture group, can be a null pointer
	Renderer::IResourceGroupPtr			mSamplerStateGroup;		///< Sampler state resource group, can be a null pointer
	Renderer::IGraphicsPipelineStatePtr	mGraphicsPipelineState;	///< Graphics pipeline state object (PSO), can be a null pointer
	Renderer::IVertexArrayPtr			mVertexArray;			///< Vertex array object (VAO), can be a null pointer


};

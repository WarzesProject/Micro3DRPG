#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Framework/PlatformTypes.h"
#include "Framework/ExampleBase.h"

#include <Core/Time/Stopwatch.h>
#include <Resource/IResourceListener.h>


//[-------------------------------------------------------]
//[ Global definitions                                    ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	typedef uint32_t MeshResourceId;	///< POD mesh resource identifier
	typedef uint32_t TextureResourceId;	///< POD texture resource identifier
}


//[-------------------------------------------------------]
//[ Classes                                               ]
//[-------------------------------------------------------]
/**
*  @brief
*    First mesh example
*
*  @remarks
*    Demonstrates:
*    - Index buffer object (IBO)
*    - Vertex buffer object (VBO)
*    - Vertex array object (VAO)
*    - Texture buffer object (TBO)
*    - Uniform buffer object (UBO)
*    - Sampler state object
*    - Vertex shader (VS) and fragment shader (FS)
*    - Root signature
*    - Graphics pipeline state object (PSO)
*    - Blinn-Phong shading
*    - Albedo, normal, roughness and emissive mapping
*    - Optimization: Cache data to not bother the renderer API to much
*    - Compact vertex format (32 bit texture coordinate, QTangent, 56 bytes vs. 28 bytes per vertex)
*/
class FirstMesh final : public ExampleBase, public RendererRuntime::IResourceListener
{


//[-------------------------------------------------------]
//[ Public methods                                        ]
//[-------------------------------------------------------]
public:
	/**
	*  @brief
	*    Constructor
	*/
	FirstMesh();

	/**
	*  @brief
	*    Destructor
	*/
	inline virtual ~FirstMesh() override
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
	virtual void onUpdate() override;
	virtual void onDraw() override;


//[-------------------------------------------------------]
//[ Protected virtual RendererRuntime::IResourceListener methods ]
//[-------------------------------------------------------]
protected:
	inline virtual void onLoadingStateChange(const RendererRuntime::IResource&) override
	{
		// Forget about the resource group so it's rebuild
		mResourceGroup = nullptr;
	}


//[-------------------------------------------------------]
//[ Protected methods                                     ]
//[-------------------------------------------------------]
protected:
	explicit FirstMesh(const FirstMesh&) = delete;
	FirstMesh& operator=(const FirstMesh&) = delete;


//[-------------------------------------------------------]
//[ Private data                                          ]
//[-------------------------------------------------------]
private:
	Renderer::CommandBuffer				  mCommandBuffer;			///< Command buffer
	Renderer::IRootSignaturePtr			  mRootSignature;			///< Root signature, can be a null pointer
	Renderer::IUniformBufferPtr			  mUniformBuffer;			///< Uniform buffer object (UBO), can be a null pointer
	Renderer::IGraphicsPipelineStatePtr	  mGraphicsPipelineState;	///< Graphics pipeline state object (PSO), can be a null pointer
	Renderer::IGraphicsProgramPtr		  mGraphicsProgram;			///< Graphics program, can be a null pointer
	RendererRuntime::MeshResourceId		  mMeshResourceId;			///< Mesh resource ID, can be set to invalid value
	RendererRuntime::TextureResourceId	  m_argb_nxaTextureResourceId;
	RendererRuntime::TextureResourceId	  m_hr_rg_mb_nyaTextureResourceId;
	RendererRuntime::TextureResourceId	  mEmissiveTextureResourceId;
	Renderer::IResourceGroupPtr			  mResourceGroup;			///< Resource group, can be a null pointer
	Renderer::ISamplerStatePtr			  mSamplerStatePtr;			///< Sampler state, can be a null pointer
	Renderer::IResourceGroupPtr			  mSamplerStateGroup;		///< Sampler state resource group, can be a null pointer
	// Optimization: Cache data to not bother the renderer API to much
	handle	 mObjectSpaceToClipSpaceMatrixUniformHandle;	///< Object space to clip space matrix uniform handle, can be NULL_HANDLE
	handle	 mObjectSpaceToViewSpaceMatrixUniformHandle;	///< Object space to view space matrix uniform handle, can be NULL_HANDLE
	// For timing
	RendererRuntime::Stopwatch mStopwatch;		///< Stopwatch instance
	float					   mGlobalTimer;	///< Global timer


};

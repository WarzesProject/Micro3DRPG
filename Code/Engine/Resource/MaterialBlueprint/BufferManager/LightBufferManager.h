#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Core/Manager.h"
#include "Core/StringId.h"
#include "Core/Platform/PlatformTypes.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4127)	// warning C4127: conditional expression is constant
	PRAGMA_WARNING_DISABLE_MSVC(4201)	// warning C4201: nonstandard extension used: nameless struct/union
	PRAGMA_WARNING_DISABLE_MSVC(4464)	// warning C4464: relative include path contains '..'
	PRAGMA_WARNING_DISABLE_MSVC(4324)	// warning C4324: '<x>': structure was padded due to alignment specifier
	#include <glm/glm.hpp>
PRAGMA_WARNING_POP

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4365)	// warning C4365: 'argument': conversion from 'long' to 'unsigned int', signed/unsigned mismatch
	PRAGMA_WARNING_DISABLE_MSVC(4571)	// warning C4571: Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
	PRAGMA_WARNING_DISABLE_MSVC(4668)	// warning C4668: '_M_HYBRID_X86_ARM64' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
	#include <vector>
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace Renderer
{
	class CommandBuffer;
	class ITextureBuffer;
	class IResourceGroup;
}
namespace RendererRuntime
{
	class SceneResource;
	class IRendererRuntime;
	class MaterialBlueprintResource;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef uint32_t			 TextureResourceId;	///< POD texture resource identifier
	typedef StringId			 AssetId;			///< Asset identifier, internally just a POD "uint32_t", string ID scheme is "<project name>/<asset directory>/<asset name>"
	typedef std::vector<AssetId> AssetIds;


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	/**
	*  @brief
	*    Light buffer manager
	*/
	class LightBufferManager final : private Manager
	{


	//[-------------------------------------------------------]
	//[ Public static methods                                 ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Return the asset IDs of automatically generated dynamic default texture assets
		*
		*  @param[out] assetIds
		*    Receives the asset IDs of automatically generated dynamic default texture assets, the list is not cleared before new entries are added
		*
		*  @remarks
		*    The light buffer manager automatically generates some dynamic default texture assets one can reference e.g. inside material blueprint resources:
		*    - "Unrimp/Texture/DynamicByCode/LightClustersMap3D"
		*/
		static void getDefaultTextureAssetIds(AssetIds& assetIds);


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Constructor
		*
		*  @param[in] rendererRuntime
		*    Renderer runtime instance to use
		*/
		explicit LightBufferManager(IRendererRuntime& rendererRuntime);

		/**
		*  @brief
		*    Destructor
		*/
		~LightBufferManager();

		/**
		*  @brief
		*    Fill the light buffer
		*
		*  @param[in] worldSpaceCameraPosition
		*    64 bit world space position of the camera for camera relative rendering
		*  @param[in] sceneResource
		*    Scene resource to use
		*  @param[out] commandBuffer
		*    Command buffer to fill
		*/
		void fillBuffer(const glm::dvec3& worldSpaceCameraPosition, SceneResource& sceneResource, Renderer::CommandBuffer& commandBuffer);

		/**
		*  @brief
		*    Bind the light buffer manager into the given graphics command buffer
		*
		*  @param[in] materialBlueprintResource
		*    Graphics material blueprint resource
		*  @param[out] commandBuffer
		*    Command buffer to fill
		*/
		void fillGraphicsCommandBuffer(const MaterialBlueprintResource& materialBlueprintResource, Renderer::CommandBuffer& commandBuffer);

		/**
		*  @brief
		*    Bind the light buffer manager into the given compute command buffer
		*
		*  @param[in] materialBlueprintResource
		*    Compute material blueprint resource
		*  @param[out] commandBuffer
		*    Command buffer to fill
		*/
		void fillComputeCommandBuffer(const MaterialBlueprintResource& materialBlueprintResource, Renderer::CommandBuffer& commandBuffer);

		/**
		*  @brief
		*    Get light clusters scale
		*
		*  @return
		*    Light clusters scale
		*/
		[[nodiscard]] glm::vec3 getLightClustersScale() const;

		/**
		*  @brief
		*    Get light clusters bias
		*
		*  @return
		*    Light clusters bias
		*/
		[[nodiscard]] glm::vec3 getLightClustersBias() const;


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		explicit LightBufferManager(const LightBufferManager&) = delete;
		LightBufferManager& operator=(const LightBufferManager&) = delete;
		void fillTextureBuffer(const glm::dvec3& worldSpaceCameraPosition, SceneResource& sceneResource);	// 64 bit world space position of the camera
		void fillClusters3DTexture(SceneResource& sceneResource, Renderer::CommandBuffer& commandBuffer);


	//[-------------------------------------------------------]
	//[ Private definitions                                   ]
	//[-------------------------------------------------------]
	private:
		typedef std::vector<uint8_t> ScratchBuffer;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		IRendererRuntime&		  mRendererRuntime;	///< Renderer runtime instance to use
		Renderer::ITextureBuffer* mTextureBuffer;	///< Texture buffer instance, always valid
		ScratchBuffer			  mTextureScratchBuffer;
		TextureResourceId		  mClusters3DTextureResourceId;
		glm::vec3				  mLightClustersAabbMinimum;
		glm::vec3				  mLightClustersAabbMaximum;
		Renderer::IResourceGroup* mResourceGroup;	///< Resource group instance, always valid


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

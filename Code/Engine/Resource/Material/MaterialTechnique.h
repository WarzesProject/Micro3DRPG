#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Resource/IResourceListener.h"
#include "Resource/Material/MaterialProperty.h"
#include "Resource/MaterialBlueprint/BufferManager/MaterialBufferSlot.h"
#include "Renderer.h"

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
namespace RendererRuntime
{
	class IRendererRuntime;
	class MaterialBufferManager;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef uint32_t TextureResourceId;				///< POD texture resource identifier
	typedef uint32_t MaterialTechniqueId;			///< Material technique identifier, result of hashing the material technique name via "RendererRuntime::StringId"
	typedef uint32_t MaterialBlueprintResourceId;	///< POD material blueprint resource identifier


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	/**
	*  @brief
	*    Material technique
	*/
	class MaterialTechnique final : public MaterialBufferSlot, public IResourceListener
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class MaterialResource;					// Material technique owner
		friend class MaterialBlueprintResourceManager;	// Needs to be able to call "RendererRuntime::MaterialTechnique::makeResourceGroupDirty()"


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Constructor
		*
		*  @param[in] materialTechniqueId
		*    Material technique ID
		*  @param[in] materialResource
		*    Owner material resource, only material resource manager and material resource ID will internally be stored
		*  @param[in] materialBlueprintResourceId
		*    Material blueprint resource ID
		*/
		MaterialTechnique(MaterialTechniqueId materialTechniqueId, MaterialResource& materialResource, MaterialBlueprintResourceId materialBlueprintResourceId);

		/**
		*  @brief
		*    Destructor
		*/
		virtual ~MaterialTechnique() override;

		/**
		*  @brief
		*    Return the material technique ID
		*
		*  @return
		*    The material technique ID
		*/
		[[nodiscard]] inline MaterialTechniqueId getMaterialTechniqueId() const
		{
			return mMaterialTechniqueId;
		}

		/**
		*  @brief
		*    Return the ID of the used material blueprint resource
		*
		*  @return
		*    The ID of the used material blueprint resource, can be invalid
		*/
		[[nodiscard]] inline MaterialBlueprintResourceId getMaterialBlueprintResourceId() const
		{
			return mMaterialBlueprintResourceId;
		}

		/**
		*  @brief
		*    Set structured buffer pointer
		*
		*  @param[in] structuredBufferPtr
		*    Structured buffer pointer
		*/
		inline void setStructuredBufferPtr(Renderer::IStructuredBufferPtr& structuredBufferPtr)
		{
			mStructuredBufferPtr = structuredBufferPtr;
		}

		/**
		*  @brief
		*    Return the FNV1a hash of "Renderer::SerializedGraphicsPipelineState"
		*
		*  @return
		*    The FNV1a hash of "Renderer::SerializedGraphicsPipelineState"
		*/
		[[nodiscard]] inline uint32_t getSerializedGraphicsPipelineStateHash() const
		{
			return mSerializedGraphicsPipelineStateHash;
		}

		/**
		*  @brief
		*    Bind the graphics material technique into the given command buffer
		*
		*  @param[in] rendererRuntime
		*    Renderer runtime to use
		*  @param[out] commandBuffer
		*    Command buffer to fill
		*  @param[out] resourceGroupRootParameterIndex
		*    Root parameter index to bind the resource group to, can be "RendererRuntime::getInvalid<uint32_t>()"
		*  @param[out] resourceGroup
		*    Renderer resource group to set
		*/
		void fillGraphicsCommandBuffer(const IRendererRuntime& rendererRuntime, Renderer::CommandBuffer& commandBuffer, uint32_t& resourceGroupRootParameterIndex, Renderer::IResourceGroup** resourceGroup);

		/**
		*  @brief
		*    Bind the compute material technique into the given command buffer
		*
		*  @param[in] rendererRuntime
		*    Renderer runtime to use
		*  @param[out] commandBuffer
		*    Command buffer to fill
		*  @param[out] resourceGroupRootParameterIndex
		*    Root parameter index to bind the resource group to, can be "RendererRuntime::getInvalid<uint32_t>()"
		*  @param[out] resourceGroup
		*    Renderer resource group to set
		*/
		void fillComputeCommandBuffer(const IRendererRuntime& rendererRuntime, Renderer::CommandBuffer& commandBuffer, uint32_t& resourceGroupRootParameterIndex, Renderer::IResourceGroup** resourceGroup);


	//[-------------------------------------------------------]
	//[ Protected virtual RendererRuntime::IResourceListener methods ]
	//[-------------------------------------------------------]
	protected:
		virtual void onLoadingStateChange(const RendererRuntime::IResource& resource) override;


	//[-------------------------------------------------------]
	//[ Private definitions                                   ]
	//[-------------------------------------------------------]
	private:
		struct Texture final
		{
			uint32_t		  rootParameterIndex;
			MaterialProperty  materialProperty;
			TextureResourceId textureResourceId;
		};
		typedef std::vector<Texture> Textures;


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		explicit MaterialTechnique(const MaterialTechnique&) = delete;
		MaterialTechnique& operator=(const MaterialTechnique&) = delete;
		MaterialBufferManager* getMaterialBufferManager() const;

		inline void clearTextures()
		{
			mTextures.clear();
			makeResourceGroupDirty();
		}

		/**
		*  @brief
		*    Return the textures
		*
		*  @param[in] rendererRuntime
		*    Renderer runtime to use
		*
		*  @return
		*    The textures
		*/
		[[nodiscard]] const Textures& getTextures(const IRendererRuntime& rendererRuntime);

		inline void makeResourceGroupDirty()
		{
			// Forget about the resource group so it's rebuild
			// TODO(co) Optimization possibility: Allow it to update resource groups instead of always having to destroy and recreate them?
			mResourceGroup = nullptr;
		}

		/**
		*  @brief
		*    Calculate FNV1a hash of "Renderer::SerializedGraphicsPipelineState"
		*/
		void calculateSerializedGraphicsPipelineStateHash();

		/**
		*  @brief
		*    Schedule the material slot for shader uniform update
		*/
		void scheduleForShaderUniformUpdate();

		/**
		*  @brief
		*    Bind the material technique into the given command buffer
		*
		*  @param[in] rendererRuntime
		*    Renderer runtime to use
		*  @param[out] resourceGroupRootParameterIndex
		*    Root parameter index to bind the resource group to, can be "RendererRuntime::getInvalid<uint32_t>()"
		*  @param[out] resourceGroup
		*    Renderer resource group to set
		*/
		void fillCommandBuffer(const IRendererRuntime& rendererRuntime, uint32_t& resourceGroupRootParameterIndex, Renderer::IResourceGroup** resourceGroup);


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		MaterialTechniqueId			   mMaterialTechniqueId;					///< Material technique ID
		MaterialBlueprintResourceId	   mMaterialBlueprintResourceId;			///< Material blueprint resource ID, can be set to invalid value
		Renderer::IStructuredBufferPtr mStructuredBufferPtr;
		Textures					   mTextures;
		uint32_t					   mSerializedGraphicsPipelineStateHash;	///< FNV1a hash of "Renderer::SerializedGraphicsPipelineState"
		Renderer::IResourceGroupPtr	   mResourceGroup;							///< Resource group, can be a null pointer


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

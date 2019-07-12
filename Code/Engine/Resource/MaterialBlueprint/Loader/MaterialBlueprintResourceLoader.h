#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Resource/ShaderBlueprint/GraphicsShaderType.h"
#include "Resource/IResourceLoader.h"
#include "Core/GetInvalid.h"
#include "Core/File/MemoryFile.h"

#include "Renderer.h"


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class IRendererRuntime;
	class MaterialBlueprintResource;
	template <class TYPE, class LOADER_TYPE, typename ID_TYPE, uint32_t MAXIMUM_NUMBER_OF_ELEMENTS> class ResourceManagerTemplate;
	namespace v1MaterialBlueprint
	{
		struct Texture;
		struct SamplerState;
	}
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef uint32_t MaterialBlueprintResourceId;	///< POD material blueprint resource identifier


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	class MaterialBlueprintResourceLoader final : public IResourceLoader
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend ResourceManagerTemplate<MaterialBlueprintResource, MaterialBlueprintResourceLoader, MaterialBlueprintResourceId, 64>;	// Type definition of template class


	//[-------------------------------------------------------]
	//[ Public definitions                                    ]
	//[-------------------------------------------------------]
	public:
		static constexpr uint32_t TYPE_ID = STRING_ID("material_blueprint");


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::IResourceLoader methods ]
	//[-------------------------------------------------------]
	public:
		[[nodiscard]] inline virtual ResourceLoaderTypeId getResourceLoaderTypeId() const override
		{
			return TYPE_ID;
		}

		virtual void initialize(const Asset& asset, bool reload, IResource& resource) override;

		[[nodiscard]] inline virtual bool hasDeserialization() const override
		{
			return true;
		}

		[[nodiscard]] virtual bool onDeserialization(IFile& file) override;

		[[nodiscard]] inline virtual bool hasProcessing() const override
		{
			return true;
		}

		virtual void onProcessing() override;
		[[nodiscard]] virtual bool onDispatch() override;
		[[nodiscard]] virtual bool isFullyLoaded() override;


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		inline MaterialBlueprintResourceLoader(IResourceManager& resourceManager, IRendererRuntime& rendererRuntime) :
			IResourceLoader(resourceManager),
			mRendererRuntime(rendererRuntime),
			mMaterialBlueprintResource(nullptr),
			mMaximumNumberOfRootParameters(0),
			mMaximumNumberOfDescriptorRanges(0),
			mVertexAttributesAssetId(getInvalid<AssetId>()),
			mGraphicsShaderBlueprintAssetId{getInvalid<AssetId>(), getInvalid<AssetId>(), getInvalid<AssetId>(), getInvalid<AssetId>(), getInvalid<AssetId>()},
			mComputeShaderBlueprintAssetId(getInvalid<AssetId>()),
			mMaximumNumberOfMaterialBlueprintSamplerStates(0),
			mMaterialBlueprintSamplerStates(nullptr),
			mMaximumNumberOfMaterialBlueprintTextures(0),
			mMaterialBlueprintTextures(nullptr)
		{
			// Nothing here
		}

		virtual ~MaterialBlueprintResourceLoader() override;
		explicit MaterialBlueprintResourceLoader(const MaterialBlueprintResourceLoader&) = delete;
		MaterialBlueprintResourceLoader& operator=(const MaterialBlueprintResourceLoader&) = delete;
		void createRendererResources();


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		IRendererRuntime&		   mRendererRuntime;			///< Renderer runtime instance, do not destroy the instance
		MaterialBlueprintResource* mMaterialBlueprintResource;	///< Destination resource

		// Temporary data
		MemoryFile mMemoryFile;

		// Temporary data: Root signature
		uint32_t							   mMaximumNumberOfRootParameters;
		std::vector<Renderer::RootParameter>   mRootParameters;
		uint32_t							   mMaximumNumberOfDescriptorRanges;
		std::vector<Renderer::DescriptorRange> mDescriptorRanges;
		Renderer::RootSignature				   mRootSignature;

		// Temporary data for graphics pipeline state: Vertex attributes and graphics shader blueprints
		AssetId mVertexAttributesAssetId;
		AssetId mGraphicsShaderBlueprintAssetId[NUMBER_OF_GRAPHICS_SHADER_TYPES];

		// Temporary data for compute pipeline state: Compute shader blueprint
		AssetId mComputeShaderBlueprintAssetId;

		// Temporary data: Sampler states
		uint32_t						   mMaximumNumberOfMaterialBlueprintSamplerStates;
		v1MaterialBlueprint::SamplerState* mMaterialBlueprintSamplerStates;

		// Temporary data: Textures
		uint32_t					  mMaximumNumberOfMaterialBlueprintTextures;
		v1MaterialBlueprint::Texture* mMaterialBlueprintTextures;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

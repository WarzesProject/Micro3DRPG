#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Resource/ResourceManager.h"
#include "Resource/Material/MaterialProperties.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4365)	// warning C4365: 'argument': conversion from 'std::_Atomic_integral_t' to 'long', signed/unsigned mismatch
	PRAGMA_WARNING_DISABLE_MSVC(4623)	// warning C4623: 'std::_UInt_is_zero': default constructor was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(4625)	// warning C4625: 'std::_Generic_error_category': copy constructor was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(4626)	// warning C4626: 'std::_Generic_error_category': assignment operator was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(5026)	// warning C5026: 'std::_Generic_error_category': move constructor was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(5027)	// warning C5027: 'std::_Generic_error_category': move assignment operator was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(5039)	// warning C5039: '_Thrd_start': pointer or reference to potentially throwing function passed to extern C function under -EHc. Undefined behavior may occur if this function throws an exception.
	#include <mutex>
	#include <unordered_map>
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class IFile;
	class MemoryFile;
	class IRendererRuntime;
	class LightBufferManager;
	class IndirectBufferManager;
	class MaterialBlueprintResource;
	class UniformInstanceBufferManager;
	class TextureInstanceBufferManager;
	class MaterialBlueprintResourceLoader;
	class IMaterialBlueprintResourceListener;
	template <class TYPE, class LOADER_TYPE, typename ID_TYPE, uint32_t MAXIMUM_NUMBER_OF_ELEMENTS> class ResourceManagerTemplate;
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
	class MaterialBlueprintResourceManager final : public ResourceManager<MaterialBlueprintResource>
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class RendererRuntimeImpl;
		friend class IResource;						// Needed so that inside this classes an static_cast<CompositorNodeResourceManager*>(IResourceManager*) works
		friend class MaterialTechnique;				// Needs to be able to call "RendererRuntime::MaterialBlueprintResourceManager::addSerializedGraphicsPipelineState()"
		friend class GraphicsPipelineStateCompiler;	// Needs to be able to call "RendererRuntime::MaterialBlueprintResourceManager::applySerializedGraphicsPipelineState()"


	//[-------------------------------------------------------]
	//[ Public definitions                                    ]
	//[-------------------------------------------------------]
	public:
		typedef std::unordered_map<uint32_t, Renderer::SerializedGraphicsPipelineState> SerializedGraphicsPipelineStates;	///< Key = FNV1a hash of "Renderer::SerializedGraphicsPipelineState"


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		[[nodiscard]] inline IRendererRuntime& getRendererRuntime() const
		{
			return mRendererRuntime;
		}

		[[nodiscard]] inline bool getCreateInitialPipelineStateCaches() const
		{
			return mCreateInitialPipelineStateCaches;
		}

		inline void setCreateInitialPipelineStateCaches(bool createInitialPipelineStateCaches)
		{
			mCreateInitialPipelineStateCaches = createInitialPipelineStateCaches;
		}

		RENDERERRUNTIME_API_EXPORT void loadMaterialBlueprintResourceByAssetId(AssetId assetId, MaterialBlueprintResourceId& materialBlueprintResourceId, IResourceListener* resourceListener = nullptr, bool reload = false, ResourceLoaderTypeId resourceLoaderTypeId = getInvalid<ResourceLoaderTypeId>(), bool createInitialPipelineStateCaches = true);	// Asynchronous

		[[nodiscard]] inline IMaterialBlueprintResourceListener& getMaterialBlueprintResourceListener() const
		{
			// We know this pointer must always be valid
			RENDERER_ASSERT(mRendererRuntime.getContext(), nullptr != mMaterialBlueprintResourceListener, "Invalid material blueprint resource listener")
			return *mMaterialBlueprintResourceListener;
		}

		RENDERERRUNTIME_API_EXPORT void setMaterialBlueprintResourceListener(IMaterialBlueprintResourceListener* materialBlueprintResourceListener);	// Does not take over the control of the memory

		/**
		*  @brief
		*    Return the global material properties
		*
		*  @return
		*    The global material properties
		*
		*  @remarks
		*    The material blueprint resource manager itself is setting the following global material properties:
		*    - Floating point property "GlobalPastSecondsSinceLastFrame"
		*    - Floating point property "GlobalTimeInSeconds"
		*    - Floating point property "PreviousGlobalTimeInSeconds"
		*    - Integer property "GlobalNumberOfMultisamples" (see "RendererRuntime::CompositorWorkspaceInstance::setNumberOfMultisamples()")
		*/
		[[nodiscard]] inline MaterialProperties& getGlobalMaterialProperties()
		{
			return mGlobalMaterialProperties;
		}

		[[nodiscard]] inline const MaterialProperties& getGlobalMaterialProperties() const
		{
			return mGlobalMaterialProperties;
		}

		/**
		*  @brief
		*    Called pre command buffer execution
		*/
		void onPreCommandBufferExecution();

		//[-------------------------------------------------------]
		//[ Default texture filtering                             ]
		//[-------------------------------------------------------]
		[[nodiscard]] inline Renderer::FilterMode getDefaultTextureFilterMode() const
		{
			return mDefaultTextureFilterMode;
		}

		[[nodiscard]] inline uint8_t getDefaultMaximumTextureAnisotropy() const
		{
			return mDefaultMaximumTextureAnisotropy;
		}

		RENDERERRUNTIME_API_EXPORT void setDefaultTextureFiltering(Renderer::FilterMode filterMode, uint8_t maximumAnisotropy);

		//[-------------------------------------------------------]
		//[ Manager                                               ]
		//[-------------------------------------------------------]
		[[nodiscard]] inline UniformInstanceBufferManager& getUniformInstanceBufferManager() const
		{
			// We know this pointer must always be valid
			RENDERER_ASSERT(mRendererRuntime.getContext(), nullptr != mUniformInstanceBufferManager, "Invalid uniform instance buffer manager")
			return *mUniformInstanceBufferManager;
		}

		[[nodiscard]] inline TextureInstanceBufferManager& getTextureInstanceBufferManager() const
		{
			// We know this pointer must always be valid
			RENDERER_ASSERT(mRendererRuntime.getContext(), nullptr != mTextureInstanceBufferManager, "Invalid texture instance buffer manager")
			return *mTextureInstanceBufferManager;
		}

		[[nodiscard]] inline IndirectBufferManager& getIndirectBufferManager() const
		{
			// We know this pointer must always be valid
			RENDERER_ASSERT(mRendererRuntime.getContext(), nullptr != mIndirectBufferManager, "Invalid indirect buffer manager")
			return *mIndirectBufferManager;
		}

		[[nodiscard]] inline LightBufferManager& getLightBufferManager() const
		{
			// We know this pointer must always be valid
			RENDERER_ASSERT(mRendererRuntime.getContext(), nullptr != mLightBufferManager, "Invalid light buffer manager")
			return *mLightBufferManager;
		}


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::IResourceManager methods ]
	//[-------------------------------------------------------]
	public:
		[[nodiscard]] virtual uint32_t getNumberOfResources() const override;
		[[nodiscard]] virtual IResource& getResourceByIndex(uint32_t index) const override;
		[[nodiscard]] virtual IResource& getResourceByResourceId(ResourceId resourceId) const override;
		[[nodiscard]] virtual IResource* tryGetResourceByResourceId(ResourceId resourceId) const override;
		virtual void reloadResourceByAssetId(AssetId assetId) override;
		virtual void update() override;


	//[-------------------------------------------------------]
	//[ Private virtual RendererRuntime::IResourceManager methods ]
	//[-------------------------------------------------------]
	private:
		[[nodiscard]] virtual IResourceLoader* createResourceLoaderInstance(ResourceLoaderTypeId resourceLoaderTypeId) override;


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		explicit MaterialBlueprintResourceManager(IRendererRuntime& rendererRuntime);
		virtual ~MaterialBlueprintResourceManager() override;
		explicit MaterialBlueprintResourceManager(const MaterialBlueprintResourceManager&) = delete;
		MaterialBlueprintResourceManager& operator=(const MaterialBlueprintResourceManager&) = delete;

		//[-------------------------------------------------------]
		//[ Pipeline state object cache                           ]
		//[-------------------------------------------------------]
		void addSerializedGraphicsPipelineState(uint32_t serializedGraphicsPipelineStateHash, const Renderer::SerializedGraphicsPipelineState& serializedGraphicsPipelineState);
		void applySerializedGraphicsPipelineState(uint32_t serializedGraphicsPipelineStateHash, Renderer::GraphicsPipelineState& graphicsPipelineState);
		void clearPipelineStateObjectCache();
		void loadPipelineStateObjectCache(IFile& file);
		[[nodiscard]] bool doesPipelineStateObjectCacheNeedSaving() const;
		void savePipelineStateObjectCache(MemoryFile& memoryFile);


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		IRendererRuntime&					mRendererRuntime;							///< Renderer runtime instance, do not destroy the instance
		bool								mCreateInitialPipelineStateCaches;			///< Create initial graphics and compute pipeline state caches after a material blueprint has been loaded?
		IMaterialBlueprintResourceListener*	mMaterialBlueprintResourceListener;			///< Material blueprint resource listener, always valid, do not destroy the instance
		MaterialProperties					mGlobalMaterialProperties;					///< Global material properties
		Renderer::FilterMode				mDefaultTextureFilterMode;					///< Default texture filter mode
		uint8_t								mDefaultMaximumTextureAnisotropy;			///< Default maximum texture anisotropy
		std::mutex							mSerializedGraphicsPipelineStatesMutex;		///< "RendererRuntime::GraphicsPipelineStateCompiler" is running asynchronous, hence we need to synchronize the serialized graphics pipeline states access
		SerializedGraphicsPipelineStates	mSerializedGraphicsPipelineStates;			///< Serialized pipeline states
		UniformInstanceBufferManager*		mUniformInstanceBufferManager;				///< Uniform instance buffer manager, always valid in a sane none-legacy environment
		TextureInstanceBufferManager*		mTextureInstanceBufferManager;				///< Texture instance buffer manager, always valid in a sane none-legacy environment
		IndirectBufferManager*				mIndirectBufferManager;						///< Indirect buffer manager, always valid in a sane none-legacy environment
		LightBufferManager*					mLightBufferManager;						///< Light buffer manager, always valid in a sane none-legacy environment

		// Internal resource manager implementation
		ResourceManagerTemplate<MaterialBlueprintResource, MaterialBlueprintResourceLoader, MaterialBlueprintResourceId, 64>* mInternalResourceManager;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

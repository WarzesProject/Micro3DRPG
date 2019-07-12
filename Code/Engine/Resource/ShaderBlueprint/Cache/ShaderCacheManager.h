#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Core/Manager.h"
#include "Resource/ShaderBlueprint/GraphicsShaderType.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4548)	// warning C4548: expression before comma has no effect; expected expression with side-effect
	PRAGMA_WARNING_DISABLE_MSVC(4623)	// warning C4623: 'std::_UInt_is_zero': default constructor was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(4626)	// warning C4626: 'std::_UInt_is_zero': assignment operator was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(5027)	// warning C5027: 'std::_UInt_is_zero': move assignment operator was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(5039)	// warning C5039: '_Thrd_start': pointer or reference to potentially throwing function passed to extern C function under -EHc. Undefined behavior may occur if this function throws an exception.
	#include <mutex>
	#include <unordered_map>
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace Renderer
{
	class IShaderLanguage;
}
namespace RendererRuntime
{
	class IFile;
	class ShaderCache;
	class MaterialBlueprintResource;
	class ComputePipelineStateSignature;
	class GraphicsPipelineStateSignature;
	class ShaderBlueprintResourceManager;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef uint32_t ShaderCacheId;			///< Shader cache identifier, often but not always identical to the shader combination ID
	typedef uint32_t ShaderSourceCodeId;	///< Shader source code identifier, result of hashing the build shader source code


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	/*
	*  @brief
	*    Shader cache manager
	*
	*  @see
	*    - See "RendererRuntime::GraphicsPipelineStateCacheManager" and "RendererRuntime::ComputePipelineStateCacheManager" for additional information
	*/
	class ShaderCacheManager final : private Manager
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class ShaderBlueprintResourceManager;	// Is creating and using a shader cache manager instance
		friend class GraphicsPipelineStateCompiler;		// Is tightly interacting with the shader cache manager
		friend class ComputePipelineStateCompiler;		// Is tightly interacting with the shader cache manager


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Return the owner shader blueprint resource manager
		*
		*  @return
		*    The owner shader blueprint resource manager
		*/
		[[nodiscard]] inline ShaderBlueprintResourceManager& getShaderBlueprintResourceManager() const
		{
			return mShaderBlueprintResourceManager;
		}

		/**
		*  @brief
		*    Get shader cache by graphics pipeline state signature and shader type; synchronous processing
		*
		*  @param[in] graphicsPipelineStateSignature
		*    Graphics pipeline state signature to use
		*  @param[in] materialBlueprintResource
		*    Material blueprint resource
		*  @param[in] shaderLanguage
		*    Shader language
		*  @param[in] graphicsShaderType
		*    Graphics shader type
		*
		*  @return
		*    The shader cache, null pointer on error
		*/
		[[nodiscard]] ShaderCache* getGraphicsShaderCache(const GraphicsPipelineStateSignature& graphicsPipelineStateSignature, const MaterialBlueprintResource& materialBlueprintResource, Renderer::IShaderLanguage& shaderLanguage, GraphicsShaderType graphicsShaderType);

		/**
		*  @brief
		*    Get shader cache by compute pipeline state signature and shader type; synchronous processing
		*
		*  @param[in] computePipelineStateSignature
		*    Compute pipeline state signature to use
		*  @param[in] materialBlueprintResource
		*    Material blueprint resource
		*  @param[in] shaderLanguage
		*    Shader language
		*
		*  @return
		*    The shader cache, null pointer on error
		*/
		[[nodiscard]] ShaderCache* getComputeShaderCache(const ComputePipelineStateSignature& computePipelineStateSignature, const MaterialBlueprintResource& materialBlueprintResource, Renderer::IShaderLanguage& shaderLanguage);


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		inline explicit ShaderCacheManager(ShaderBlueprintResourceManager& shaderBlueprintResourceManager) :
			mShaderBlueprintResourceManager(shaderBlueprintResourceManager),
			mCacheNeedsSaving(false)
		{
			// Nothing here
		}

		inline ~ShaderCacheManager()
		{
			clearCache();
		}

		explicit ShaderCacheManager(const ShaderCacheManager&) = delete;
		ShaderCacheManager& operator=(const ShaderCacheManager&) = delete;
		// TODO(co) Make this private. See "TODO(co) Do only clear the influenced shader cache entries" for context.
	public:
		void clearCache();
	private:
		void loadCache(IFile& file);

		[[nodiscard]] inline bool doesCacheNeedSaving() const
		{
			return mCacheNeedsSaving;
		}

		void saveCache(IFile& file);


	//[-------------------------------------------------------]
	//[ Private definitions                                   ]
	//[-------------------------------------------------------]
	private:
		typedef std::unordered_map<ShaderCacheId, ShaderCache*>		  ShaderCacheByShaderCacheId;
		typedef std::unordered_map<ShaderSourceCodeId, ShaderCacheId> ShaderCacheByShaderSourceCodeId;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		ShaderBlueprintResourceManager& mShaderBlueprintResourceManager;	///< Owner shader blueprint resource manager
		ShaderCacheByShaderCacheId		mShaderCacheByShaderCacheId;		///< Manages the shader cache instances
		ShaderCacheByShaderSourceCodeId	mShaderCacheByShaderSourceCodeId;	///< Shader source code ID to shader cache ID mapping
		bool							mCacheNeedsSaving;					///< "true" if a cache needs saving due to changes during runtime, else "false"
		std::mutex						mMutex;								///< Mutex due to "RendererRuntime::GraphicsPipelineStateCompiler" and "RendererRuntime::ComputePipelineStateCompiler" interaction, no too fine granular lock/unlock required because usually it's only asynchronous or synchronous processing, not both at one and the same time


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

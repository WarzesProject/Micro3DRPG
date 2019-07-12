#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Resource/MaterialBlueprint/Cache/GraphicsPipelineStateSignature.h"
#include "Resource/MaterialBlueprint/Cache/GraphicsProgramCacheManager.h"
#include "Core/File/IFile.h"
#include "Renderer.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4365)	// warning C4365: 'argument': conversion from 'long' to 'unsigned int', signed/unsigned mismatch
	PRAGMA_WARNING_DISABLE_MSVC(4571)	// warning C4571: Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
	PRAGMA_WARNING_DISABLE_MSVC(4625)	// warning C4625: 'std::codecvt_base': copy constructor was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(4626)	// warning C4626: 'std::codecvt<char16_t,char,_Mbstatet>': assignment operator was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(4668)	// warning C4668: '_M_HYBRID_X86_ARM64' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
	PRAGMA_WARNING_DISABLE_MSVC(4774)	// warning C4774: 'sprintf_s' : format string expected in argument 3 is not a string literal
	PRAGMA_WARNING_DISABLE_MSVC(5026)	// warning C5026: 'std::_Generic_error_category': move constructor was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(5027)	// warning C5027: 'std::_Generic_error_category': move assignment operator was implicitly defined as deleted
	#include <string>
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class GraphicsPipelineStateCache;
}


// Disable warnings
// TODO(co) See "RendererRuntime::GraphicsPipelineStateCacheManager::GraphicsPipelineStateCacheManager()": How the heck should we avoid such a situation without using complicated solutions like a pointer to an instance? (= more individual allocations/deallocations)
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4355)	// warning C4355: 'this': used in base member initializer list


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef uint32_t GraphicsPipelineStateSignatureId;	///< Graphics pipeline state signature identifier, result of hashing the referenced shaders as well as other pipeline state properties


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	/**
	*  @brief
	*    Graphics pipeline state cache manager
	*
	*  @remarks
	*    The graphics pipeline state cache is the top of the shader related cache hierarchy and maps to Vulkan, Direct3D 12,
	*    Apple Metal and other rendering APIs using pipeline state objects (PSO). The next cache hierarchy
	*    level is the graphics program cache which maps to linked monolithic OpenGL programs and is also nice as a collection
	*    of shader compiler results which are fed into pipeline states. The next lowest cache hierarchy level is the
	*    shader cache (vertex shader, fragment shader etc.) which handles the binary results of the shader compiler.
	*    As of January 2016, although claimed to fulfill the OpenGL 4.1 specification, Apples OpenGL implementation used
	*    on Mac OS X lacks the feature of receiving the program binary in order to reuse it for the next time instead of
	*    fully compiling a program. Hence, at the lowest cache hierarchy, there's a shader source code cache for the build
	*    shader source codes so at least this doesn't need to be performed during each program execution.
	*
	*    Sum up of the cache hierarchy:
	*    - 0: "RendererRuntime::GraphicsPipelineStateCacheManager": Maps to Vulkan, Direct3D 12, Apple Metal etc.; managed by material blueprint
	*    - 1: "RendererRuntime::GraphicsProgramCacheManager": Maps to linked monolithic OpenGL programs; managed by shader blueprint manager
	*    - 2: "RendererRuntime::ShaderCacheManager": Maps to Direct3D 9 - 11, separate OpenGL shader objects and is still required for Direct3D 12
	*      and other similar designed APIs because the binary shaders are required when creating pipeline state objects;
	*      managed by shader blueprint manager
	*    - 3: "RendererRuntime::ShaderSourceCodeCacheManager": Shader source code cache for the build shader source codes, used for e.g. Apples
	*      OpenGL implementation lacking of binary program support; managed by shader blueprint manager   TODO(co) "RendererRuntime::ShaderSourceCodeCacheManager" doesn't exist, yet
	*
	*    The graphics pipeline state cache has two types of IDs:
	*    - "RendererRuntime::GraphicsPipelineStateSignatureId" -> Result of hashing the material blueprint ID and the shader combination generating shader properties and dynamic shader pieces
	*    - "RendererRuntime::GraphicsPipelineStateCacheId" -> Includes the hashing the build shader source code
	*    Those two types of IDs are required because it's possible that different "RendererRuntime::GraphicsPipelineStateSignatureId" result in one and the
	*    same build shader source code of references shaders.
	*
	*  @note
	*    - One pipeline state cache manager per material blueprint instance
	*
	*  @todo
	*    - TODO(co) For Vulkan, DirectX 12 and Apple Metal the pipeline state object instance will be managed in here
	*    - TODO(co) Direct3D 12: Pipeline state object: Add support for "GetCachedBlob" (super efficient material cache), see https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/Samples/D3D12PipelineStateCache/src/PSOLibrary.cpp
	*/
	class GraphicsPipelineStateCacheManager final : private Manager
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class MaterialBlueprintResource;	// Is creating and using a graphics program cache manager instance


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Return the owner material blueprint resource
		*
		*  @return
		*    The owner material blueprint resource
		*/
		[[nodiscard]] inline MaterialBlueprintResource& getMaterialBlueprintResource() const
		{
			return mMaterialBlueprintResource;
		}

		/**
		*  @brief
		*    Return the graphics program cache manager
		*
		*  @return
		*    The graphics program cache manager
		*/
		[[nodiscard]] inline GraphicsProgramCacheManager& getGraphicsProgramCacheManager()
		{
			return mGraphicsProgramCacheManager;
		}

		/**
		*  @brief
		*    Request a graphics pipeline state cache instance by combination
		*
		*  @param[in] serializeGraphicsdPipelineStateHash
		*    FNV1a hash of "Renderer::SerializedGraphicsPipelineState"
		*  @param[in] shaderProperties
		*    Shader properties to use
		*  @param[in] allowEmergencySynchronousCompilation
		*    Allow emergency synchronous compilation if no fallback could be found? This will result in a runtime hiccup instead of graphics artifacts.
		*
		*  @return
		*    The requested graphics pipeline state cache instance, null pointer on error, do not destroy the instance
		*/
		[[nodiscard]] const GraphicsPipelineStateCache* getGraphicsPipelineStateCacheByCombination(uint32_t serializedGraphicsPipelineStateHash, const ShaderProperties& shaderProperties, bool allowEmergencySynchronousCompilation);

		/**
		*  @brief
		*    Clear the pipeline state cache manager
		*/
		void clearCache();


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		inline explicit GraphicsPipelineStateCacheManager(MaterialBlueprintResource& materialBlueprintResource) :
			mMaterialBlueprintResource(materialBlueprintResource),
			mGraphicsProgramCacheManager(*this),
			mPipelineStateObjectCacheNeedSaving(false)
		{
			// Nothing here
		}

		explicit GraphicsPipelineStateCacheManager(const GraphicsPipelineStateCacheManager&) = delete;

		inline ~GraphicsPipelineStateCacheManager()
		{
			clearCache();
		}

		GraphicsPipelineStateCacheManager& operator=(const GraphicsPipelineStateCacheManager&) = delete;

		GraphicsPipelineStateCache* getFallbackGraphicsPipelineStateCache(uint32_t serializedGraphicsPipelineStateHash, const ShaderProperties& shaderProperties);

		//[-------------------------------------------------------]
		//[ Pipeline state object cache                           ]
		//[-------------------------------------------------------]
		void loadPipelineStateObjectCache(IFile& file);

		[[nodiscard]] inline bool doesPipelineStateObjectCacheNeedSaving() const
		{
			return mPipelineStateObjectCacheNeedSaving;
		}

		void savePipelineStateObjectCache(IFile& file);


	//[-------------------------------------------------------]
	//[ Private definitions                                   ]
	//[-------------------------------------------------------]
	private:
		typedef std::unordered_map<GraphicsPipelineStateSignatureId, GraphicsPipelineStateCache*> GraphicsPipelineStateCacheByGraphicsPipelineStateSignatureId;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		MaterialBlueprintResource&									 mMaterialBlueprintResource;			///< Owner material blueprint resource
		GraphicsProgramCacheManager									 mGraphicsProgramCacheManager;
		GraphicsPipelineStateCacheByGraphicsPipelineStateSignatureId mGraphicsPipelineStateCacheByGraphicsPipelineStateSignatureId;
		bool														 mPipelineStateObjectCacheNeedSaving;	///< "true" if a cache needs saving due to changes during runtime, else "false"

		// Temporary instances to reduce the number of memory allocations/deallocations
		GraphicsPipelineStateSignature mTemporaryGraphicsPipelineStateSignature;
		ShaderProperties			   mFallbackShaderProperties;
		GraphicsPipelineStateSignature mFallbackGraphicsPipelineStateSignature;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime


// Reset warning manipulations we did above
PRAGMA_WARNING_POP

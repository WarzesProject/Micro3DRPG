#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Core/StringId.h"
#include "Core/GetInvalid.h"

#include "Renderer.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4365)	// warning C4365: 'argument': conversion from 'long' to 'unsigned int', signed/unsigned mismatch
	PRAGMA_WARNING_DISABLE_MSVC(4571)	// warning C4571: Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
	PRAGMA_WARNING_DISABLE_MSVC(4668)	// warning C4668: '_M_HYBRID_X86_ARM64' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
	#include <vector>
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef uint32_t			 ShaderCacheId;	///< Shader cache identifier, identical to the shader combination ID
	typedef StringId			 AssetId;		///< Asset identifier, internally just a POD "uint32_t", string ID scheme is "<project name>/<asset directory>/<asset name>"
	typedef std::vector<AssetId> AssetIds;


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	class ShaderCache final
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class ShaderCacheManager;			// Is creating and managing shader cache instances
		friend class GraphicsPipelineStateCompiler;	// Is creating shader cache instances
		friend class ComputePipelineStateCompiler;	// Is creating shader cache instances


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Return the shader cache ID
		*
		*  @return
		*    The shader cache ID
		*/
		[[nodiscard]] inline ShaderCacheId getShaderCacheId() const
		{
			return mShaderCacheId;
		}

		/**
		*  @brief
		*    Return master shader cache
		*
		*  @return
		*    The master shader cache, can be a null pointer, don't destroy the instance
		*/
		[[nodiscard]] inline ShaderCache* getMasterShaderCache() const
		{
			return mMasterShaderCache;
		}

		/**
		*  @brief
		*    Return shader bytecode
		*
		*  @return
		*    The shader bytecode
		*/
		[[nodiscard]] inline const Renderer::ShaderBytecode& getShaderBytecode() const
		{
			return mShaderBytecode;
		}

		/**
		*  @brief
		*    Return shader
		*
		*  @return
		*    The shader
		*/
		[[nodiscard]] inline const Renderer::IShaderPtr& getShaderPtr() const
		{
			return (nullptr != mMasterShaderCache) ? mMasterShaderCache->mShaderPtr : mShaderPtr;
		}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		inline ShaderCache() :
			mShaderCacheId(getInvalid<ShaderCacheId>()),
			mMasterShaderCache(nullptr)
		{
			// Nothing here
		}

		inline explicit ShaderCache(ShaderCacheId shaderCacheId) :
			mShaderCacheId(shaderCacheId),
			mMasterShaderCache(nullptr)
		{
			// Nothing here
		}

		inline ShaderCache(ShaderCacheId shaderCacheId, ShaderCache* masterShaderCache) :
			mShaderCacheId(shaderCacheId),
			mMasterShaderCache(masterShaderCache)
		{
			// Nothing here
		}

		inline ~ShaderCache()
		{
			// Nothing here
		}

		explicit ShaderCache(const ShaderCache&) = delete;
		ShaderCache& operator=(const ShaderCache&) = delete;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		ShaderCacheId			 mShaderCacheId;
		ShaderCache*			 mMasterShaderCache;		///< If there's a master shader cache instance, we don't own the references shader but only redirect to it (multiple shader combinations resulting in same shader source code topic), don't destroy the instance
		AssetIds				 mAssetIds;					///< List of IDs of the assets (shader blueprint, shader piece) which took part in the shader cache creation
		uint64_t				 mCombinedAssetFileHashes;	///< Combination of the file hash of all assets (shader blueprint, shader piece) which took part in the shader cache creation
		Renderer::ShaderBytecode mShaderBytecode;
		Renderer::IShaderPtr	 mShaderPtr;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

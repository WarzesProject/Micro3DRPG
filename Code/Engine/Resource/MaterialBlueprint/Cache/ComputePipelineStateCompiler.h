#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Core/Platform/PlatformTypes.h"
#include "Renderer.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4668)	// warning C4668: '_M_HYBRID_X86_ARM64' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
	#include <inttypes.h>	// For uint32_t, uint64_t etc.
PRAGMA_WARNING_POP

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4365)	// warning C4365: 'argument': conversion from 'long' to 'unsigned int', signed/unsigned mismatch
	PRAGMA_WARNING_DISABLE_MSVC(4571)	// warning C4571: Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
	PRAGMA_WARNING_DISABLE_MSVC(4623)	// warning C4623: 'std::_UInt_is_zero': default constructor was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(4625)	// warning C4625: 'std::codecvt_base': copy constructor was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(4626)	// warning C4626: 'std::codecvt<char16_t,char,_Mbstatet>': assignment operator was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(4668)	// warning C4668: '_M_HYBRID_X86_ARM64' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
	PRAGMA_WARNING_DISABLE_MSVC(4774)	// warning C4774: 'sprintf_s' : format string expected in argument 3 is not a string literal
	PRAGMA_WARNING_DISABLE_MSVC(5026)	// warning C5026: 'std::_Generic_error_category': move constructor was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(5027)	// warning C5027: 'std::_Generic_error_category': move assignment operator was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(5039)	// warning C5039: '_Thrd_start': pointer or reference to potentially throwing function passed to extern C function under -EHc. Undefined behavior may occur if this function throws an exception.
	#include <vector>
	#include <string>
	#include <atomic>	// For "std::atomic<>"
	#include <deque>
	#include <mutex>
	#include <thread>
	#include <condition_variable>
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace Renderer
{
	class IComputePipelineState;
}
namespace RendererRuntime
{
	class ShaderCache;
	class IRendererRuntime;
	class MaterialBlueprintResource;
	class ComputePipelineStateCache;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	/**
	*  @brief
	*    Compute pipeline state compiler class
	*
	*  @remarks
	*    A compute pipeline state must master the following stages in order to archive the inner wisdom:
	*    1. Asynchronous shader building
	*    2. Asynchronous shader compilation
	*    3. Synchronous renderer backend dispatch TODO(co) Asynchronous renderer backend dispatch if supported by the renderer API
	*
	*  @note
	*    - Takes care of asynchronous compute pipeline state compilation
	*/
	class ComputePipelineStateCompiler final
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class RendererRuntimeImpl;
		friend class ComputePipelineStateCacheManager;	// Only the compute pipeline state cache manager is allowed to commit compiler requests


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		[[nodiscard]] inline bool isAsynchronousCompilationEnabled() const
		{
			return mAsynchronousCompilationEnabled;
		}

		void setAsynchronousCompilationEnabled(bool enabled);

		[[nodiscard]] inline uint32_t getNumberOfCompilerThreads() const
		{
			return mNumberOfCompilerThreads;
		}

		void setNumberOfCompilerThreads(uint32_t numberOfCompilerThreads);

		[[nodiscard]] inline uint32_t getNumberOfInFlightCompilerRequests() const
		{
			return mNumberOfInFlightCompilerRequests;
		}

		inline void flushBuilderQueue()
		{
			flushQueue(mBuilderMutex, mBuilderQueue);
		}

		inline void flushCompilerQueue()
		{
			flushQueue(mCompilerMutex, mCompilerQueue);
		}

		inline void flushAllQueues()
		{
			flushBuilderQueue();
			flushCompilerQueue();
		}

		void dispatch();


	//[-------------------------------------------------------]
	//[ Private definitions                                   ]
	//[-------------------------------------------------------]
	private:
		struct CompilerRequest final
		{
			// Input
			ComputePipelineStateCache&		 computePipelineStateCache;
			// Internal
			ShaderCache*					 shaderCache;
			std::string						 shaderSourceCode;
			Renderer::IComputePipelineState* computePipelineStateObject;

			inline explicit CompilerRequest(ComputePipelineStateCache& _computePipelineStateCache) :
				computePipelineStateCache(_computePipelineStateCache),
				shaderCache(nullptr),
				computePipelineStateObject(nullptr)
			{
				// Nothing here
			}
			inline explicit CompilerRequest(const CompilerRequest& compilerRequest) :
				computePipelineStateCache(compilerRequest.computePipelineStateCache),
				shaderCache(compilerRequest.shaderCache),
				shaderSourceCode(compilerRequest.shaderSourceCode),
				computePipelineStateObject(compilerRequest.computePipelineStateObject)
			{
				// Nothing here
			}
			CompilerRequest& operator=(const CompilerRequest&) = delete;
		};

		typedef std::vector<std::thread> CompilerThreads;
		typedef std::deque<CompilerRequest> CompilerRequests;


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		explicit ComputePipelineStateCompiler(IRendererRuntime& rendererRuntime);
		explicit ComputePipelineStateCompiler(const ComputePipelineStateCompiler&) = delete;
		~ComputePipelineStateCompiler();
		ComputePipelineStateCompiler& operator=(const ComputePipelineStateCompiler&) = delete;
		void addAsynchronousCompilerRequest(ComputePipelineStateCache& computePipelineStateCache);
		void instantSynchronousCompilerRequest(MaterialBlueprintResource& materialBlueprintResource, ComputePipelineStateCache& computePipelineStateCache);
		void flushQueue(std::mutex& mutex, const CompilerRequests& compilerRequests);
		void builderThreadWorker();
		void compilerThreadWorker();
		[[nodiscard]] Renderer::IComputePipelineState* createComputePipelineState(const RendererRuntime::MaterialBlueprintResource& materialBlueprintResource, Renderer::IShader& shader) const;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		IRendererRuntime&	  mRendererRuntime;	///< Renderer runtime instance, do not destroy the instance
		bool				  mAsynchronousCompilationEnabled;
		uint32_t			  mNumberOfCompilerThreads;
		std::atomic<uint32_t> mNumberOfInFlightCompilerRequests;

		// Asynchronous building (moderate cost)
		std::atomic<bool>		mShutdownBuilderThread;
		std::mutex				mBuilderMutex;
		std::condition_variable	mBuilderConditionVariable;
		CompilerRequests		mBuilderQueue;
		std::thread				mBuilderThread;

		// Asynchronous compilation (nuts cost)
		std::atomic<bool>		mShutdownCompilerThread;
		std::mutex				mCompilerMutex;
		std::condition_variable	mCompilerConditionVariable;
		CompilerRequests		mCompilerQueue;
		CompilerThreads			mCompilerThreads;

		// Synchronous dispatch
		std::mutex		 mDispatchMutex;
		CompilerRequests mDispatchQueue;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

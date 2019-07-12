#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Asset/Asset.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4365)	// warning C4365: 'return': conversion from 'int' to 'std::char_traits<wchar_t>::int_type', signed/unsigned mismatch
	PRAGMA_WARNING_DISABLE_MSVC(4623)	// warning C4623: 'std::_UInt_is_zero': default constructor was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(4626)	// warning C4626: 'std::_UInt_is_zero': assignment operator was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(4668)	// warning C4668: '_M_HYBRID_X86_ARM64' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
	PRAGMA_WARNING_DISABLE_MSVC(5027)	// warning C5027: 'std::_UInt_is_zero': move assignment operator was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(5039)	// warning C5039: '_Thrd_start': pointer or reference to potentially throwing function passed to extern C function under -EHc. Undefined behavior may occur if this function throws an exception.
	#include <atomic>	// For "std::atomic<>"
	#include <deque>
	#include <mutex>
	#include <thread>
	#include <unordered_map>
	#include <condition_variable>
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class IResource;
	class IResourceLoader;
	class IResourceManager;
	class IRendererRuntime;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef uint32_t ResourceId;			///< POD resource identifier
	typedef StringId ResourceLoaderTypeId;	///< Resource loader type identifier, internally just a POD "uint32_t", usually created by hashing the file format extension (if the resource loader is processing file data in the first place)


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	/**
	*  @brief
	*    Resource streamer responsible for getting the resource data into memory
	*
	*  @remarks
	*    By default, asynchronous resource streaming is used. This is also known as
	*    - Asynchronous content streaming
	*    - Asynchronous asset loading
	*    - Asynchronous data streaming
	*    - Background resource loading
	*
	*    A resource must master the following stages in order to archive the inner wisdom:
	*    1. Asynchronous deserialization
	*    2. Asynchronous processing
	*    3. Synchronous dispatch, e.g. to the renderer backend
	*
	*  @todo
	*    - TODO(co) It might make sense to use lock-free-queues in here
	*/
	class ResourceStreamer final
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class RendererRuntimeImpl;


	//[-------------------------------------------------------]
	//[ Public definitions                                    ]
	//[-------------------------------------------------------]
	public:
		struct LoadRequest final
		{
			// Data provided from the outside
			const Asset*		 asset;					///< Used asset, must be valid
			ResourceLoaderTypeId resourceLoaderTypeId;	///< Must be valid
			bool				 reload;				///< "true" if the resource is new in memory, else "false" for reload an already loaded resource (and e.g. update cache entries)
			IResourceManager*	 resourceManager;		///< Must be valid, do not destroy the instance
			ResourceId			 resourceId;			///< Must be valid
			// In-flight data
			mutable IResourceLoader* resourceLoader;	///< Null pointer at first, must be valid as soon as the load request is in-flight, do not destroy the instance
			bool					 loadingFailed;		///< "true" if loading failed, else "false"

			// Methods
			inline LoadRequest(const Asset& _asset, ResourceLoaderTypeId _resourceLoaderTypeId, bool _reload, IResourceManager& _resourceManager, ResourceId _resourceId) :
				asset(&_asset),
				resourceLoaderTypeId(_resourceLoaderTypeId),
				reload(_reload),
				resourceManager(&_resourceManager),
				resourceId(_resourceId),
				resourceLoader(nullptr),
				loadingFailed(false)
			{
				// Nothing here
			}
			[[nodiscard]] IResource& getResource() const;
		};


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		[[nodiscard]] inline uint32_t getNumberOfInFlightLoadRequests() const
		{
			return mNumberOfInFlightLoadRequests;
		}

		void commitLoadRequest(const LoadRequest& loadRequest);
		void flushAllQueues();

		/**
		*  @brief
		*    Resource streamer update performing dispatch to e.g. the renderer backend
		*
		*  @note
		*    - Call this once per frame
		*/
		void dispatch();


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		explicit ResourceStreamer(IRendererRuntime& rendererRuntime);
		~ResourceStreamer();
		explicit ResourceStreamer(const ResourceStreamer&) = delete;
		ResourceStreamer& operator=(const ResourceStreamer&) = delete;
		void deserializationThreadWorker();
		void processingThreadWorker();
		void finalizeLoadRequest(const LoadRequest& loadRequest);


	//[-------------------------------------------------------]
	//[ Private definitions                                   ]
	//[-------------------------------------------------------]
	private:
		typedef std::vector<IResourceLoader*> ResourceLoaders;
		typedef std::deque<LoadRequest> LoadRequests;
		struct ResourceLoaderType final
		{
			uint32_t		numberOfInstances;
			ResourceLoaders	freeResourceLoaders;
			LoadRequests	waitingLoadRequests;
		};
		typedef std::unordered_map<uint32_t, ResourceLoaderType> ResourceLoaderTypeManager;	///< Key = "RendererRuntime::ResourceLoaderTypeId"


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		IRendererRuntime&	  mRendererRuntime;	///< Renderer runtime instance, do not destroy the instance
		std::mutex			  mResourceManagerMutex;
		std::atomic<uint32_t> mNumberOfInFlightLoadRequests;
		// Resource streamer stage: 1. Asynchronous deserialization
		std::atomic<bool>		    mShutdownDeserializationThread;
		std::mutex					mDeserializationMutex;
		std::condition_variable		mDeserializationConditionVariable;
		LoadRequests				mDeserializationQueue;
		ResourceLoaderTypeManager	mResourceLoaderTypeManager;	// Do only touch if "mResourceManagerMutex" is locked
		std::atomic<uint32_t>		mDeserializationWaitingQueueRequests;
		std::thread					mDeserializationThread;
		// Resource streamer stage: 2. Asynchronous processing
		std::atomic<bool>		mShutdownProcessingThread;
		std::mutex				mProcessingMutex;
		std::condition_variable mProcessingConditionVariable;
		LoadRequests			mProcessingQueue;
		std::thread				mProcessingThread;
		// Resource streamer stage: 3. Synchronous dispatch to e.g. the renderer backend
		std::mutex	 mDispatchMutex;
		LoadRequests mDispatchQueue;
		LoadRequests mFullyLoadedWaitingQueue;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

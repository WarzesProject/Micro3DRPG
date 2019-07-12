#include "stdafx.h"
#include "Resource/ResourceStreamer.h"
#include "Resource/IResourceLoader.h"
#include "Resource/IResourceManager.h"
#include "Core/Platform/PlatformManager.h"
#include "Core/File/IFileManager.h"
#include "IRendererRuntime.h"

// TODO(co) Can we do something about the warning which does not involve using "std::thread"-pointers?
PRAGMA_WARNING_DISABLE_MSVC(4355)	// warning C4355: 'this': used in base member initializer list


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public RendererRuntime::ResourceStreamer::LoadRequest methods ]
	//[-------------------------------------------------------]
	IResource& ResourceStreamer::LoadRequest::getResource() const
	{
		return resourceManager->getResourceByResourceId(resourceId);
	}


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	void ResourceStreamer::commitLoadRequest(const LoadRequest& loadRequest)
	{
		// The first thing we do: Update the resource loading state
		++mNumberOfInFlightLoadRequests;
		loadRequest.getResource().setLoadingState(IResource::LoadingState::LOADING);

		// Push the load request into the queue of the first resource streamer pipeline stage
		// -> Resource streamer stage: 1. Asynchronous deserialization
		std::unique_lock<std::mutex> deserializationMutexLock(mDeserializationMutex);
		mDeserializationQueue.push_back(loadRequest);
		deserializationMutexLock.unlock();
		mDeserializationConditionVariable.notify_one();
	}

	void ResourceStreamer::flushAllQueues()
	{
		bool everythingFlushed = false;
		do
		{
			{ // Process
				{ // Resource streamer stage: 1. Asynchronous deserialization
					std::lock_guard<std::mutex> deserializationMutexLock(mDeserializationMutex);
					everythingFlushed = (mDeserializationQueue.empty() && 0 == mDeserializationWaitingQueueRequests);
				}

				// Resource streamer stage: 2. Asynchronous processing
				if (everythingFlushed)
				{
					std::lock_guard<std::mutex> processingMutexLock(mProcessingMutex);
					everythingFlushed = mProcessingQueue.empty();
				}

				// Resource streamer stage: 3. Synchronous dispatch to e.g. the renderer backend
				if (everythingFlushed)
				{
					std::lock_guard<std::mutex> dispatchMutexLock(mDispatchMutex);
					everythingFlushed = (mDispatchQueue.empty() && mFullyLoadedWaitingQueue.empty());
				}
			}
			dispatch();

			// Wait for a moment to not totally pollute the CPU
			if (!everythingFlushed)
			{
				using namespace std::chrono_literals;
				std::this_thread::sleep_for(1ms);
			}
		} while (!everythingFlushed);

		// Sanity check
		RENDERER_ASSERT(mRendererRuntime.getContext(), 0 == mNumberOfInFlightLoadRequests, "Invalid number of in flight load requests")
	}

	void ResourceStreamer::dispatch()
	{
		// Resource streamer stage: 3. Synchronous dispatch to e.g. the renderer backend

		// Continue as long as there's a load request left inside the queue
		bool stillInTimeBudget = true;	// TODO(co) Add a maximum time budget so we're not blocking too long (the show must go on)
		while (stillInTimeBudget)
		{
			// Get the load request
			std::unique_lock<std::mutex> dispatchMutexLock(mDispatchMutex);
			if (mDispatchQueue.empty())
			{
				break;
			}
			LoadRequest loadRequest = mDispatchQueue.front();
			mDispatchQueue.pop_front();
			dispatchMutexLock.unlock();

			// Do the work
			if (loadRequest.loadingFailed || loadRequest.resourceLoader->onDispatch())
			{
				// Load request is finished now
				finalizeLoadRequest(loadRequest);
			}
			else
			{
				mFullyLoadedWaitingQueue.push_back(loadRequest);
			}
		}

		// Check fully loaded waiting queue
		for (LoadRequests::iterator iterator = mFullyLoadedWaitingQueue.begin(); iterator != mFullyLoadedWaitingQueue.end();)
		{
			const LoadRequest& loadRequest = *iterator;
			if (loadRequest.resourceLoader->isFullyLoaded())
			{
				// Load request is finished now
				finalizeLoadRequest(loadRequest);

				// Remove from queue
				iterator = mFullyLoadedWaitingQueue.erase(iterator);
			}
			else
			{
				// Next, please
				++iterator;
			}
		}
	}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	ResourceStreamer::ResourceStreamer(IRendererRuntime& rendererRuntime) :
		mRendererRuntime(rendererRuntime),
		mNumberOfInFlightLoadRequests(0),
		mShutdownDeserializationThread(false),
		mDeserializationWaitingQueueRequests(0),
		mDeserializationThread(&ResourceStreamer::deserializationThreadWorker, this),
		mShutdownProcessingThread(false),
		mProcessingThread(&ResourceStreamer::processingThreadWorker, this)
	{
		// Nothing here
	}

	ResourceStreamer::~ResourceStreamer()
	{
		// Deserialization thread and processing thread shutdown
		mShutdownDeserializationThread = true;
		mShutdownProcessingThread = true;
		mDeserializationConditionVariable.notify_one();
		mProcessingConditionVariable.notify_one();
		mDeserializationThread.join();
		mProcessingThread.join();

		// Destroy resource loader instances
		for (auto& resourceLoaderType : mResourceLoaderTypeManager)
		{
			for (IResourceLoader* resourceLoader : resourceLoaderType.second.freeResourceLoaders)
			{
				delete resourceLoader;
			}
		}
	}

	void ResourceStreamer::deserializationThreadWorker()
	{
		RENDERER_RUNTIME_SET_CURRENT_THREAD_DEBUG_NAME("RS: Stage 1", "Renderer runtime: Resource streamer stage: 1. Asynchronous deserialization");

		// Resource streamer stage: 1. Asynchronous deserialization
		while (!mShutdownDeserializationThread)
		{
			// Continue as long as there's a load request left inside the queue, if it's empty go to sleep
			std::unique_lock<std::mutex> deserializationMutexLock(mDeserializationMutex);
			mDeserializationConditionVariable.wait(deserializationMutexLock);
			while (!mDeserializationQueue.empty() && !mShutdownDeserializationThread)
			{
				// Get the load request
				LoadRequest loadRequest = mDeserializationQueue.front();
				mDeserializationQueue.pop_front();

				{ // Get resource loader instance
					std::lock_guard<std::mutex> resourceManagerMutexLock(mResourceManagerMutex);
					const ResourceLoaderTypeId resourceLoaderTypeId = loadRequest.resourceLoaderTypeId;
					ResourceLoaderTypeManager::iterator iterator = mResourceLoaderTypeManager.find(resourceLoaderTypeId);
					if (mResourceLoaderTypeManager.cend() == iterator)
					{
						// The resource loader type ID is unknown, yet
						ResourceLoaderType resourceLoaderType;
						resourceLoaderType.numberOfInstances = 1;
						mResourceLoaderTypeManager.emplace(resourceLoaderTypeId, resourceLoaderType);
						loadRequest.resourceLoader = loadRequest.resourceManager->createResourceLoaderInstance(resourceLoaderTypeId);
					}
					else
					{
						// The resource loader type ID is already known

						// First check whether or not we're able to reuse a free resource loader instance
						ResourceLoaderType& resourceLoaderType = iterator->second;
						ResourceLoaders& freeResourceLoaders = resourceLoaderType.freeResourceLoaders;
						if (freeResourceLoaders.empty())
						{
							// In order to keep the memory consumption under control, we limit the number of simultaneous resource loader type instances
							if (resourceLoaderType.numberOfInstances < 5)
							{
								loadRequest.resourceLoader = loadRequest.resourceManager->createResourceLoaderInstance(resourceLoaderTypeId);
								RENDERER_ASSERT(mRendererRuntime.getContext(), nullptr != loadRequest.resourceLoader, "Invalid load request resource loader")
								++resourceLoaderType.numberOfInstances;
							}
							else
							{
								// We were unable to acquire a resource loader instance, we just have to try it later again
								resourceLoaderType.waitingLoadRequests.push_back(loadRequest);
								++mDeserializationWaitingQueueRequests;
							}
						}
						else
						{
							loadRequest.resourceLoader = freeResourceLoaders.back();
							freeResourceLoaders.pop_back();
						}
					}
				}

				// If we've got a resource loader instance now, let's continue with the resource streaming pipeline
				if (nullptr != loadRequest.resourceLoader)
				{
					deserializationMutexLock.unlock();
					loadRequest.resourceLoader->initialize(*loadRequest.asset, loadRequest.reload, loadRequest.getResource());

					// Do the work
					if (loadRequest.resourceLoader->hasDeserialization())
					{
						IFileManager& fileManager = mRendererRuntime.getFileManager();
						IFile* file = fileManager.openFile(IFileManager::FileMode::READ, loadRequest.resourceLoader->getAsset().virtualFilename);
						if (nullptr != file)
						{
							if (loadRequest.resourceLoader->onDeserialization(*file))
							{
								// Push the load request into the queue of the next resource streamer pipeline stage
								if (loadRequest.resourceLoader->hasProcessing())
								{
									// Resource streamer stage: 2. Asynchronous processing
									std::unique_lock<std::mutex> processingMutexLock(mProcessingMutex);
									mProcessingQueue.push_back(loadRequest);
									processingMutexLock.unlock();
									mProcessingConditionVariable.notify_one();
								}
								else
								{
									// Resource streamer stage: 3. Synchronous dispatch to e.g. the renderer backend
									std::lock_guard<std::mutex> dispatchMutexLock(mDispatchMutex);
									mDispatchQueue.push_back(loadRequest);
								}
							}
							else
							{
								// Resource streamer stage: 3. Synchronous dispatch to finish off the failed loading attempt
								loadRequest.loadingFailed = true;
								std::lock_guard<std::mutex> dispatchMutexLock(mDispatchMutex);
								mDispatchQueue.push_back(loadRequest);
							}
							fileManager.closeFile(*file);
						}
						else
						{
							// Error! This is horrible, now we've got a zombie inside the resource streamer. We could let it crash, but maybe the zombie won't directly eat brains.
							RENDERER_ASSERT(mRendererRuntime.getContext(), false, "We should never end up in here")
						}
					}
					else
					{
						// Push the load request into the queue of the next resource streamer pipeline stage
						// -> Resource streamer stage: 2. Asynchronous processing
						std::unique_lock<std::mutex> processingMutexLock(mProcessingMutex);
						mProcessingQueue.push_back(loadRequest);
						processingMutexLock.unlock();
						mProcessingConditionVariable.notify_one();
					}

					// We're ready for the next round
					deserializationMutexLock.lock();
				}
			}
		}
	}

	void ResourceStreamer::processingThreadWorker()
	{
		RENDERER_RUNTIME_SET_CURRENT_THREAD_DEBUG_NAME("RS: Stage 2", "Renderer runtime: Resource streamer stage: 2. Asynchronous processing");

		// Resource streamer stage: 2. Asynchronous processing
		while (!mShutdownProcessingThread)
		{
			// Continue as long as there's a load request left inside the queue, if it's empty go to sleep
			std::unique_lock<std::mutex> processingMutexLock(mProcessingMutex);
			mProcessingConditionVariable.wait(processingMutexLock);
			while (!mProcessingQueue.empty() && !mShutdownProcessingThread)
			{
				// Get the load request
				LoadRequest loadRequest = mProcessingQueue.front();
				mProcessingQueue.pop_front();
				processingMutexLock.unlock();

				// Do the work
				loadRequest.resourceLoader->onProcessing();

				{ // Push the load request into the queue of the next resource streamer pipeline stage
				  // -> Resource streamer stage: 3. Synchronous dispatch to e.g. the renderer backend
					std::lock_guard<std::mutex> dispatchMutexLock(mDispatchMutex);
					mDispatchQueue.push_back(loadRequest);
				}

				// We're ready for the next round
				processingMutexLock.lock();
			}
		}
	}

	void ResourceStreamer::finalizeLoadRequest(const LoadRequest& loadRequest)
	{
		{ // Release the resource loader instance
			std::unique_lock<std::mutex> resourceManagerMutexLock(mResourceManagerMutex);
			ResourceLoaderTypeManager::iterator iterator = mResourceLoaderTypeManager.find(loadRequest.resourceLoaderTypeId);
			if (mResourceLoaderTypeManager.cend() != iterator)
			{
				#ifdef _DEBUG
					loadRequest.getResource().setDebugName(std::string(loadRequest.resourceLoader->getAsset().virtualFilename) + IFileManager::INVALID_CHARACTER + "[Loaded]");
				#endif

				// The resource loader instance is free now and ready to be reused
				iterator->second.freeResourceLoaders.push_back(loadRequest.resourceLoader);

				// Check whether or not another resource streamer load request is already waiting for the just released resource loader instance
				LoadRequests& waitingLoadRequests = iterator->second.waitingLoadRequests;
				if (!waitingLoadRequests.empty())
				{
					// Get the waiting resource streamer load request and immediately release our resource manager mutex
					LoadRequest waitingLoadRequest = waitingLoadRequests.front();
					waitingLoadRequests.pop_front();
					RENDERER_ASSERT(mRendererRuntime.getContext(), 0 != mDeserializationWaitingQueueRequests, "Invalid deserialization waiting queue requests")
					--mDeserializationWaitingQueueRequests;
					resourceManagerMutexLock.unlock();

					// Throw the fish back into the ocean
					std::unique_lock<std::mutex> deserializationMutexLock(mDeserializationMutex);
					mDeserializationQueue.push_back(waitingLoadRequest);
					deserializationMutexLock.unlock();
					mDeserializationConditionVariable.notify_one();
				}
			}
			else
			{
				// Error! This shouldn't be possible if we're in here
				RENDERER_ASSERT(mRendererRuntime.getContext(), false, "We should never end up in here")
			}
		}

		// The last thing we do: Update the resource loading state
		loadRequest.getResource().setLoadingState(loadRequest.loadingFailed ? IResource::LoadingState::FAILED : IResource::LoadingState::LOADED);
		RENDERER_ASSERT(mRendererRuntime.getContext(), 0 != mNumberOfInFlightLoadRequests, "Invalid number of in flight load requests")
		--mNumberOfInFlightLoadRequests;
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

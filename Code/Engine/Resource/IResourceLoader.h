#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Core/Loader.h"
#include "Asset/Asset.h"


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class IFile;
	class IResource;
	class IResourceManager;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef StringId ResourceLoaderTypeId;	///< Resource loader type identifier, internally just a POD "uint32_t", usually created by hashing the file format extension (if the resource loader is processing file data in the first place)


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	class IResourceLoader : protected Loader
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class ResourceStreamer;	// Needs to be able to destroy resource loader instances


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Return the owner resource manager
		*
		*  @return
		*    The owner resource manager
		*/
		[[nodiscard]] inline IResourceManager& getResourceManager() const
		{
			return mResourceManager;
		}

		/**
		*  @brief
		*    Return the asset the resource is using
		*
		*  @return
		*    The asset the resource is using
		*/
		[[nodiscard]] inline const Asset& getAsset() const
		{
			assert(nullptr != mAsset);
			return *mAsset;
		}

		/**
		*  @brief
		*    Return whether or not the resource gets reloaded or not
		*
		*  @return
		*    "true" if the resource is new in memory, else "false" for reload an already loaded resource (and e.g. update cache entries)
		*/
		[[nodiscard]] inline bool getReload() const
		{
			return mReload;
		}


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::IResourceLoader methods ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Return the resource loader type ID
		*/
		[[nodiscard]] virtual ResourceLoaderTypeId getResourceLoaderTypeId() const = 0;

		/**
		*  @brief
		*    Asynchronous initialize the resource loader type ID
		*
		*  @param[in] asset
		*    Asset to load
		*  @param[in] reload
		*    "true" if the resource is new in memory, else "false" for reload an already loaded resource (and e.g. update cache entries)
		*  @param[out] resource
		*    Resource instance to fill
		*/
		virtual void initialize(const Asset& asset, bool reload, IResource& resource) = 0;

		/**
		*  @brief
		*    Asynchronous called to check whether or not the resource loader has to deserialize (usually from file)
		*
		*  @return
		*    "true" if deserialization has to be called, else "false" (for example a procedural resource or a resource received via an API like OpenVR)
		*/
		[[nodiscard]] virtual bool hasDeserialization() const = 0;

		/**
		*  @brief
		*    Asynchronous called when the resource loader has to deserialize (usually from file) the internal data into memory
		*
		*  @param[in] file
		*    File to read from
		*
		*  @return
		*    "true" if all went fine, else "false"
		*/
		[[nodiscard]] virtual bool onDeserialization(IFile& file) = 0;

		/**
		*  @brief
		*    Asynchronous called to check whether or not the resource loader has to perform processing
		*
		*  @return
		*    "true" if processing has to be called, else "false" (if for example deserialized compressed data stays compressed in memory)
		*/
		[[nodiscard]] virtual bool hasProcessing() const = 0;

		/**
		*  @brief
		*    Asynchronous called when the resource loader has to perform internal in-memory data processing
		*/
		virtual void onProcessing() = 0;

		/**
		*  @brief
		*    Synchronous called when the resource loader has to dispatch the data (e.g. to the renderer backend)
		*
		*  @return
		*    "true" if the resource is fully loaded, else "false" (e.g. asset dependencies are not fully loaded, yet) meaning this method will be called later on again
		*/
		[[nodiscard]] virtual bool onDispatch() = 0;

		/**
		*  @brief
		*    Synchronous called when the resource loader is about to switch the resource into the loaded state
		*
		*  @return
		*    "true" if the resource is fully loaded, else "false" (e.g. asset dependencies are not fully loaded, yet) meaning this method will be called later on again
		*/
		[[nodiscard]] virtual bool isFullyLoaded() = 0;


	//[-------------------------------------------------------]
	//[ Protected methods                                     ]
	//[-------------------------------------------------------]
	protected:
		inline explicit IResourceLoader(IResourceManager& resourceManager) :
			mResourceManager(resourceManager),
			mAsset(nullptr),
			mReload(false)
		{
			// Nothing here
		}

		inline virtual ~IResourceLoader()
		{
			// Nothing here
		}

		explicit IResourceLoader(const IResourceLoader&) = delete;
		IResourceLoader& operator=(const IResourceLoader&) = delete;

		inline void initialize(const Asset& asset, bool reload)
		{
			mAsset  = &asset;
			mReload = reload;
		}


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		IResourceManager& mResourceManager;	///< Owner resource manager
		const Asset*	  mAsset;			///< Used asset, must be valid
		bool			  mReload;			///< "true" if the resource is new in memory, else "false" for reload an already loaded resource (and e.g. update cache entries)


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Resource/IResourceLoader.h"
#include "Core/File/MemoryFile.h"


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class IRendererRuntime;
	class VertexAttributesResource;
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
	typedef uint32_t VertexAttributesResourceId;	///< POD vertex attributes resource identifier


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	class VertexAttributesResourceLoader final : public IResourceLoader
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend ResourceManagerTemplate<VertexAttributesResource, VertexAttributesResourceLoader, VertexAttributesResourceId, 32>;	// Type definition of template class


	//[-------------------------------------------------------]
	//[ Public definitions                                    ]
	//[-------------------------------------------------------]
	public:
		static constexpr uint32_t TYPE_ID = STRING_ID("vertex_attributes");


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

		[[nodiscard]] inline virtual bool onDispatch() override
		{
			// Fully loaded
			return true;
		}

		[[nodiscard]] inline virtual bool isFullyLoaded() override
		{
			// Fully loaded
			return true;
		}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		inline VertexAttributesResourceLoader(IResourceManager& resourceManager, IRendererRuntime& rendererRuntime) :
			IResourceLoader(resourceManager),
			mRendererRuntime(rendererRuntime),
			mVertexAttributesResource(nullptr)
		{
			// Nothing here
		}

		inline virtual ~VertexAttributesResourceLoader() override
		{
			// Nothing here
		}

		explicit VertexAttributesResourceLoader(const VertexAttributesResourceLoader&) = delete;
		VertexAttributesResourceLoader& operator=(const VertexAttributesResourceLoader&) = delete;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		IRendererRuntime&		  mRendererRuntime;				///< Renderer runtime instance, do not destroy the instance
		VertexAttributesResource* mVertexAttributesResource;	///< Destination resource

		// Temporary data
		MemoryFile mMemoryFile;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

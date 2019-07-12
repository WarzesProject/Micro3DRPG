#include "stdafx.h"
#include "Resource/CompositorWorkspace/Loader/CompositorWorkspaceResourceLoader.h"
#include "Resource/CompositorWorkspace/Loader/CompositorWorkspaceFileFormat.h"
#include "Resource/CompositorWorkspace/CompositorWorkspaceResource.h"


// TODO(co) Error handling


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
namespace
{
	namespace detail
	{


		//[-------------------------------------------------------]
		//[ Global functions                                      ]
		//[-------------------------------------------------------]
		void nodesDeserialization(RendererRuntime::IFile& file, RendererRuntime::CompositorWorkspaceResource& compositorWorkspaceResource)
		{
			// Read in the compositor workspace resource nodes
			RendererRuntime::v1CompositorWorkspace::Nodes nodes;
			file.read(&nodes, sizeof(RendererRuntime::v1CompositorWorkspace::Nodes));

			// Sanity check
			assert((nodes.numberOfNodes > 0) && "Invalid compositor workspace asset without any nodes detected");

			// Read in the compositor node asset IDs
			compositorWorkspaceResource.reserveCompositorNodes(nodes.numberOfNodes);
			// TODO(co) Get rid of the evil const-cast
			RendererRuntime::CompositorWorkspaceResource::CompositorNodeAssetIds& compositorNodeAssetIds = const_cast<RendererRuntime::CompositorWorkspaceResource::CompositorNodeAssetIds&>(compositorWorkspaceResource.getCompositorNodeAssetIds());
			compositorNodeAssetIds.resize(nodes.numberOfNodes);
			file.read(compositorNodeAssetIds.data(), sizeof(RendererRuntime::AssetId) * nodes.numberOfNodes);
		}


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
	} // detail
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::IResourceLoader methods ]
	//[-------------------------------------------------------]
	void CompositorWorkspaceResourceLoader::initialize(const Asset& asset, bool reload, IResource& resource)
	{
		IResourceLoader::initialize(asset, reload);
		mCompositorWorkspaceResource = static_cast<CompositorWorkspaceResource*>(&resource);
	}

	bool CompositorWorkspaceResourceLoader::onDeserialization(IFile& file)
	{
		// Tell the memory mapped file about the LZ4 compressed data
		return mMemoryFile.loadLz4CompressedDataFromFile(v1CompositorWorkspace::FORMAT_TYPE, v1CompositorWorkspace::FORMAT_VERSION, file);
	}

	void CompositorWorkspaceResourceLoader::onProcessing()
	{
		// Decompress LZ4 compressed data
		mMemoryFile.decompress();

		// Read in the compositor workspace header
		// TODO(co) Currently the compositor workspace header is unused
		v1CompositorWorkspace::CompositorWorkspaceHeader compositorWorkspaceHeader;
		mMemoryFile.read(&compositorWorkspaceHeader, sizeof(v1CompositorWorkspace::CompositorWorkspaceHeader));

		// Read in the compositor workspace resource nodes
		::detail::nodesDeserialization(mMemoryFile, *mCompositorWorkspaceResource);
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

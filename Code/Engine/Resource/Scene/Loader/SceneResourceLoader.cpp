#include "stdafx.h"
#include "Resource/Scene/Loader/SceneResourceLoader.h"
#include "Resource/Scene/Loader/SceneFileFormat.h"
#include "Resource/Scene/Item/ISceneItem.h"
#include "Resource/Scene/SceneResource.h"
#include "IRendererRuntime.h"


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
		void itemDeserialization(RendererRuntime::IFile& file, RendererRuntime::SceneResource& sceneResource, RendererRuntime::SceneNode& sceneNode)
		{
			// Read the scene item header
			RendererRuntime::v1Scene::ItemHeader itemHeader;
			file.read(&itemHeader, sizeof(RendererRuntime::v1Scene::ItemHeader));

			// Create the scene item
			RendererRuntime::ISceneItem* sceneItem = sceneResource.createSceneItem(itemHeader.typeId, sceneNode);
			if (nullptr != sceneItem && 0 != itemHeader.numberOfBytes)
			{
				// Load in the scene item data
				// TODO(co) Get rid of the new/delete in here
				uint8_t* data = new uint8_t[itemHeader.numberOfBytes];
				file.read(data, itemHeader.numberOfBytes);

				// Deserialize the scene item
				sceneItem->deserialize(itemHeader.numberOfBytes, data);

				// Cleanup
				delete [] data;
			}
			else
			{
				// TODO(co) Error handling
			}
		}

		void nodeDeserialization(RendererRuntime::IFile& file, RendererRuntime::SceneResource& sceneResource)
		{
			// Read in the scene node
			RendererRuntime::v1Scene::Node node;
			file.read(&node, sizeof(RendererRuntime::v1Scene::Node));

			// Create the scene node
			RendererRuntime::SceneNode* sceneNode = sceneResource.createSceneNode(node.transform);
			if (nullptr != sceneNode)
			{
				// Read in the scene items
				for (uint32_t i = 0; i < node.numberOfItems; ++i)
				{
					itemDeserialization(file, sceneResource, *sceneNode);
				}
			}
			else
			{
				// TODO(co) Error handling
			}
		}

		void nodesDeserialization(RendererRuntime::IFile& file, RendererRuntime::SceneResource& sceneResource)
		{
			// Read in the scene nodes
			RendererRuntime::v1Scene::Nodes nodes;
			file.read(&nodes, sizeof(RendererRuntime::v1Scene::Nodes));

			// Sanity check
			assert((nodes.numberOfNodes > 0) && "Invalid scene asset without any nodes detected");

			// Read in the scene nodes
			for (uint32_t i = 0; i < nodes.numberOfNodes; ++i)
			{
				nodeDeserialization(file, sceneResource);
			}
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
	void SceneResourceLoader::initialize(const Asset& asset, bool reload, IResource& resource)
	{
		IResourceLoader::initialize(asset, reload);
		mSceneResource = static_cast<SceneResource*>(&resource);
	}

	bool SceneResourceLoader::onDeserialization(IFile& file)
	{
		// Tell the memory mapped file about the LZ4 compressed data
		return mMemoryFile.loadLz4CompressedDataFromFile(v1Scene::FORMAT_TYPE, v1Scene::FORMAT_VERSION, file);
	}

	void SceneResourceLoader::onProcessing()
	{
		// Decompress LZ4 compressed data
		mMemoryFile.decompress();

		// Read in the scene header
		// TODO(co) Currently the scene header is unused
		v1Scene::SceneHeader sceneHeader;
		mMemoryFile.read(&sceneHeader, sizeof(v1Scene::SceneHeader));

		// Can we create the renderer resource asynchronous as well?
		// -> For example scene items might create renderer resources, so we have to check for native renderer multithreading support in here
		if (mRendererRuntime.getRenderer().getCapabilities().nativeMultithreading)
		{
			// Read in the scene resource nodes
			::detail::nodesDeserialization(mMemoryFile, *mSceneResource);
		}
	}

	bool SceneResourceLoader::onDispatch()
	{
		// Can we create the renderer resource asynchronous as well?
		// -> For example scene items might create renderer resources, so we have to check for native renderer multithreading support in here
		if (!mRendererRuntime.getRenderer().getCapabilities().nativeMultithreading)
		{
			// Read in the scene resource nodes
			::detail::nodesDeserialization(mMemoryFile, *mSceneResource);
		}

		// Fully loaded
		return true;
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

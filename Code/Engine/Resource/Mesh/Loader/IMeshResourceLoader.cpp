#include "stdafx.h"
#include "Resource/Mesh/Loader/IMeshResourceLoader.h"
#include "Resource/Mesh/MeshResource.h"
#include "Resource/Material/MaterialResourceManager.h"
#include "IRendererRuntime.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::IResourceLoader methods ]
	//[-------------------------------------------------------]
	void IMeshResourceLoader::initialize(const Asset& asset, bool reload, IResource& resource)
	{
		IResourceLoader::initialize(asset, reload);
		mMeshResource = static_cast<MeshResource*>(&resource);
	}

	bool IMeshResourceLoader::isFullyLoaded()
	{
		// Fully loaded?
		const MaterialResourceManager& materialResourceManager = mRendererRuntime.getMaterialResourceManager();
		const SubMeshes& subMeshes = mMeshResource->getSubMeshes();
		const uint32_t numberOfUsedSubMeshes = static_cast<uint32_t>(subMeshes.size());
		for (uint32_t i = 0; i < numberOfUsedSubMeshes; ++i)
		{
			if (IResource::LoadingState::LOADED != materialResourceManager.getResourceByResourceId(subMeshes[i].getMaterialResourceId()).getLoadingState())
			{
				// Not fully loaded
				return false;
			}
		}

		// Fully loaded
		return true;
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

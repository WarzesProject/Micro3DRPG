#include "stdafx.h"
#include "Resource/Material/MaterialResourceManager.h"
#include "Resource/Material/MaterialResource.h"
#include "Resource/Material/MaterialTechnique.h"
#include "Resource/Material/Loader/MaterialResourceLoader.h"
#include "Resource/MaterialBlueprint/MaterialBlueprintResourceManager.h"
#include "Resource/MaterialBlueprint/MaterialBlueprintResource.h"
#include "Resource/ResourceManagerTemplate.h"
#include "Core/File/IFileManager.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	MaterialResource* MaterialResourceManager::getMaterialResourceByAssetId(AssetId assetId) const
	{
		return mInternalResourceManager->getResourceByAssetId(assetId);
	}

	MaterialResourceId MaterialResourceManager::getMaterialResourceIdByAssetId(AssetId assetId) const
	{
		const MaterialResource* materialResource = getMaterialResourceByAssetId(assetId);
		return (nullptr != materialResource) ? materialResource->getId() : getInvalid<MaterialResourceId>();
	}

	void MaterialResourceManager::loadMaterialResourceByAssetId(AssetId assetId, MaterialResourceId& materialResourceId, IResourceListener* resourceListener, bool reload, ResourceLoaderTypeId resourceLoaderTypeId)
	{
		mInternalResourceManager->loadResourceByAssetId(assetId, materialResourceId, resourceListener, reload, resourceLoaderTypeId);
	}

	MaterialResourceId MaterialResourceManager::createMaterialResourceByAssetId(AssetId assetId, AssetId materialBlueprintAssetId, MaterialTechniqueId materialTechniqueId)
	{
		// Sanity check
		RENDERER_ASSERT(mRendererRuntime.getContext(), nullptr == getMaterialResourceByAssetId(assetId), "Material resource is not allowed to exist, yet")

		// Create the material resource instance
		MaterialResource& materialResource = mInternalResourceManager->getResources().addElement();
		materialResource.setResourceManager(this);
		materialResource.setAssetId(assetId);
		#ifdef _DEBUG
		{
			const AssetManager& assetManager = mRendererRuntime.getAssetManager();
			const VirtualFilename virtualFilename = assetManager.tryGetVirtualFilenameByAssetId(assetId);
			if (nullptr != virtualFilename)
			{
				if (assetId == materialBlueprintAssetId)
				{
					materialResource.setDebugName(IFileManager::INVALID_CHARACTER + std::string("[CreatedMaterial][InstanceOfMaterialBlueprintAsset=\"") + std::string(virtualFilename) + "\"]");
				}
				else
				{
					materialResource.setDebugName(IFileManager::INVALID_CHARACTER + std::string("[CreatedMaterial][Asset=\"") + std::string(virtualFilename) + "\"][MaterialBlueprintAsset=\"" + assetManager.getAssetByAssetId(materialBlueprintAssetId).virtualFilename + "\"]");
				}
			}
			else
			{
				materialResource.setDebugName(IFileManager::INVALID_CHARACTER + std::string("[CreatedMaterial][AssetId=") + std::to_string(assetId) + "][MaterialBlueprintAsset=\"" + assetManager.getAssetByAssetId(materialBlueprintAssetId).virtualFilename + "\"]");
			}
		}
		#endif

		{ // Setup material resource instance
			// Copy over the material properties of the material blueprint resource
			MaterialBlueprintResourceManager& materialBlueprintResourceManager = mRendererRuntime.getMaterialBlueprintResourceManager();
			MaterialBlueprintResourceId materialBlueprintResourceId = getInvalid<MaterialBlueprintResourceId>();
			materialBlueprintResourceManager.loadMaterialBlueprintResourceByAssetId(materialBlueprintAssetId, materialBlueprintResourceId);
			MaterialBlueprintResource* materialBlueprintResource = materialBlueprintResourceManager.tryGetById(materialBlueprintResourceId);
			if (nullptr != materialBlueprintResource)
			{
				// TODO(co) Possible optimization: Right now we don't filter for "RendererRuntime::MaterialProperty::Usage::GLOBAL_REFERENCE_FALLBACK" properties.
				//          Only the material blueprint resource needs to store such properties while they're useless inside material resources. The filtering
				//          makes the following more complex and it might not bring any real benefit. So, review this place in here later when we have more pressure on the system.
				materialResource.mMaterialProperties = materialBlueprintResource->mMaterialProperties;

				// Create default material technique
				materialResource.mSortedMaterialTechniqueVector.push_back(new MaterialTechnique(materialTechniqueId, materialResource, materialBlueprintResourceId));
			}
			else
			{
				// Error!
				RENDERER_ASSERT(mRendererRuntime.getContext(), false, "Invalid material blueprint resource")
			}
		}

		// Done
		setResourceLoadingState(materialResource, IResource::LoadingState::LOADED);
		return materialResource.getId();
	}

	MaterialResourceId MaterialResourceManager::createMaterialResourceByCloning(MaterialResourceId parentMaterialResourceId, AssetId assetId)
	{
		RENDERER_ASSERT(mRendererRuntime.getContext(), mInternalResourceManager->getResources().getElementById(parentMaterialResourceId).getLoadingState() == IResource::LoadingState::LOADED, "Invalid parent material resource ID")

		// Create the material resource instance
		MaterialResource& materialResource = mInternalResourceManager->getResources().addElement();
		materialResource.setResourceManager(this);
		materialResource.setAssetId(assetId);
		materialResource.setParentMaterialResourceId(parentMaterialResourceId);
		#ifdef _DEBUG
			materialResource.setDebugName(mInternalResourceManager->getResources().getElementById(parentMaterialResourceId).getDebugName() + "[Clone]");
		#endif

		// Done
		setResourceLoadingState(materialResource, IResource::LoadingState::LOADED);
		return materialResource.getId();
	}

	void MaterialResourceManager::destroyMaterialResource(MaterialResourceId materialResourceId)
	{
		mInternalResourceManager->getResources().removeElement(materialResourceId);
	}


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::IResourceManager methods ]
	//[-------------------------------------------------------]
	uint32_t MaterialResourceManager::getNumberOfResources() const
	{
		return mInternalResourceManager->getResources().getNumberOfElements();
	}

	IResource& MaterialResourceManager::getResourceByIndex(uint32_t index) const
	{
		return mInternalResourceManager->getResources().getElementByIndex(index);
	}

	IResource& MaterialResourceManager::getResourceByResourceId(ResourceId resourceId) const
	{
		return mInternalResourceManager->getResources().getElementById(resourceId);
	}

	IResource* MaterialResourceManager::tryGetResourceByResourceId(ResourceId resourceId) const
	{
		return mInternalResourceManager->getResources().tryGetElementById(resourceId);
	}

	void MaterialResourceManager::reloadResourceByAssetId(AssetId assetId)
	{
		return mInternalResourceManager->reloadResourceByAssetId(assetId);
	}


	//[-------------------------------------------------------]
	//[ Private virtual RendererRuntime::IResourceManager methods ]
	//[-------------------------------------------------------]
	IResourceLoader* MaterialResourceManager::createResourceLoaderInstance(ResourceLoaderTypeId resourceLoaderTypeId)
	{
		return mInternalResourceManager->createResourceLoaderInstance(resourceLoaderTypeId);
	}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	MaterialResourceManager::MaterialResourceManager(IRendererRuntime& rendererRuntime) :
		mRendererRuntime(rendererRuntime)
	{
		mInternalResourceManager = new ResourceManagerTemplate<MaterialResource, MaterialResourceLoader, MaterialResourceId, 4096>(rendererRuntime, *this);
	}

	MaterialResourceManager::~MaterialResourceManager()
	{
		delete mInternalResourceManager;
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

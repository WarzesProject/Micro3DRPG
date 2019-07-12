#include "stdafx.h"
#include "Resource/Material/Loader/MaterialResourceLoader.h"
#include "Resource/Material/Loader/MaterialFileFormat.h"
#include "Resource/Material/MaterialTechnique.h"
#include "Resource/Material/MaterialResource.h"
#include "Resource/MaterialBlueprint/MaterialBlueprintResourceManager.h"
#include "Resource/Texture/TextureResourceManager.h"
#include "IRendererRuntime.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::IResourceLoader methods ]
	//[-------------------------------------------------------]
	void MaterialResourceLoader::initialize(const Asset& asset, bool reload, IResource& resource)
	{
		IResourceLoader::initialize(asset, reload);
		mMaterialResource = static_cast<MaterialResource*>(&resource);
	}

	bool MaterialResourceLoader::onDeserialization(IFile& file)
	{
		// Tell the memory mapped file about the LZ4 compressed data
		return mMemoryFile.loadLz4CompressedDataFromFile(v1Material::FORMAT_TYPE, v1Material::FORMAT_VERSION, file);
	}

	void MaterialResourceLoader::onProcessing()
	{
		// Decompress LZ4 compressed data
		mMemoryFile.decompress();

		// Read in the material header
		v1Material::MaterialHeader materialHeader;
		mMemoryFile.read(&materialHeader, sizeof(v1Material::MaterialHeader));

		// Read techniques
		mNumberOfTechniques = materialHeader.numberOfTechniques;
		if (mNumberOfTechniques > 0)
		{
			// Allocate memory for the temporary data
			if (mMaximumNumberOfMaterialTechniques < mNumberOfTechniques)
			{
				delete [] mMaterialTechniques;
				mMaximumNumberOfMaterialTechniques = mNumberOfTechniques;
				mMaterialTechniques = new v1Material::Technique[mMaximumNumberOfMaterialTechniques];
			}

			// Read already sorted techniques
			mMemoryFile.read(mMaterialTechniques, sizeof(v1Material::Technique) * mNumberOfTechniques);
		}

		// Read properties
		// TODO(co) Get rid of the evil const-cast
		MaterialProperties::SortedPropertyVector& sortedPropertyVector = const_cast<MaterialProperties::SortedPropertyVector&>(mMaterialResource->mMaterialProperties.getSortedPropertyVector());
		if (materialHeader.numberOfProperties > 0)
		{
			sortedPropertyVector.resize(materialHeader.numberOfProperties);
			mMemoryFile.read(sortedPropertyVector.data(), sizeof(MaterialProperty) * materialHeader.numberOfProperties);
		}
		else
		{
			// Unusual border case but still valid to have a material which doesn't have any material properties
			sortedPropertyVector.clear();
		}
	}

	bool MaterialResourceLoader::onDispatch()
	{
		// TODO(co) Material resource update
		mMaterialResource->releaseTextures();
		mMaterialResource->destroyAllMaterialTechniques();

		{ // Request all referenced material blueprint resources to be loaded now
			MaterialBlueprintResourceManager& materialBlueprintResourceManager = mRendererRuntime.getMaterialBlueprintResourceManager();
			const v1Material::Technique* v1MaterialTechnique = mMaterialTechniques;
			mMaterialBlueprintResourceIds.resize(mNumberOfTechniques);
			for (size_t i = 0; i < mNumberOfTechniques; ++i, ++v1MaterialTechnique)
			{
				materialBlueprintResourceManager.loadMaterialBlueprintResourceByAssetId(v1MaterialTechnique->materialBlueprintAssetId, mMaterialBlueprintResourceIds[i]);
				RENDERER_ASSERT(mRendererRuntime.getContext(), isValid(mMaterialBlueprintResourceIds[i]), "Invalid material blueprint resource ID")
			}
		}

		// Fully loaded?
		return isFullyLoaded();
	}

	bool MaterialResourceLoader::isFullyLoaded()
	{
		{ // Fully loaded?
			const MaterialBlueprintResourceManager& materialBlueprintResourceManager = mRendererRuntime.getMaterialBlueprintResourceManager();
			const v1Material::Technique* v1MaterialTechnique = mMaterialTechniques;
			for (size_t i = 0; i < mNumberOfTechniques; ++i, ++v1MaterialTechnique)
			{
				if (IResource::LoadingState::LOADED != materialBlueprintResourceManager.getResourceByResourceId(mMaterialBlueprintResourceIds[i]).getLoadingState())
				{
					// Not fully loaded
					return false;
				}
			}
		}

		{ // Create the material techniques (list is already sorted)
			MaterialResource::SortedMaterialTechniqueVector& sortedMaterialTechniqueVector = mMaterialResource->mSortedMaterialTechniqueVector;
			const v1Material::Technique* v1MaterialTechnique = mMaterialTechniques;
			for (size_t i = 0; i < mNumberOfTechniques; ++i, ++v1MaterialTechnique)
			{
				sortedMaterialTechniqueVector.push_back(new MaterialTechnique(v1MaterialTechnique->materialTechniqueId, *mMaterialResource, mMaterialBlueprintResourceIds[i]));
			}
		}

		// Fully loaded
		return true;
	}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	MaterialResourceLoader::~MaterialResourceLoader()
	{
		// Free temporary data
		delete [] mMaterialTechniques;
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

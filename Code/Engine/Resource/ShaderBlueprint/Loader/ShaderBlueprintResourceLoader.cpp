#include "stdafx.h"
#include "Resource/ShaderBlueprint/Loader/ShaderBlueprintResourceLoader.h"
#include "Resource/ShaderBlueprint/Loader/ShaderBlueprintFileFormat.h"
#include "Resource/ShaderBlueprint/ShaderBlueprintResourceManager.h"
#include "Resource/ShaderBlueprint/ShaderBlueprintResource.h"
#include "Resource/ShaderPiece/ShaderPieceResourceManager.h"
#include "Resource/Material/MaterialResourceManager.h"
#include "Resource/MaterialBlueprint/MaterialBlueprintResourceManager.h"
#include "Resource/MaterialBlueprint/MaterialBlueprintResource.h"
#include "IRendererRuntime.h"

#include <unordered_set>


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::IResourceLoader methods ]
	//[-------------------------------------------------------]
	void ShaderBlueprintResourceLoader::initialize(const Asset& asset, bool reload, IResource& resource)
	{
		IResourceLoader::initialize(asset, reload);
		mShaderBlueprintResource = static_cast<ShaderBlueprintResource*>(&resource);
	}

	bool ShaderBlueprintResourceLoader::onDeserialization(IFile& file)
	{
		// Tell the memory mapped file about the LZ4 compressed data
		return mMemoryFile.loadLz4CompressedDataFromFile(v1ShaderBlueprint::FORMAT_TYPE, v1ShaderBlueprint::FORMAT_VERSION, file);
	}

	void ShaderBlueprintResourceLoader::onProcessing()
	{
		// Decompress LZ4 compressed data
		mMemoryFile.decompress();

		// Read in the shader blueprint header
		v1ShaderBlueprint::ShaderBlueprintHeader shaderBlueprintHeader;
		mMemoryFile.read(&shaderBlueprintHeader, sizeof(v1ShaderBlueprint::ShaderBlueprintHeader));

		// Sanity check
		RENDERER_ASSERT(mRendererRuntime.getContext(), shaderBlueprintHeader.numberOfShaderSourceCodeBytes > 0, "Invalid shader blueprint asset without any shader source code detected")

		// Read the asset IDs of the shader pieces to include
		if (shaderBlueprintHeader.numberOfIncludeShaderPieceAssetIds > 0)
		{
			// Allocate more temporary memory, if required
			if (mMaximumNumberOfIncludeShaderPieceAssetIds < shaderBlueprintHeader.numberOfIncludeShaderPieceAssetIds)
			{
				mMaximumNumberOfIncludeShaderPieceAssetIds = shaderBlueprintHeader.numberOfIncludeShaderPieceAssetIds;
				delete [] mIncludeShaderPieceAssetIds;
				mIncludeShaderPieceAssetIds = new AssetId[mMaximumNumberOfIncludeShaderPieceAssetIds];
			}

			// Read the asset IDs of the shader pieces to include
			mMemoryFile.read(mIncludeShaderPieceAssetIds, sizeof(AssetId) * shaderBlueprintHeader.numberOfIncludeShaderPieceAssetIds);
			mShaderBlueprintResource->mIncludeShaderPieceResourceIds.resize(shaderBlueprintHeader.numberOfIncludeShaderPieceAssetIds);
		}
		else
		{
			mShaderBlueprintResource->mIncludeShaderPieceResourceIds.clear();
		}

		{ // Read the referenced shader properties
			ShaderProperties::SortedPropertyVector& sortedPropertyVector = mShaderBlueprintResource->mReferencedShaderProperties.getSortedPropertyVector();
			if (shaderBlueprintHeader.numberOfReferencedShaderProperties > 0)
			{
				sortedPropertyVector.resize(shaderBlueprintHeader.numberOfReferencedShaderProperties);
				mMemoryFile.read(sortedPropertyVector.data(), sizeof(ShaderProperties::Property) * shaderBlueprintHeader.numberOfReferencedShaderProperties);
			}
			else
			{
				sortedPropertyVector.clear();
			}
		}

		{ // Read the shader blueprint ASCII source code
			// Allocate more temporary memory, if required
			if (mMaximumNumberOfShaderSourceCodeBytes < shaderBlueprintHeader.numberOfShaderSourceCodeBytes)
			{
				mMaximumNumberOfShaderSourceCodeBytes = shaderBlueprintHeader.numberOfShaderSourceCodeBytes;
				delete [] mShaderSourceCode;
				mShaderSourceCode = new char[mMaximumNumberOfShaderSourceCodeBytes];
			}

			// Read the shader blueprint ASCII source code
			mMemoryFile.read(mShaderSourceCode, shaderBlueprintHeader.numberOfShaderSourceCodeBytes);
			mShaderBlueprintResource->mShaderSourceCode.assign(mShaderSourceCode, mShaderSourceCode + shaderBlueprintHeader.numberOfShaderSourceCodeBytes);
		}
	}

	bool ShaderBlueprintResourceLoader::onDispatch()
	{
		{ // Read the shader piece resources to include
			ShaderPieceResourceManager& shaderPieceResourceManager = mRendererRuntime.getShaderPieceResourceManager();
			ShaderBlueprintResource::IncludeShaderPieceResourceIds& includeShaderPieceResourceIds = mShaderBlueprintResource->mIncludeShaderPieceResourceIds;
			const size_t numberOfShaderPieceResources = includeShaderPieceResourceIds.size();
			const AssetId* includeShaderPieceAssetIds = mIncludeShaderPieceAssetIds;
			for (size_t i = 0; i < numberOfShaderPieceResources; ++i, ++includeShaderPieceAssetIds)
			{
				shaderPieceResourceManager.loadShaderPieceResourceByAssetId(*includeShaderPieceAssetIds, includeShaderPieceResourceIds[i]);
			}
		}

		// TODO(co) Cleanup: Get all influenced material blueprint resources
		if (getReload())
		{
			const ShaderBlueprintResourceId shaderBlueprintResourceId = mShaderBlueprintResource->getId();
			typedef std::unordered_set<MaterialBlueprintResource*> MaterialBlueprintResourcePointers;
			MaterialBlueprintResourcePointers materialBlueprintResourcePointers;
			const MaterialBlueprintResourceManager& materialBlueprintResourceManager = mRendererRuntime.getMaterialBlueprintResourceManager();
			const uint32_t numberOfElements = materialBlueprintResourceManager.getNumberOfResources();
			for (uint32_t i = 0; i < numberOfElements; ++i)
			{
				MaterialBlueprintResource& materialBlueprintResource = materialBlueprintResourceManager.getByIndex(i);
				const ShaderBlueprintResourceId computeShaderBlueprintResourceId = materialBlueprintResource.getComputeShaderBlueprintResourceId();
				if (isValid(computeShaderBlueprintResourceId))
				{
					// Compute pipeline state object (PSO)
					if (shaderBlueprintResourceId == computeShaderBlueprintResourceId)
					{
						materialBlueprintResourcePointers.insert(&materialBlueprintResource);
					}
				}
				else
				{
					// Graphics pipeline state object (PSO)
					for (uint8_t graphicsShaderType = 0; graphicsShaderType < NUMBER_OF_GRAPHICS_SHADER_TYPES; ++graphicsShaderType)
					{
						if (materialBlueprintResource.getGraphicsShaderBlueprintResourceId(static_cast<GraphicsShaderType>(graphicsShaderType)) == shaderBlueprintResourceId)
						{
							materialBlueprintResourcePointers.insert(&materialBlueprintResource);
							break;
						}
					}
				}
			}
			for (MaterialBlueprintResource* materialBlueprintResource : materialBlueprintResourcePointers)
			{
				materialBlueprintResource->getGraphicsPipelineStateCacheManager().clearCache();
				materialBlueprintResource->getGraphicsPipelineStateCacheManager().getGraphicsProgramCacheManager().clearCache();
				materialBlueprintResource->getComputePipelineStateCacheManager().clearCache();
				++const_cast<MaterialProperties&>(materialBlueprintResource->getMaterialProperties()).mShaderCombinationGenerationCounter;
			}

			// TODO(co) Do only clear the influenced shader cache entries
			mRendererRuntime.getShaderBlueprintResourceManager().getShaderCacheManager().clearCache();
		}

		// Fully loaded?
		return isFullyLoaded();
	}

	bool ShaderBlueprintResourceLoader::isFullyLoaded()
	{
		const ShaderPieceResourceManager& shaderPieceResourceManager = mRendererRuntime.getShaderPieceResourceManager();
		for (ShaderPieceResourceId shaderPieceResourceId : mShaderBlueprintResource->mIncludeShaderPieceResourceIds)
		{
			if (isValid(shaderPieceResourceId) && IResource::LoadingState::LOADED != shaderPieceResourceManager.getResourceByResourceId(shaderPieceResourceId).getLoadingState())
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

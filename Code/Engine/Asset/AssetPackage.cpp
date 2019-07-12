#include "stdafx.h"
#include "Asset/AssetPackage.h"
#include "Core/Math/Math.h"
#include "Context.h"
#include <algorithm>


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
namespace
{
	namespace detail
	{


		//[-------------------------------------------------------]
		//[ Structures                                            ]
		//[-------------------------------------------------------]
		struct OrderByAssetId final
		{
			[[nodiscard]] inline bool operator()(const RendererRuntime::Asset& left, RendererRuntime::AssetId right) const
			{
				return (left.assetId < right);
			}

			[[nodiscard]] inline bool operator()(RendererRuntime::AssetId left, const RendererRuntime::Asset& right) const
			{
				return (left < right.assetId);
			}
		};


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
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	void AssetPackage::addAsset([[maybe_unused]] const Context& context, AssetId assetId, VirtualFilename virtualFilename)
	{
		RENDERER_ASSERT(context, nullptr == tryGetAssetByAssetId(assetId), "Renderer runtime asset ID is already used")
		RENDERER_ASSERT(context, strlen(virtualFilename) < Asset::MAXIMUM_ASSET_FILENAME_LENGTH, "The renderer runtime asset filename is too long")
		SortedAssetVector::const_iterator iterator = std::lower_bound(mSortedAssetVector.cbegin(), mSortedAssetVector.cend(), assetId, ::detail::OrderByAssetId());
		Asset& asset = *mSortedAssetVector.insert(iterator, Asset());
		asset.assetId = assetId;
		strncpy(asset.virtualFilename, virtualFilename, Asset::MAXIMUM_ASSET_FILENAME_LENGTH - 1);	// -1 not including the terminating zero
	}

	const Asset* AssetPackage::tryGetAssetByAssetId(AssetId assetId) const
	{
		SortedAssetVector::const_iterator iterator = std::lower_bound(mSortedAssetVector.cbegin(), mSortedAssetVector.cend(), assetId, ::detail::OrderByAssetId());
		return (iterator != mSortedAssetVector.cend() && iterator->assetId == assetId) ? &(*iterator) : nullptr;
	}

	bool AssetPackage::validateIntegrity(const IFileManager& fileManager) const
	{
		for (const Asset& asset : mSortedAssetVector)
		{
			if (Math::calculateFileFNV1a64ByVirtualFilename(fileManager, asset.virtualFilename) != asset.fileHash)
			{
				// Invalid integrity
				return false;
			}
		}

		// Valid integrity
		return true;
	}

	Asset* AssetPackage::tryGetWritableAssetByAssetId(AssetId assetId)
	{
		SortedAssetVector::iterator iterator = std::lower_bound(mSortedAssetVector.begin(), mSortedAssetVector.end(), assetId, ::detail::OrderByAssetId());
		return (iterator != mSortedAssetVector.cend() && iterator->assetId == assetId) ? &(*iterator) : nullptr;
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

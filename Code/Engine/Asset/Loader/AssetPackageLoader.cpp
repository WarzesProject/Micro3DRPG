#include "stdafx.h"
#include "Asset/Loader/AssetPackageLoader.h"
#include "Asset/Loader/AssetPackageFileFormat.h"
#include "Asset/AssetPackage.h"
#include "Core/File/MemoryFile.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	void AssetPackageLoader::loadAssetPackage(AssetPackage& assetPackage, IFile& file)
	{
		// Tell the memory mapped file about the LZ4 compressed data and decompress it at once
		MemoryFile memoryFile;
		if (memoryFile.loadLz4CompressedDataFromFile(v1AssetPackage::FORMAT_TYPE, v1AssetPackage::FORMAT_VERSION, file))
		{
			memoryFile.decompress();

			// Read in the asset package header
			v1AssetPackage::AssetPackageHeader assetPackageHeader;
			memoryFile.read(&assetPackageHeader, sizeof(v1AssetPackage::AssetPackageHeader));

			// Sanity check
			assert((assetPackageHeader.numberOfAssets > 0) && "Invalid empty asset package detected");

			// Read in the asset package content in one single burst
			AssetPackage::SortedAssetVector& sortedAssetVector = assetPackage.getWritableSortedAssetVector();
			sortedAssetVector.resize(assetPackageHeader.numberOfAssets);
			memoryFile.read(sortedAssetVector.data(), sizeof(Asset) * assetPackageHeader.numberOfAssets);
		}
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

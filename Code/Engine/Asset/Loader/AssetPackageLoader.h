#pragma once

#include "Core/Loader.h"


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class IFile;
	class AssetPackage;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	class AssetPackageLoader final : protected Loader
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class AssetManager;


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		inline AssetPackageLoader()
		{
			// Nothing here
		}

		inline ~AssetPackageLoader()
		{
			// Nothing here
		}

		explicit AssetPackageLoader(const AssetPackageLoader&) = delete;
		AssetPackageLoader& operator=(const AssetPackageLoader&) = delete;
		void loadAssetPackage(AssetPackage& assetPackage, IFile& file);


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

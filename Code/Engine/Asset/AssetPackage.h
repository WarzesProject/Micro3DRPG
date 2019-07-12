#pragma once

#include "Export.h"
#include "Asset/Asset.h"
#include "Core/GetInvalid.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4365)	// warning C4365: 'argument': conversion from 'long' to 'unsigned int', signed/unsigned mismatch
	PRAGMA_WARNING_DISABLE_MSVC(4571)	// warning C4571: Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
	PRAGMA_WARNING_DISABLE_MSVC(4668)	// warning C4668: '_M_HYBRID_X86_ARM64' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
	#include <vector>
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class Context;
	class IFileManager;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef StringId AssetPackageId;		///< Resource loader type identifier, internally just a POD "uint32_t", string ID scheme is "<project name>/<asset package name>"
	typedef const char* VirtualFilename;	///< UTF-8 virtual filename, the virtual filename scheme is "<mount point = project name>/<asset directory>/<asset name>.<file extension>" (example "Example/Mesh/Monster/Squirrel.mesh"), never ever a null pointer and always finished by a terminating zero


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	class AssetPackage final
	{


	//[-------------------------------------------------------]
	//[ Public definitions                                    ]
	//[-------------------------------------------------------]
	public:
		typedef std::vector<Asset> SortedAssetVector;


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class AssetManager;


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		inline AssetPackage() :
			mAssetPackageId(getInvalid<AssetPackageId>())
		{
			// Nothing here
		}

		inline explicit AssetPackage(AssetPackageId assetPackageId) :
			mAssetPackageId(assetPackageId)
		{
			// Nothing here
		}

		inline ~AssetPackage()
		{
			// Nothing here
		}

		[[nodiscard]] inline AssetPackageId getAssetPackageId() const
		{
			return mAssetPackageId;
		}

		inline void clear()
		{
			mSortedAssetVector.clear();
		}

		[[nodiscard]] inline const SortedAssetVector& getSortedAssetVector() const
		{
			return mSortedAssetVector;
		}

		RENDERERRUNTIME_API_EXPORT void addAsset(const Context& context, AssetId assetId, VirtualFilename virtualFilename);
		[[nodiscard]] RENDERERRUNTIME_API_EXPORT const Asset* tryGetAssetByAssetId(AssetId assetId) const;

		[[nodiscard]] inline VirtualFilename tryGetVirtualFilenameByAssetId(AssetId assetId) const
		{
			const Asset* asset = tryGetAssetByAssetId(assetId);
			return (nullptr != asset) ? asset->virtualFilename : nullptr;
		}

		[[nodiscard]] RENDERERRUNTIME_API_EXPORT bool validateIntegrity(const IFileManager& fileManager) const;

		// For internal use only (exposed for API performance reasons)
		[[nodiscard]] inline SortedAssetVector& getWritableSortedAssetVector()
		{
			return mSortedAssetVector;
		}
		[[nodiscard]] RENDERERRUNTIME_API_EXPORT Asset* tryGetWritableAssetByAssetId(AssetId assetId);


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		explicit AssetPackage(const AssetPackage&) = delete;
		AssetPackage& operator=(const AssetPackage&) = delete;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		AssetPackageId	  mAssetPackageId;
		SortedAssetVector mSortedAssetVector;	///< Sorted vector of assets


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

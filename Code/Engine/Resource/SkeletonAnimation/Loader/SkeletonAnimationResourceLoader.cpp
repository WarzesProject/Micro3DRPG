#include "stdafx.h"
#include "Resource/SkeletonAnimation/Loader/SkeletonAnimationResourceLoader.h"
#include "Resource/SkeletonAnimation/Loader/SkeletonAnimationFileFormat.h"
#include "Resource/SkeletonAnimation/SkeletonAnimationResource.h"
#include "Core/File/IFile.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::IResourceLoader methods ]
	//[-------------------------------------------------------]
	void SkeletonAnimationResourceLoader::initialize(const Asset& asset, bool reload, IResource& resource)
	{
		IResourceLoader::initialize(asset, reload);
		mSkeletonAnimationResource = static_cast<SkeletonAnimationResource*>(&resource);
	}

	bool SkeletonAnimationResourceLoader::onDeserialization(IFile& file)
	{
		// Read in the file format header
		struct FileFormatHeader final
		{
			uint32_t formatType;
			uint32_t formatVersion;
		};
		FileFormatHeader fileFormatHeader;
		file.read(&fileFormatHeader, sizeof(FileFormatHeader));
		if (v1SkeletonAnimation::FORMAT_TYPE == fileFormatHeader.formatType && v1SkeletonAnimation::FORMAT_VERSION == fileFormatHeader.formatVersion)
		{
			// Read in the skeleton animation header
			v1SkeletonAnimation::SkeletonAnimationHeader skeletonAnimationHeader;
			file.read(&skeletonAnimationHeader, sizeof(v1SkeletonAnimation::SkeletonAnimationHeader));
			mSkeletonAnimationResource->mNumberOfChannels = skeletonAnimationHeader.numberOfChannels;
			mSkeletonAnimationResource->mDurationInTicks  = skeletonAnimationHeader.durationInTicks;
			mSkeletonAnimationResource->mTicksPerSecond   = skeletonAnimationHeader.ticksPerSecond;

			// Sanity checks
			RENDERER_ASSERT(mRendererRuntime.getContext(), skeletonAnimationHeader.numberOfChannels > 0, "Invalid skeleton animation asset with zero channels detected")
			RENDERER_ASSERT(mRendererRuntime.getContext(), skeletonAnimationHeader.aclCompressedClipSize > 0, "Invalid skeleton animation asset with zero ACL compressed clip size detected")

			// Read in bone IDs
			mSkeletonAnimationResource->mBoneIds.resize(skeletonAnimationHeader.numberOfChannels);
			file.read(mSkeletonAnimationResource->mBoneIds.data(), sizeof(uint32_t) * skeletonAnimationHeader.numberOfChannels);

			// Read in the ACL ( https://github.com/nfrechette/acl ) compressed skeleton animation clip
			mSkeletonAnimationResource->mAclCompressedClip.resize(skeletonAnimationHeader.aclCompressedClipSize);
			file.read(mSkeletonAnimationResource->mAclCompressedClip.data(), skeletonAnimationHeader.aclCompressedClipSize);

			// Done
			return true;
		}
		else
		{
			// Error!
			return false;
		}
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

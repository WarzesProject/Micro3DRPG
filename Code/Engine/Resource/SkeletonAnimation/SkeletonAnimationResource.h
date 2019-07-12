#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Resource/IResource.h"


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	template <class ELEMENT_TYPE, typename ID_TYPE, uint32_t MAXIMUM_NUMBER_OF_ELEMENTS> class PackedElementManager;
	template <class TYPE, class LOADER_TYPE, typename ID_TYPE, uint32_t MAXIMUM_NUMBER_OF_ELEMENTS> class ResourceManagerTemplate;
	class SkeletonAnimationResourceLoader;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef uint32_t SkeletonAnimationResourceId;	///< POD skeleton animation resource identifier


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	/**
	*  @brief
	*    Rigid skeleton animation clip resource
	*/
	class SkeletonAnimationResource final : public IResource
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class SkeletonAnimationResourceLoader;
		friend PackedElementManager<SkeletonAnimationResource, SkeletonAnimationResourceId, 2048>;										// Type definition of template class
		friend ResourceManagerTemplate<SkeletonAnimationResource, SkeletonAnimationResourceLoader, SkeletonAnimationResourceId, 2048>;	// Type definition of template class


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		[[nodiscard]] inline uint8_t getNumberOfChannels() const
		{
			return mNumberOfChannels;
		}

		[[nodiscard]] inline float getDurationInTicks() const
		{
			return mDurationInTicks;
		}

		[[nodiscard]] inline float getTicksPerSecond() const
		{
			return mTicksPerSecond;
		}

		[[nodiscard]] inline const std::vector<uint32_t>& getBoneIds() const
		{
			return mBoneIds;
		}

		[[nodiscard]] inline const std::vector<uint8_t>& getAclCompressedClip() const
		{
			return mAclCompressedClip;
		}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		inline SkeletonAnimationResource() :
			mNumberOfChannels(0),
			mDurationInTicks(0.0f),
			mTicksPerSecond(0.0f)
		{
			// Nothing here
		}

		inline virtual ~SkeletonAnimationResource() override
		{
			// Sanity checks
			assert(0 == mNumberOfChannels);
			assert(0.0f == mDurationInTicks);
			assert(0.0f == mTicksPerSecond);
		}

		explicit SkeletonAnimationResource(const SkeletonAnimationResource&) = delete;
		SkeletonAnimationResource& operator=(const SkeletonAnimationResource&) = delete;

		inline void clearSkeletonAnimationData()
		{
			mNumberOfChannels = 0;
			mDurationInTicks  = 0.0f;
			mTicksPerSecond   = 0.0f;
		}

		//[-------------------------------------------------------]
		//[ "RendererRuntime::PackedElementManager" management    ]
		//[-------------------------------------------------------]
		inline void initializeElement(SkeletonAnimationResourceId skeletonAnimationResourceId)
		{
			// Sanity checks
			assert(0 == mNumberOfChannels);
			assert(0.0f == mDurationInTicks);
			assert(0.0f == mTicksPerSecond);

			// Call base implementation
			IResource::initializeElement(skeletonAnimationResourceId);
		}

		inline void deinitializeElement()
		{
			// Reset everything
			clearSkeletonAnimationData();

			// Call base implementation
			IResource::deinitializeElement();
		}


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		uint8_t				  mNumberOfChannels;	///< The number of bone animation channels; each channel affects a single bone
		float				  mDurationInTicks;		///< Duration of the animation in ticks
		float				  mTicksPerSecond;		///< Ticks per second; 0 if not specified in the imported file
		std::vector<uint32_t> mBoneIds;
		std::vector<uint8_t>  mAclCompressedClip;	///< ACL ( https://github.com/nfrechette/acl ) compressed skeleton animation clip


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

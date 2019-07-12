#include "stdafx.h"
#include "Resource/SkeletonAnimation/SkeletonAnimationEvaluator.h"
#include "Resource/SkeletonAnimation/SkeletonAnimationResourceManager.h"
#include "Resource/SkeletonAnimation/SkeletonAnimationResource.h"
#include "Core/Math/Math.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4365)	// warning C4365: 'initializing': conversion from 'int' to 'uint8_t', signed/unsigned mismatch
	#include <acl/algorithm/uniformly_sampled/decoder.h>
PRAGMA_WARNING_POP

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4464)	// warning C4464: relative include path contains '..'
	#include <glm/gtx/quaternion.hpp>
	#include <glm/gtc/matrix_transform.hpp>
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
namespace
{
	namespace detail
	{


		//[-------------------------------------------------------]
		//[ Global definitions                                    ]
		//[-------------------------------------------------------]
		typedef acl::uniformly_sampled::DecompressionContext<acl::uniformly_sampled::DefaultDecompressionSettings> AclDecompressionContext;


		//[-------------------------------------------------------]
		//[ Classes                                               ]
		//[-------------------------------------------------------]
		class AclAllocator final : public acl::IAllocator
		{


		//[-------------------------------------------------------]
		//[ Public methods                                        ]
		//[-------------------------------------------------------]
		public:
			inline explicit AclAllocator(Renderer::IAllocator& allocator) :
				mAllocator(allocator)
			{
				// Nothing here
			}

			explicit AclAllocator(const AclAllocator&) = delete;

			virtual ~AclAllocator() override
			{
				// Nothing here
			}

			AclAllocator& operator=(const AclAllocator&) = delete;


		//[-------------------------------------------------------]
		//[ Public virtual acl::IAllocator methods                ]
		//[-------------------------------------------------------]
		public:
			virtual void* allocate(size_t size, size_t alignment = k_default_alignment) override
			{
				return mAllocator.reallocate(nullptr, 0, size, alignment);
			}

			virtual void deallocate(void* ptr, size_t size) override
			{
				mAllocator.reallocate(ptr, size, 0, 1);
			}


		//[-------------------------------------------------------]
		//[ Private data                                          ]
		//[-------------------------------------------------------]
		private:
			Renderer::IAllocator& mAllocator;


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
	SkeletonAnimationEvaluator::SkeletonAnimationEvaluator(Renderer::IAllocator& allocator, SkeletonAnimationResourceManager& skeletonAnimationResourceManager, SkeletonAnimationResourceId skeletonAnimationResourceId) :
		mSkeletonAnimationResourceManager(skeletonAnimationResourceManager),
		mSkeletonAnimationResourceId(skeletonAnimationResourceId)
	{
		mAclAllocator = new ::detail::AclAllocator(allocator);
		mAclDecompressionContext = new ::detail::AclDecompressionContext();
//		mAclDecompressionContext = new ::detail::AclDecompressionContext(static_cast<acl::IAllocator*>(mAclAllocator));	// TODO(co) When using ACL decompression context with custom allocator there are reports about damaged blocks on destruction
		const SkeletonAnimationResource& skeletonAnimationResource = mSkeletonAnimationResourceManager.getById(mSkeletonAnimationResourceId);
		static_cast<::detail::AclDecompressionContext*>(mAclDecompressionContext)->initialize(*reinterpret_cast<const acl::CompressedClip*>(skeletonAnimationResource.getAclCompressedClip().data()));
		mBoneIds = skeletonAnimationResource.getBoneIds();
		mTransformMatrices.resize(skeletonAnimationResource.getNumberOfChannels());
	}

	SkeletonAnimationEvaluator::~SkeletonAnimationEvaluator()
	{
		delete static_cast<::detail::AclDecompressionContext*>(mAclDecompressionContext);
		delete static_cast<::detail::AclAllocator*>(mAclAllocator);
	}

	void SkeletonAnimationEvaluator::evaluate(float timeInSeconds)
	{
		const SkeletonAnimationResource& skeletonAnimationResource = mSkeletonAnimationResourceManager.getById(mSkeletonAnimationResourceId);
		const uint8_t numberOfChannels = skeletonAnimationResource.getNumberOfChannels();

		// Decompress the ACL compressed skeleton animation clip
		::detail::AclDecompressionContext* aclDecompressionContext = static_cast<::detail::AclDecompressionContext*>(mAclDecompressionContext);
		const float duration = skeletonAnimationResource.getDurationInTicks() / skeletonAnimationResource.getTicksPerSecond();
		while (timeInSeconds > duration)
		{
			timeInSeconds -= duration;
		}
		static_cast<::detail::AclDecompressionContext*>(mAclDecompressionContext)->seek(timeInSeconds, acl::SampleRoundingPolicy::None);
		for (uint8_t i = 0; i < numberOfChannels; ++i)
		{
			acl::Quat_32 rotation;
			acl::Vector4_32 translation;
			acl::Vector4_32 scale;
			aclDecompressionContext->decompress_bone(i, &rotation, &translation, &scale);

			// Build a transformation matrix from it
			// TODO(co) Handle case of no scale
			// TODO(co) Review temporary matrix instances on the C-runtime stack
			glm::quat presentRotation(acl::quat_get_w(rotation), acl::quat_get_x(rotation), acl::quat_get_y(rotation), acl::quat_get_z(rotation));
			glm::vec3 presentPosition(acl::vector_get_x(translation), acl::vector_get_y(translation), acl::vector_get_z(translation));
			glm::vec3 presentScale(acl::vector_get_x(scale), acl::vector_get_y(scale), acl::vector_get_z(scale));
			mTransformMatrices[i] = glm::translate(Math::MAT4_IDENTITY, presentPosition) * glm::toMat4(presentRotation) * glm::scale(Math::MAT4_IDENTITY, presentScale);
		}
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

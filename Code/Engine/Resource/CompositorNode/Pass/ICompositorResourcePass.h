#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Core/StringId.h"
#include "Core/GetInvalid.h"


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class CompositorTarget;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef StringId CompositorPassTypeId;	///< Compositor pass type identifier, internally just a POD "uint32_t"


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	class ICompositorResourcePass
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class CompositorTarget;	// Needs to destroy compositor resource pass instances


	//[-------------------------------------------------------]
	//[ Public definitions                                    ]
	//[-------------------------------------------------------]
	public:
		static constexpr uint32_t MAXIMUM_PASS_NAME_LENGTH = 63 + 1;	// +1 for the terminating zero


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		[[nodiscard]] inline const CompositorTarget& getCompositorTarget() const
		{
			return mCompositorTarget;
		}

		#if defined(_DEBUG) || defined(RENDERER_RUNTIME_PROFILER)
			[[nodiscard]] inline const char* getDebugName() const
			{
				return mDebugName;
			}

			inline void setDebugName(const char* debugName)
			{
				strncpy(mDebugName, debugName, MAXIMUM_PASS_NAME_LENGTH);
			}
		#endif

		[[nodiscard]] inline float getMinimumDepth() const
		{
			return mMinimumDepth;
		}

		[[nodiscard]] inline float getMaximumDepth() const
		{
			return mMaximumDepth;
		}

		[[nodiscard]] inline bool getSkipFirstExecution() const
		{
			return mSkipFirstExecution;
		}

		[[nodiscard]] inline uint32_t getNumberOfExecutions() const
		{
			return mNumberOfExecutions;
		}


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::ICompositorResourcePass methods ]
	//[-------------------------------------------------------]
	public:
		[[nodiscard]] virtual CompositorPassTypeId getTypeId() const = 0;

		inline virtual void deserialize([[maybe_unused]] uint32_t numberOfBytes, const uint8_t* data)
		{
			// Keep this in sync with "RendererRuntime::v1CompositorNode::Pass"
			// -> Don't include "RendererRuntime/Resource/CompositorNode/Loader/CompositorNodeFileFormat.h" here to keep the header complexity low (compile times matter)
			#pragma pack(push)
			#pragma pack(1)
				struct PassData final
				{
					char	 name[MAXIMUM_PASS_NAME_LENGTH] = { "Compositor pass" };	///< Human readable ASCII pass name for debugging and profiling, contains terminating zero
					float	 minimumDepth					= 0.0f;
					float	 maximumDepth					= 1.0f;
					uint32_t numberOfExecutions				= RendererRuntime::getInvalid<uint32_t>();
					bool	 skipFirstExecution				= false;
				};
			#pragma pack(pop)

			// Sanity check
			assert(sizeof(PassData) == numberOfBytes);

			// Read data
			const PassData* pass = reinterpret_cast<const PassData*>(data);
			#if defined(_DEBUG) || defined(RENDERER_RUNTIME_PROFILER)
				strncpy(mDebugName, pass->name, MAXIMUM_PASS_NAME_LENGTH);
			#endif
			mMinimumDepth		= pass->minimumDepth;
			mMaximumDepth		= pass->maximumDepth;
			mNumberOfExecutions = pass->numberOfExecutions;
			mSkipFirstExecution = pass->skipFirstExecution;

			// Sanity checks
			assert(mNumberOfExecutions > 0);
			assert(!mSkipFirstExecution || mNumberOfExecutions > 1);
		}

		/**
		*  @brief
		*    Return the render queue index range
		*
		*   @param[in] minimumRenderQueueIndex
		*     If this compositor resource pass has a render queue range defined, this will receive the minimum render queue index (inclusive), if there's no range no value will be written at all
		*   @param[in] maximumRenderQueueIndex
		*     If this compositor resource pass has a render queue range defined, this will receive the maximum render queue index (inclusive), if there's no range no value will be written at all
		*
		*  @return
		*    "true" if this compositor resource pass has a render queue range defined, else "false"
		*/
		[[nodiscard]] inline virtual bool getRenderQueueIndexRange([[maybe_unused]] uint8_t& minimumRenderQueueIndex, [[maybe_unused]] uint8_t& maximumRenderQueueIndex) const
		{
			// This compositor resource pass has no render queue range defined
			return false;
		}


	//[-------------------------------------------------------]
	//[ Protected methods                                     ]
	//[-------------------------------------------------------]
	protected:
		inline explicit ICompositorResourcePass(const CompositorTarget& compositorTarget) :
			mCompositorTarget(compositorTarget),
			#if defined(_DEBUG) || defined(RENDERER_RUNTIME_PROFILER)
				mDebugName{ "Compositor pass" },
			#endif
			mMinimumDepth(0.0f),
			mMaximumDepth(1.0f),
			mSkipFirstExecution(false),
			mNumberOfExecutions(RendererRuntime::getInvalid<uint32_t>())
		{
			// Nothing here
		}

		inline virtual ~ICompositorResourcePass()
		{
			// Nothing here
		}

		explicit ICompositorResourcePass(const ICompositorResourcePass&) = delete;
		ICompositorResourcePass& operator=(const ICompositorResourcePass&) = delete;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		const CompositorTarget& mCompositorTarget;
		#if defined(_DEBUG) || defined(RENDERER_RUNTIME_PROFILER)
			char				mDebugName[MAXIMUM_PASS_NAME_LENGTH];	///< Human readable ASCII pass name for debugging and profiling, contains terminating zero
		#endif
		float					mMinimumDepth;
		float					mMaximumDepth;
		bool					mSkipFirstExecution;
		uint32_t				mNumberOfExecutions;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

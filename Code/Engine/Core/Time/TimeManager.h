#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Core/Manager.h"
#include "Core/Time/Stopwatch.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4668)	// warning C4668: '_M_HYBRID_X86_ARM64' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
	#include <limits>		// For "std::numeric_limits()"
	#include <inttypes.h>	// For uint32_t, uint64_t etc.
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	/**
	*  @brief
	*    Time manager
	*/
	class TimeManager final : public Manager
	{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		inline TimeManager() :
			mPastSecondsSinceLastFrame(std::numeric_limits<float>::min()),	// Don't initialize with zero or time advancing enforcement asserts will get more complicated
			mGlobalTimeInSeconds(0.0f),
			mNumberOfRenderedFrames(0)
		{
			// Nothing here
		}

		inline ~TimeManager()
		{
			// Nothing here
		}

		[[nodiscard]] inline float getPastSecondsSinceLastFrame() const
		{
			return mPastSecondsSinceLastFrame;
		}

		[[nodiscard]] inline float getGlobalTimeInSeconds() const
		{
			return mGlobalTimeInSeconds;
		}

		[[nodiscard]] inline uint64_t getNumberOfRenderedFrames() const
		{
			return mNumberOfRenderedFrames;
		}

		/**
		*  @brief
		*    Time manager update
		*
		*  @note
		*    - Call this once per frame
		*/
		void update();


	//[-------------------------------------------------------]
	//[ Protected methods                                     ]
	//[-------------------------------------------------------]
	protected:
		explicit TimeManager(const TimeManager&) = delete;
		TimeManager& operator=(const TimeManager&) = delete;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		Stopwatch mStopwatch;
		float	  mPastSecondsSinceLastFrame;
		float	  mGlobalTimeInSeconds;
		uint64_t  mNumberOfRenderedFrames;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

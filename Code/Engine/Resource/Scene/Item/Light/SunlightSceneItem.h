#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Resource/Scene/Item/Light/LightSceneItem.h"


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
	*    Sunlight scene item
	*
	*  @note
	*    - Automatically controls light parameters as well as the owner scene node rotation via time-of-day
	*/
	class SunlightSceneItem final : public LightSceneItem
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class SceneFactory;	// Needs to be able to create scene item instances


	//[-------------------------------------------------------]
	//[ Public definitions                                    ]
	//[-------------------------------------------------------]
	public:
		static constexpr uint32_t TYPE_ID = STRING_ID("SunlightSceneItem");


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		//[-------------------------------------------------------]
		//[ Usually fixed                                         ]
		//[-------------------------------------------------------]
		[[nodiscard]] inline float getSunriseTime() const
		{
			return mSunriseTime;
		}

		inline void setSunriseTime(float sunriseTime)
		{
			if (mSunriseTime != sunriseTime)
			{
				// Sanity check (unit in O'clock)
				RENDERER_ASSERT(getContext(), sunriseTime >= 00.00f && sunriseTime < 24.00f, "Invalid data")

				// Set new value and calculate derived sunlight properties
				mSunriseTime = sunriseTime;
				calculatedDerivedSunlightProperties();
			}
		}

		[[nodiscard]] inline float getSunsetTime() const
		{
			return mSunsetTime;
		}

		inline void setSunsetTime(float sunsetTime)
		{
			if (mSunsetTime != sunsetTime)
			{
				// Sanity check (unit in O'clock)
				RENDERER_ASSERT(getContext(), sunsetTime >= 00.00f && sunsetTime < 24.00f, "Invalid data")

				// Set new value and calculate derived sunlight properties
				mSunsetTime = sunsetTime;
				calculatedDerivedSunlightProperties();
			}
		}

		[[nodiscard]] inline float getEastDirection() const
		{
			return mEastDirection;
		}

		inline void setEastDirection(float eastDirection)
		{
			if (mEastDirection != eastDirection)
			{
				mEastDirection = eastDirection;
				calculatedDerivedSunlightProperties();
			}
		}

		[[nodiscard]] inline float getAngleOfIncidence() const
		{
			return mAngleOfIncidence;
		}

		inline void setAngleOfIncidence(float angleOfIncidence)
		{
			if (mAngleOfIncidence != angleOfIncidence)
			{
				mAngleOfIncidence = angleOfIncidence;
				calculatedDerivedSunlightProperties();
			}
		}

		//[-------------------------------------------------------]
		//[ Usually animated                                      ]
		//[-------------------------------------------------------]
		[[nodiscard]] inline float getTimeOfDay() const
		{
			return mTimeOfDay;
		}

		inline void setTimeOfDay(float timeOfDay)
		{
			if (mTimeOfDay != timeOfDay)
			{
				// Sanity check (unit in O'clock)
				RENDERER_ASSERT(getContext(), timeOfDay >= 00.00f && timeOfDay < 24.00f, "Invalid data")

				// Set new value and calculate derived sunlight properties
				mTimeOfDay = timeOfDay;
				calculatedDerivedSunlightProperties();
			}
		}


	//[-------------------------------------------------------]
	//[ Public RendererRuntime::ISceneItem methods            ]
	//[-------------------------------------------------------]
	public:
		[[nodiscard]] inline virtual SceneItemTypeId getSceneItemTypeId() const override
		{
			return TYPE_ID;
		}

		virtual void deserialize(uint32_t numberOfBytes, const uint8_t* data) override;


	//[-------------------------------------------------------]
	//[ Public RendererRuntime::ISceneItem methods            ]
	//[-------------------------------------------------------]
	public:
		inline virtual void onAttachedToSceneNode(SceneNode& sceneNode) override
		{
			// Call base implementation
			LightSceneItem::onAttachedToSceneNode(sceneNode);

			// Calculate derived sunlight properties
			calculatedDerivedSunlightProperties();
		}


	//[-------------------------------------------------------]
	//[ Protected methods                                     ]
	//[-------------------------------------------------------]
	protected:
		inline explicit SunlightSceneItem(SceneResource& sceneResource) :
			LightSceneItem(sceneResource),
			// Usually fixed
			mSunriseTime(07.50f),	// O'clock
			mSunsetTime(20.50f),	// O'clock
			mEastDirection(0.0f),
			mAngleOfIncidence(glm::radians(20.0f)),
			// Usually animated
			mTimeOfDay(10.00f)		// O'clock
		{
			setLightTypeAndRadius(LightType::DIRECTIONAL, 0.0f);
		}

		inline virtual ~SunlightSceneItem() override
		{
			// Nothing here
		}

		explicit SunlightSceneItem(const SunlightSceneItem&) = delete;
		SunlightSceneItem& operator=(const SunlightSceneItem&) = delete;
		RENDERERRUNTIME_API_EXPORT void calculatedDerivedSunlightProperties();


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		// Usually fixed
		float mSunriseTime;			///< Sunrise time in "hour.minute"
		float mSunsetTime;			///< Sunset time in "hour.minute"
		float mEastDirection;		///< East direction in radians, clockwise orientation starting from north for zero
		float mAngleOfIncidence;	///< Angle of incidence in radians
		// Usually animated
		float mTimeOfDay;			///< Current time-of-day in "hour.minute"


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

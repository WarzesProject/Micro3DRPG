#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Core/Manager.h"
#include "RenderQueue/Renderable.h"


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class Transform;
}


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
	*    Renderable collection management
	*
	*  @note
	*    - Example: Abstract representation of an mesh scene item containing sub-meshes
	*/
	class RenderableManager final : private Manager
	{


	//[-------------------------------------------------------]
	//[ Public definitions                                    ]
	//[-------------------------------------------------------]
	public:
		typedef std::vector<Renderable> Renderables;


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		RenderableManager();

		inline ~RenderableManager()
		{
			// Nothing here
		}

		//[-------------------------------------------------------]
		//[ Data                                                  ]
		//[-------------------------------------------------------]
		[[nodiscard]] inline const Renderables& getRenderables() const
		{
			return mRenderables;
		}

		[[nodiscard]] inline Renderables& getRenderables()	// Don't forget to call "RendererRuntime::RenderableManager::updateCachedRenderablesData()" if you changed something relevant in here
		{
			return mRenderables;
		}

		[[nodiscard]] inline const Transform& getTransform() const
		{
			// We know that this pointer is always valid
			assert(nullptr != mTransform);
			return *mTransform;
		}

		void setTransform(const Transform* transform);	// Can be a null pointer (internally a identity transform will be set), transform instance must stay valid as long as the renderable manager is referencing it

		[[nodiscard]] inline bool isVisible() const
		{
			return mVisible;
		}

		inline void setVisible(bool visible)
		{
			mVisible = visible;
		}

		//[-------------------------------------------------------]
		//[ Cached data                                           ]
		//[-------------------------------------------------------]
		[[nodiscard]] inline float getCachedDistanceToCamera() const
		{
			return mCachedDistanceToCamera;
		}

		inline void setCachedDistanceToCamera(float distanceToCamera)
		{
			mCachedDistanceToCamera = distanceToCamera;
		}

		/**
		*  @brief
		*    Return the minimum renderables render queue index (inclusive)
		*
		*  @return
		*    The minimum renderables render queue index (inclusive)
		*
		*  @see
		*    - "RendererRuntime::RenderableManager::updateCachedRenderablesData()"
		*/
		[[nodiscard]] inline uint8_t getMinimumRenderQueueIndex() const
		{
			return mMinimumRenderQueueIndex;
		}

		/**
		*  @brief
		*    Return the maximum renderables render queue index (inclusive)
		*
		*  @return
		*    The maximum renderables render queue index (inclusive)
		*
		*  @see
		*    - "RendererRuntime::RenderableManager::updateCachedRenderablesData()"
		*/
		[[nodiscard]] inline uint8_t getMaximumRenderQueueIndex() const
		{
			return mMaximumRenderQueueIndex;
		}

		/**
		*  @brief
		*    Return whether or not at least one of the renderables is casting shadows
		*
		*  @return
		*    "true" if at least one of the renderables is casting shadows, else "false"
		*
		*  @see
		*    - "RendererRuntime::RenderableManager::updateCachedRenderablesData()"
		*/
		[[nodiscard]] inline bool getCastShadows() const
		{
			return mCastShadows;
		}

		/**
		*  @brief
		*    Update cached renderables data
		*
		*  @note
		*    - Assumed to not be called frequently, optimally only during renderable manager initialization
		*    - Usually only called by the renderable manager owner since renderables and the render queue index and shadow casting data is assumed to not change frequently
		*    - Updates the minimum and maximum renderables render queue index (inclusive) as well as whether or not at least one of the renderables is casting shadows
		*/
		RENDERERRUNTIME_API_EXPORT void updateCachedRenderablesData();


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		explicit RenderableManager(const RenderableManager&) = delete;
		RenderableManager& operator=(const RenderableManager&) = delete;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		// Data
		Renderables		 mRenderables;				///< Renderables
		const Transform* mTransform;				///< Transform instance, always valid, just shared meaning doesn't own the instance so don't delete it
		bool			 mVisible;
		// Cached data
		float			 mCachedDistanceToCamera;	///< Cached distance to camera is updated during the culling phase
		uint8_t			 mMinimumRenderQueueIndex;	///< The minimum renderables render queue index (inclusive, set inside "RendererRuntime::RenderableManager::updateCachedRenderablesData()")
		uint8_t			 mMaximumRenderQueueIndex;	///< The maximum renderables render queue index (inclusive, set inside "RendererRuntime::RenderableManager::updateCachedRenderablesData()")
		bool			 mCastShadows;				///< "true" if at least one of the renderables is casting shadows, else "false" (set inside "RendererRuntime::RenderableManager::updateCachedRenderablesData()")


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Core/Platform/PlatformTypes.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4127)	// warning C4127: conditional expression is constant
	PRAGMA_WARNING_DISABLE_MSVC(4201)	// warning C4201: nonstandard extension used: nameless struct/union
	PRAGMA_WARNING_DISABLE_MSVC(4464)	// warning C4464: relative include path contains '..'
	PRAGMA_WARNING_DISABLE_MSVC(4324)	// warning C4324: '<x>': structure was padded due to alignment specifier
	#include <glm/glm.hpp>
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class LightSceneItem;
	class CameraSceneItem;
	class MaterialBlueprintResource;
	class CompositorWorkspaceInstance;
	class CompositorInstancePassShadowMap;
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
	*    Compositor context data used during compositor execution
	*/
	class CompositorContextData final
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class RenderQueue;	// Needs access to "mCurrentlyBoundMaterialBlueprintResource" and "mGlobalComputeSize"


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		inline CompositorContextData() :
			mCompositorWorkspaceInstance(nullptr),
			mCameraSceneItem(nullptr),
			mSinglePassStereoInstancing(false),
			mLightSceneItem(nullptr),
			mCompositorInstancePassShadowMap(nullptr),
			mWorldSpaceCameraPosition(0.0, 0.0, 0.0),
			mCurrentlyBoundMaterialBlueprintResource(nullptr),
			mGlobalComputeSize{0, 0, 0}
		{
			// Nothing here
		}

		CompositorContextData(const CompositorWorkspaceInstance* compositorWorkspaceInstance, const CameraSceneItem* cameraSceneItem, bool singlePassStereoInstancing = false, const LightSceneItem* lightSceneItem = nullptr, const CompositorInstancePassShadowMap* compositorInstancePassShadowMap = nullptr);

		inline ~CompositorContextData()
		{
			// Nothing here
		}

		[[nodiscard]] inline const CompositorWorkspaceInstance* getCompositorWorkspaceInstance() const
		{
			return mCompositorWorkspaceInstance;
		}

		[[nodiscard]] inline const CameraSceneItem* getCameraSceneItem() const
		{
			return mCameraSceneItem;
		}

		[[nodiscard]] inline bool getSinglePassStereoInstancing() const
		{
			return mSinglePassStereoInstancing;
		}

		[[nodiscard]] inline const LightSceneItem* getLightSceneItem() const
		{
			return mLightSceneItem;
		}

		[[nodiscard]] inline const CompositorInstancePassShadowMap* getCompositorInstancePassShadowMap() const
		{
			return mCompositorInstancePassShadowMap;
		}

		[[nodiscard]] inline const glm::dvec3& getWorldSpaceCameraPosition() const
		{
			// 64 bit world space position of the camera
			return mWorldSpaceCameraPosition;
		}

		inline void resetCurrentlyBoundMaterialBlueprintResource() const
		{
			mCurrentlyBoundMaterialBlueprintResource = nullptr;
		}

		[[nodiscard]] inline MaterialBlueprintResource* getCurrentlyBoundMaterialBlueprintResource() const
		{
			return mCurrentlyBoundMaterialBlueprintResource;
		}

		[[nodiscard]] inline uint32_t* getGlobalComputeSize() const
		{
			return mGlobalComputeSize;
		}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		explicit CompositorContextData(const CompositorContextData&) = delete;
		CompositorContextData& operator=(const CompositorContextData&) = delete;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		const CompositorWorkspaceInstance*	   mCompositorWorkspaceInstance;
		const CameraSceneItem*				   mCameraSceneItem;
		bool								   mSinglePassStereoInstancing;
		const LightSceneItem*				   mLightSceneItem;
		const CompositorInstancePassShadowMap* mCompositorInstancePassShadowMap;
		// Cached data
		glm::dvec3 mWorldSpaceCameraPosition;	///< Cached 64 bit world space position of the camera since often accessed due to camera relative rendering
		// Cached "RendererRuntime::RenderQueue" data to reduce the number of state changes across different render queue instances (beneficial for complex compositors with e.g. multiple Gaussian blur passes)
		mutable MaterialBlueprintResource* mCurrentlyBoundMaterialBlueprintResource;
		mutable uint32_t				   mGlobalComputeSize[3];


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

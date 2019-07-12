#include "stdafx.h"
#include "Resource/Scene/Item/Light/LightSceneItem.h"
#include "Resource/Scene/Loader/SceneFileFormat.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4201)	// warning C4201: nonstandard extension used: nameless struct/union
	PRAGMA_WARNING_DISABLE_MSVC(4464)	// warning C4464: relative include path contains '..'
	#include <glm/gtc/type_ptr.hpp>
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public RendererRuntime::ISceneItem methods            ]
	//[-------------------------------------------------------]
	void LightSceneItem::deserialize([[maybe_unused]] uint32_t numberOfBytes, const uint8_t* data)
	{
		RENDERER_ASSERT(getContext(), sizeof(v1Scene::LightItem) == numberOfBytes, "Invalid number of bytes")

		// Read data
		const v1Scene::LightItem* lightItem = reinterpret_cast<const v1Scene::LightItem*>(data);
		mPackedShaderData.lightType = static_cast<float>(lightItem->lightType);
		memcpy(glm::value_ptr(mPackedShaderData.color), lightItem->color, sizeof(float) * 3);
		mPackedShaderData.radius = lightItem->radius;
		setInnerOuterAngle(lightItem->innerAngle, lightItem->outerAngle);
		mPackedShaderData.nearClipDistance = lightItem->nearClipDistance;
		mPackedShaderData.iesLightProfileIndex = static_cast<float>(lightItem->iesLightProfileIndex);

		// Sanity checks
		RENDERER_ASSERT(getContext(), mPackedShaderData.color.x >= 0.0f && mPackedShaderData.color.y >= 0.0f && mPackedShaderData.color.z >= 0.0f, "Invalid data")
		RENDERER_ASSERT(getContext(), lightItem->lightType == LightType::DIRECTIONAL || mPackedShaderData.radius > 0.0f, "Invalid data")
		RENDERER_ASSERT(getContext(), lightItem->lightType != LightType::DIRECTIONAL || 0.0f == mPackedShaderData.radius, "Invalid data")
		RENDERER_ASSERT(getContext(), mInnerAngle >= 0.0f, "Invalid data")
		RENDERER_ASSERT(getContext(), mOuterAngle < glm::radians(90.0f), "Invalid data")
		RENDERER_ASSERT(getContext(), mInnerAngle < mOuterAngle, "Invalid data")
		RENDERER_ASSERT(getContext(), mPackedShaderData.nearClipDistance >= 0.0f, "Invalid data")
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

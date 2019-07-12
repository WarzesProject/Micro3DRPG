#include "stdafx.h"
#include "Resource/CompositorNode/Pass/ShadowMap/CompositorInstancePassShadowMap.h"
#include "Resource/CompositorNode/Pass/Compute/CompositorInstancePassCompute.h"
#include "Resource/CompositorNode/Pass/Compute/CompositorResourcePassCompute.h"
#include "Resource/CompositorNode/CompositorNodeInstance.h"
#include "Resource/CompositorWorkspace/CompositorContextData.h"
#include "Resource/Texture/TextureResourceManager.h"
#include "Resource/Scene/Item/Camera/CameraSceneItem.h"
#include "Resource/Scene/Item/Light/LightSceneItem.h"
#include "Resource/Scene/SceneNode.h"
#include "RenderQueue/RenderableManager.h"
#include "Core/Math/Math.h"
#include "Core/IProfiler.h"
#include "IRendererRuntime.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4464)	// warning C4464: relative include path contains '..'
	#include <glm/gtc/matrix_transform.hpp>
PRAGMA_WARNING_POP

#include <algorithm>


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
		static constexpr float	  SHADOW_MAP_FILTER_SIZE							  = 7.0f;
		static constexpr uint8_t  INTERMEDIATE_CASCADE_INDEX						  = 3;
		static constexpr uint32_t DEPTH_SHADOW_MAP_TEXTURE_ASSET_ID					  = ASSET_ID("Unrimp/Texture/DynamicByCode/DepthShadowMap");
		static constexpr uint32_t INTERMEDIATE_DEPTH_BLUR_SHADOW_MAP_TEXTURE_ASSET_ID = ASSET_ID("Unrimp/Texture/DynamicByCode/IntermediateDepthBlurShadowMap");


		//[-------------------------------------------------------]
		//[ Global functions                                      ]
		//[-------------------------------------------------------]
		[[nodiscard]] glm::vec4 transformVectorByMatrix(const glm::mat4& matrix, const glm::vec4& vector)
		{
			const glm::vec4 temporaryVector = matrix * vector;
			return temporaryVector / temporaryVector.w;
		}


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
	void CompositorInstancePassShadowMap::setNumberOfShadowCascades(uint8_t numberOfShadowCascades)
	{
		if (mNumberOfShadowCascades != numberOfShadowCascades)
		{
			RENDERER_ASSERT(getCompositorNodeInstance().getCompositorWorkspaceInstance().getRendererRuntime().getContext(), numberOfShadowCascades <= CompositorResourcePassShadowMap::MAXIMUM_NUMBER_OF_SHADOW_CASCADES, "Invalid number of shadow cascades")
			mNumberOfShadowCascades = numberOfShadowCascades;
			++mSettingsGenerationCounter;
		}
	}

	void CompositorInstancePassShadowMap::setNumberOfShadowMultisamples(uint8_t numberOfShadowMultisamples)
	{
		if (mNumberOfShadowMultisamples != numberOfShadowMultisamples)
		{
			RENDERER_ASSERT(getCompositorNodeInstance().getCompositorWorkspaceInstance().getRendererRuntime().getContext(), numberOfShadowMultisamples >= 1, "Invalid number of shadow multisamples")
			RENDERER_ASSERT(getCompositorNodeInstance().getCompositorWorkspaceInstance().getRendererRuntime().getContext(), numberOfShadowMultisamples <= getCompositorNodeInstance().getCompositorWorkspaceInstance().getRendererRuntime().getRenderer().getCapabilities().maximumNumberOfMultisamples, "Invalid number of shadow multisamples")
			mNumberOfShadowMultisamples = numberOfShadowMultisamples;
			++mSettingsGenerationCounter;
		}
	}


	//[-------------------------------------------------------]
	//[ Protected virtual RendererRuntime::ICompositorInstancePass methods ]
	//[-------------------------------------------------------]
	void CompositorInstancePassShadowMap::onFillCommandBuffer([[maybe_unused]] const Renderer::IRenderTarget* renderTarget, const CompositorContextData& compositorContextData, Renderer::CommandBuffer& commandBuffer)
	{
		const CompositorWorkspaceInstance& compositorWorkspaceInstance = getCompositorNodeInstance().getCompositorWorkspaceInstance();
		const IRendererRuntime& rendererRuntime = compositorWorkspaceInstance.getRendererRuntime();

		// Sanity check
		RENDERER_ASSERT(rendererRuntime.getContext(), nullptr == renderTarget, "The shadow map compositor instance pass needs an invalid render target")

		// Settings update handling
		if (mUsedSettingsGenerationCounter != mSettingsGenerationCounter)
		{
			mUsedSettingsGenerationCounter = mSettingsGenerationCounter;
			destroyShadowMapRenderTarget();
			createShadowMapRenderTarget();
		}

		// Early escape if shadow is disabled
		if (!mEnabled)
		{
			return;
		}

		// Fill command buffer
		const CameraSceneItem* cameraSceneItem = compositorContextData.getCameraSceneItem();
		const LightSceneItem* lightSceneItem = compositorContextData.getLightSceneItem();
		if (nullptr != mDepthFramebufferPtr && nullptr != cameraSceneItem && cameraSceneItem->getParentSceneNode() && nullptr != lightSceneItem && lightSceneItem->getParentSceneNode())
		{
			const glm::vec3 worldSpaceSunlightDirection = lightSceneItem->getParentSceneNode()->getGlobalTransform().rotation * Math::VEC3_FORWARD;
			mPassData.shadowMapSize = static_cast<int>(mShadowMapSize);

			// TODO(co) Stabilize cascades Reversed-Z update
			const bool stabilizeCascades = false;
//			const bool stabilizeCascades = mStabilizeCascades;

			// TODO(co) The minimum and maximum distance need to be calculated dynamically via depth buffer reduction as seen inside e.g. https://github.com/TheRealMJP/MSAAFilter/tree/master/MSAAFilter
			const float minimumDistance = 0.0f;
			const float maximumDistance = 0.5f;

			// Compute the split distances based on the partitioning mode
			float cascadeSplits[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			{
				const float nearClip = cameraSceneItem->getNearZ();
				const float farClip = cameraSceneItem->getFarZ();
				const float clipRange = farClip - nearClip;
				const float minimumZ = nearClip + minimumDistance * clipRange;
				const float maximumZ = nearClip + maximumDistance * clipRange;
				const float range = maximumZ - minimumZ;
				const float ratio = maximumZ / minimumZ;

				for (uint8_t cascadeIndex = 0; cascadeIndex < mNumberOfShadowCascades; ++cascadeIndex)
				{
					const float p = (cascadeIndex + 1) / static_cast<float>(mNumberOfShadowCascades);
					const float log = minimumZ * std::pow(ratio, p);
					const float uniform = minimumZ + range * p;
					const float d = mCascadeSplitsLambda * (log - uniform) + uniform;
					cascadeSplits[cascadeIndex] = (d - nearClip) / clipRange;
				}
			}

			// Coordinate system related adjustments
			// -> Vulkan and Direct3D: Left-handed coordinate system with clip space depth value range 0..1
			// -> OpenGL without "GL_ARB_clip_control"-extension: Right-handed coordinate system with clip space depth value range -1..1
			const float nearZ = rendererRuntime.getRenderer().getCapabilities().zeroToOneClipZ ? 0.0f : -1.0f;

			// Get the 8 points of the view frustum in world space
			glm::vec4 worldSpaceFrustumCorners[8] =
			{
				// Near
				{-1.0f,  1.0f, nearZ, 1.0f},	// 0: Near top left
				{ 1.0f,  1.0f, nearZ, 1.0f},	// 1: Near top right
				{-1.0f, -1.0f, nearZ, 1.0f},	// 2: Near bottom left
				{ 1.0f, -1.0f, nearZ, 1.0f},	// 3: Near bottom right
				// Far
				{-1.0f,  1.0f, 1.0f, 1.0f},		// 4: Far top left
				{ 1.0f,  1.0f, 1.0f, 1.0f},		// 5: Far top right
				{-1.0f, -1.0f, 1.0f, 1.0f},		// 6: Far bottom left
				{ 1.0f, -1.0f, 1.0f, 1.0f}		// 7: Far bottom right
			};
			{
				uint32_t renderTargetWidth = 0;
				uint32_t renderTargetHeight = 0;
				RENDERER_ASSERT(rendererRuntime.getContext(), nullptr != compositorWorkspaceInstance.getExecutionRenderTarget(), "Invalid compositor workspace instance execution render target")
				compositorWorkspaceInstance.getExecutionRenderTarget()->getWidthAndHeight(renderTargetWidth, renderTargetHeight);
				if (compositorContextData.getSinglePassStereoInstancing())
				{
					renderTargetWidth /= 2;
				}
				const glm::mat4 worldSpaceToClipSpaceMatrix = cameraSceneItem->getViewSpaceToClipSpaceMatrix(static_cast<float>(renderTargetWidth) / renderTargetHeight) * cameraSceneItem->getCameraRelativeWorldSpaceToViewSpaceMatrix();
				const glm::mat4 clipSpaceToWorldSpaceMatrix = glm::inverse(worldSpaceToClipSpaceMatrix);
				for (int i = 0; i < 8; ++i)
				{
					worldSpaceFrustumCorners[i] = ::detail::transformVectorByMatrix(clipSpaceToWorldSpaceMatrix, worldSpaceFrustumCorners[i]);
				}
			}

			// Combined scoped profiler CPU and GPU sample as well as renderer debug event command
			RENDERER_SCOPED_PROFILER_EVENT_DYNAMIC(rendererRuntime.getContext(), commandBuffer, static_cast<const CompositorResourcePassShadowMap&>(getCompositorResourcePass()).getDebugName())

			// Render the meshes to each cascade
			// -> Shadows should never be rendered via single pass stereo instancing
			const CompositorContextData shadowCompositorContextData(compositorContextData.getCompositorWorkspaceInstance(), compositorContextData.getCameraSceneItem(), false, compositorContextData.getLightSceneItem(), compositorContextData.getCompositorInstancePassShadowMap());
			for (uint8_t cascadeIndex = 0; cascadeIndex < mNumberOfShadowCascades; ++cascadeIndex)
			{
				// Scoped debug event
				RENDERER_SCOPED_PROFILER_EVENT_DYNAMIC(rendererRuntime.getContext(), commandBuffer, ("Shadow cascade " + std::to_string(cascadeIndex)).c_str())

				// Compute the MVP matrix from the light's point of view
				glm::mat4 depthProjectionMatrix;
				glm::mat4 depthViewMatrix;
				glm::vec3 minimumExtents;
				glm::vec3 maximumExtents;
				glm::vec3 cascadeExtents;
				const float splitDistance = cascadeSplits[cascadeIndex];
				{
					const float previousSplitDistance = (0 == cascadeIndex) ? minimumDistance : cascadeSplits[cascadeIndex - 1];

					// Get the corners of the current cascade slice of the view frustum
					glm::vec4 cascadeSliceWorldSpaceFrustumCorners[8];
					for (int i = 0; i < 4; ++i)
					{
						const glm::vec4 cornerRay = worldSpaceFrustumCorners[i + 4] - worldSpaceFrustumCorners[i];
						const glm::vec4 nearCornerRay = cornerRay * previousSplitDistance;
						const glm::vec4 farCornerRay = cornerRay * splitDistance;
						cascadeSliceWorldSpaceFrustumCorners[i + 4] = worldSpaceFrustumCorners[i] + farCornerRay;
						cascadeSliceWorldSpaceFrustumCorners[i] = worldSpaceFrustumCorners[i] + nearCornerRay;
					}

					// Calculate the centroid of the view frustum slice
					glm::vec4 temporaryFrustumCenter = Math::VEC4_ZERO;
					for (int i = 0; i < 8; ++i)
					{
						temporaryFrustumCenter += cascadeSliceWorldSpaceFrustumCorners[i];
					}
					const glm::vec3 frustumCenter = temporaryFrustumCenter / 8.0f;

					// Pick the right vector to use for the light camera, this needs to be constant for it to be stable
					const glm::vec3 rightDirection = stabilizeCascades ? Math::VEC3_RIGHT : (cameraSceneItem->getParentSceneNodeSafe().getTransform().rotation * Math::VEC3_RIGHT);

					// Calculate the minimum and maximum extents
					if (stabilizeCascades)
					{
						// Calculate the radius of a bounding sphere surrounding the frustum corners
						float sphereRadius = 0.0f;
						for (int i = 0; i < 8; ++i)
						{
							const float distance = glm::distance(glm::vec3(cascadeSliceWorldSpaceFrustumCorners[i]), frustumCenter);
							sphereRadius = std::max(sphereRadius, distance);
						}
						sphereRadius = std::ceil(sphereRadius * 16.0f) / 16.0f;
						maximumExtents = glm::vec3(sphereRadius, sphereRadius, sphereRadius);
						minimumExtents = -maximumExtents;
					}
					else
					{
						// Create a temporary view matrix for the light
						const glm::vec3& lightCameraPosition = frustumCenter;
						const glm::vec3 lightCameraTarget = frustumCenter - worldSpaceSunlightDirection;
						const glm::mat4 lightView = glm::lookAt(lightCameraPosition, lightCameraTarget, rightDirection);

						// Calculate an AABB around the frustum corners
						glm::vec4 mins(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
						glm::vec4 maxes(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
						for (int i = 0; i < 8; ++i)
						{
							const glm::vec4 corner = ::detail::transformVectorByMatrix(lightView, cascadeSliceWorldSpaceFrustumCorners[i]);
							mins = glm::min(mins, corner);
							maxes = glm::max(maxes, corner);
						}
						minimumExtents = mins;
						maximumExtents = maxes;

						// Adjust the minimum/maximum to accommodate the filtering size
						const float scale = (static_cast<float>(mShadowMapSize) + ::detail::SHADOW_MAP_FILTER_SIZE) / static_cast<float>(mShadowMapSize);
						minimumExtents.x *= scale;
						minimumExtents.y *= scale;
						maximumExtents.x *= scale;
						maximumExtents.x *= scale;
					}
					cascadeExtents = maximumExtents - minimumExtents;

					// Get position of the shadow camera
					const glm::vec3 shadowCameraPosition = frustumCenter + worldSpaceSunlightDirection * -minimumExtents.z;

					// Come up with a new orthographic camera for the shadow caster
					depthProjectionMatrix = glm::ortho(minimumExtents.x, maximumExtents.x, minimumExtents.y, maximumExtents.y, 0.0f, cascadeExtents.z);
					depthViewMatrix = glm::lookAt(shadowCameraPosition, frustumCenter, rightDirection);
				}

				// Create the rounding matrix, by projecting the world-space origin and determining the fractional offset in texel space
				glm::mat4 viewSpaceToClipSpace = depthProjectionMatrix * depthViewMatrix;
				if (stabilizeCascades)
				{
					glm::vec4 shadowOrigin(0.0f, 0.0f, 0.0f, 1.0f);
					shadowOrigin = ::detail::transformVectorByMatrix(viewSpaceToClipSpace, shadowOrigin);
					shadowOrigin *= static_cast<float>(mShadowMapSize) * 0.5f;

					const glm::vec4 roundedOrigin = glm::round(shadowOrigin);
					glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
					roundOffset *= 2.0f / static_cast<float>(mShadowMapSize);
					roundOffset.z = 0.0f;
					roundOffset.w = 0.0f;

					depthProjectionMatrix[3] += roundOffset;
					viewSpaceToClipSpace = depthProjectionMatrix * depthViewMatrix;
				}

				// Set custom camera matrices
				const_cast<CameraSceneItem*>(cameraSceneItem)->setCustomWorldSpaceToViewSpaceMatrix(depthViewMatrix);
				const_cast<CameraSceneItem*>(cameraSceneItem)->setCustomViewSpaceToClipSpaceMatrix(depthProjectionMatrix, glm::ortho(minimumExtents.x, maximumExtents.x, minimumExtents.y, maximumExtents.y, cascadeExtents.z, 0.0f));

				{ // Render shadow casters
					// Scoped debug event
					RENDERER_SCOPED_PROFILER_EVENT(rendererRuntime.getContext(), commandBuffer, "Render shadow casters")

					// Set graphics render target
					Renderer::Command::SetGraphicsRenderTarget::create(commandBuffer, mDepthFramebufferPtr);

					// Set the graphics viewport and scissor rectangle
					Renderer::Command::SetGraphicsViewportAndScissorRectangle::create(commandBuffer, 0, 0, mShadowMapSize, mShadowMapSize);

					{ // Clear the graphics depth buffer of the current render target
						const float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
						Renderer::Command::ClearGraphics::create(commandBuffer, Renderer::ClearFlag::DEPTH, color);
					}

					// Render shadow casters
					// TODO(co) Optimization: Do only render stuff which calls into the current shadow cascade
					RENDERER_ASSERT(rendererRuntime.getContext(), nullptr != mRenderQueueIndexRange, "Invalid render queue index range")
					const MaterialTechniqueId materialTechniqueId = static_cast<const CompositorResourcePassScene&>(getCompositorResourcePass()).getMaterialTechniqueId();
					for (const RenderableManager* renderableManager : mRenderQueueIndexRange->renderableManagers)
					{
						// The render queue index range covered by this compositor instance pass scene might be smaller than the range of the
						// cached render queue index range. So, we could add a range check in here to reject renderable managers, but it's not
						// really worth to do so since the render queue only considers renderables inside the render queue range anyway.
						if (renderableManager->getCastShadows())
						{
							mRenderQueue.addRenderablesFromRenderableManager(*renderableManager, materialTechniqueId, shadowCompositorContextData, true);
						}
					}
					if (mRenderQueue.getNumberOfDrawCalls() > 0)
					{
						mRenderQueue.fillGraphicsCommandBuffer(*mDepthFramebufferPtr, shadowCompositorContextData, commandBuffer);
						mRenderQueue.clear();
					}
				}

				// Unset custom camera matrices
				const_cast<CameraSceneItem*>(cameraSceneItem)->unsetCustomWorldSpaceToViewSpaceMatrix();
				const_cast<CameraSceneItem*>(cameraSceneItem)->unsetCustomViewSpaceToClipSpaceMatrix();

				// Apply the scale/offset matrix, which transforms from [-1,1] post-projection space to [0,1] UV space
				const glm::mat4 shadowMatrix = Math::getTextureScaleBiasMatrix(rendererRuntime.getRenderer()) * viewSpaceToClipSpace;

				// Store the split distance in terms of view space depth
				const float clipDistance = cameraSceneItem->getFarZ() - cameraSceneItem->getNearZ();
				mPassData.shadowCascadeSplits[cascadeIndex] = cameraSceneItem->getNearZ() + splitDistance * clipDistance;
				if (0 == cascadeIndex)
				{
					mPassData.shadowMatrix = shadowMatrix;
					mPassData.shadowCascadeOffsets[0] = Math::VEC4_ZERO;
					mPassData.shadowCascadeScales[0] = Math::VEC4_ONE;
				}
				else
				{
					// Calculate the position of the lower corner of the cascade partition, in the UV space of the first cascade partition
					const glm::mat4 inverseShadowMatrix = glm::inverse(shadowMatrix);
					glm::vec4 cascadeCorner = ::detail::transformVectorByMatrix(inverseShadowMatrix, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
					cascadeCorner = ::detail::transformVectorByMatrix(mPassData.shadowMatrix, cascadeCorner);

					// Do the same for the upper corner
					glm::vec4 otherCorner = ::detail::transformVectorByMatrix(inverseShadowMatrix, Math::VEC4_ONE);
					otherCorner = ::detail::transformVectorByMatrix(mPassData.shadowMatrix, otherCorner);

					// Calculate the scale and offset
					const glm::vec4 cascadeScale = Math::VEC4_ONE / (otherCorner - cascadeCorner);
					mPassData.shadowCascadeOffsets[cascadeIndex] = glm::vec4(-glm::vec3(cascadeCorner), 0.0f);
					mPassData.shadowCascadeScales[cascadeIndex] = glm::vec4(glm::vec3(cascadeScale), 1.0f);
				}
				mPassData.currentShadowCascadeIndex = cascadeIndex;

				// Calculate exponential variance shadow map (EVSM) and blur if necessary
				const glm::vec4& cascadeScale = mPassData.shadowCascadeScales[cascadeIndex];
				const float filterSizeX = std::max(mShadowFilterSize * cascadeScale.x, 1.0f);
				const float filterSizeY = std::max(mShadowFilterSize * cascadeScale.y, 1.0f);
				if (filterSizeX > 1.0f || filterSizeY > 1.0f)
				{
					// Execute compositor instance pass compute, use cascade index three as intermediate render target
					RENDERER_ASSERT(rendererRuntime.getContext(), nullptr != mVarianceFramebufferPtr[::detail::INTERMEDIATE_CASCADE_INDEX], "Invalid variance framebuffer")
					Renderer::Command::SetGraphicsRenderTarget::create(commandBuffer, mVarianceFramebufferPtr[::detail::INTERMEDIATE_CASCADE_INDEX]);
					mDepthToExponentialVarianceCompositorInstancePassCompute->onFillCommandBuffer(mVarianceFramebufferPtr[::detail::INTERMEDIATE_CASCADE_INDEX], shadowCompositorContextData, commandBuffer);
					mDepthToExponentialVarianceCompositorInstancePassCompute->onPostCommandBufferExecution();

					// Horizontal blur
					mPassData.shadowFilterSize = filterSizeX;
					Renderer::Command::SetGraphicsRenderTarget::create(commandBuffer, mIntermediateFramebufferPtr);
					mHorizontalBlurCompositorInstancePassCompute->onFillCommandBuffer(mIntermediateFramebufferPtr, shadowCompositorContextData, commandBuffer);
					mHorizontalBlurCompositorInstancePassCompute->onPostCommandBufferExecution();

					// Vertical blur
					mPassData.shadowFilterSize = filterSizeY;
					RENDERER_ASSERT(rendererRuntime.getContext(), nullptr != mVarianceFramebufferPtr[cascadeIndex], "Invalid variance framebuffer")
					Renderer::Command::SetGraphicsRenderTarget::create(commandBuffer, mVarianceFramebufferPtr[cascadeIndex]);
					mVerticalBlurCompositorInstancePassCompute->onFillCommandBuffer(mVarianceFramebufferPtr[cascadeIndex], shadowCompositorContextData, commandBuffer);
					mVerticalBlurCompositorInstancePassCompute->onPostCommandBufferExecution();
				}
				else
				{
					// Execute compositor instance pass compute
					RENDERER_ASSERT(rendererRuntime.getContext(), nullptr != mVarianceFramebufferPtr[cascadeIndex], "Invalid variance framebuffer")
					Renderer::Command::SetGraphicsRenderTarget::create(commandBuffer, mVarianceFramebufferPtr[cascadeIndex]);
					mDepthToExponentialVarianceCompositorInstancePassCompute->onFillCommandBuffer(mVarianceFramebufferPtr[cascadeIndex], shadowCompositorContextData, commandBuffer);
					mDepthToExponentialVarianceCompositorInstancePassCompute->onPostCommandBufferExecution();
				}
			}
		}
		else
		{
			// Error!
			RENDERER_ASSERT(rendererRuntime.getContext(), false, "We should never end up in here")
		}
	}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	CompositorInstancePassShadowMap::CompositorInstancePassShadowMap(const CompositorResourcePassShadowMap& compositorResourcePassShadowMap, const CompositorNodeInstance& compositorNodeInstance) :
		CompositorInstancePassScene(compositorResourcePassShadowMap, compositorNodeInstance),
		// Settings
		mEnabled(true),
		mShadowMapSize(1024),
		mNumberOfShadowCascades(4),
		mNumberOfShadowMultisamples(2),
		mCascadeSplitsLambda(0.99f),
		mShadowFilterSize(8.0f),
		mStabilizeCascades(true),
		// Internal
		mSettingsGenerationCounter(0),
		mUsedSettingsGenerationCounter(0),
		mDepthTextureResourceId(getInvalid<TextureResourceId>()),
		mVarianceTextureResourceId(getInvalid<TextureResourceId>()),
		mIntermediateDepthBlurTextureResourceId(getInvalid<TextureResourceId>()),
		mDepthToExponentialVarianceCompositorResourcePassCompute(nullptr),
		mDepthToExponentialVarianceCompositorInstancePassCompute(nullptr),
		mHorizontalBlurCompositorResourcePassCompute(nullptr),
		mHorizontalBlurCompositorInstancePassCompute(nullptr),
		mVerticalBlurCompositorResourcePassCompute(nullptr),
		mVerticalBlurCompositorInstancePassCompute(nullptr)
	{
		mPassData.shadowMatrix = Math::MAT4_IDENTITY;
		for (int i = 0; i < CompositorResourcePassShadowMap::MAXIMUM_NUMBER_OF_SHADOW_CASCADES; ++i)
		{
			mPassData.shadowCascadeScales[i] = Math::VEC4_ONE;
		}
		createShadowMapRenderTarget();
	}

	void CompositorInstancePassShadowMap::createShadowMapRenderTarget()
	{
		const CompositorResourcePassShadowMap& compositorResourcePassShadowMap = static_cast<const CompositorResourcePassShadowMap&>(getCompositorResourcePass());
		const IRendererRuntime& rendererRuntime = getCompositorNodeInstance().getCompositorWorkspaceInstance().getRendererRuntime();
		const AssetId assetId = compositorResourcePassShadowMap.getTextureAssetId();

		// Tell the texture resource manager about our render target texture so it can be referenced inside e.g. compositor nodes
		TextureResourceManager& textureResourceManager = rendererRuntime.getTextureResourceManager();
		TextureResource* textureResource = textureResourceManager.getTextureResourceByAssetId(assetId);
		if (nullptr == textureResource)
		{
			Renderer::IRenderer& renderer = rendererRuntime.getRenderer();
			if (mEnabled)
			{
				// Check shadow map settings
				RENDERER_ASSERT(rendererRuntime.getContext(), mNumberOfShadowCascades <= CompositorResourcePassShadowMap::MAXIMUM_NUMBER_OF_SHADOW_CASCADES, "Invalid number of shadow cascades")
				RENDERER_ASSERT(rendererRuntime.getContext(), mNumberOfShadowMultisamples >= 1, "Invalid number of shadow multisamples")
				uint8_t numberOfShadowMultisamples = mNumberOfShadowMultisamples;
				{ // Multisamples sanity check
					const uint8_t maximumNumberOfMultisamples = renderer.getCapabilities().maximumNumberOfMultisamples;
					if (numberOfShadowMultisamples > maximumNumberOfMultisamples)
					{
						RENDERER_ASSERT(rendererRuntime.getContext(), false, "Number of shadow multisamples not supported by the renderer backend")
						numberOfShadowMultisamples = maximumNumberOfMultisamples;
					}
				}

				{ // Depth shadow map
					const Renderer::TextureFormat::Enum textureFormat = Renderer::TextureFormat::D32_FLOAT;
					Renderer::ITexture* texture = rendererRuntime.getTextureManager().createTexture2D(mShadowMapSize, mShadowMapSize, textureFormat, nullptr, Renderer::TextureFlag::SHADER_RESOURCE | Renderer::TextureFlag::RENDER_TARGET, Renderer::TextureUsage::DEFAULT, numberOfShadowMultisamples);
					RENDERER_SET_RESOURCE_DEBUG_NAME(texture, "Compositor instance pass depth shadow map")

					// Create the framebuffer object (FBO) instance
					const Renderer::FramebufferAttachment depthStencilFramebufferAttachment(texture);
					mDepthFramebufferPtr = renderer.createFramebuffer(*renderer.createRenderPass(0, nullptr, textureFormat), nullptr, &depthStencilFramebufferAttachment);
					RENDERER_SET_RESOURCE_DEBUG_NAME(mDepthFramebufferPtr, "Compositor instance pass depth shadow map")

					// Create texture resource
					mDepthTextureResourceId = textureResourceManager.createTextureResourceByAssetId(::detail::DEPTH_SHADOW_MAP_TEXTURE_ASSET_ID, *texture);
				}

				{ // Depth to exponential variance
					MaterialProperties materialProperties;
					materialProperties.setPropertyById(STRING_ID("DepthMap"), MaterialPropertyValue::fromTextureAssetId(::detail::DEPTH_SHADOW_MAP_TEXTURE_ASSET_ID), MaterialProperty::Usage::UNKNOWN, true);
					materialProperties.setPropertyById(STRING_ID("NumberOfMultisamples"), MaterialPropertyValue::fromInteger((numberOfShadowMultisamples == 1) ? 0 : numberOfShadowMultisamples), MaterialProperty::Usage::UNKNOWN, true);
					mDepthToExponentialVarianceCompositorResourcePassCompute = new CompositorResourcePassCompute(compositorResourcePassShadowMap.getCompositorTarget(), compositorResourcePassShadowMap.getDepthToExponentialVarianceMaterialBlueprintAssetId(), materialProperties);
					#if defined(_DEBUG) || defined(RENDERER_RUNTIME_PROFILER)
						mDepthToExponentialVarianceCompositorResourcePassCompute->setDebugName("Depth to exponential variance");
					#endif
					mDepthToExponentialVarianceCompositorInstancePassCompute = new CompositorInstancePassCompute(*mDepthToExponentialVarianceCompositorResourcePassCompute, getCompositorNodeInstance());
				}

				{ // Variance shadow map
					const Renderer::TextureFormat::Enum textureFormat = Renderer::TextureFormat::R32G32B32A32F;
					Renderer::ITexture* texture = rendererRuntime.getTextureManager().createTexture2DArray(mShadowMapSize, mShadowMapSize, CompositorResourcePassShadowMap::MAXIMUM_NUMBER_OF_SHADOW_CASCADES, textureFormat, nullptr, Renderer::TextureFlag::SHADER_RESOURCE | Renderer::TextureFlag::RENDER_TARGET);
					RENDERER_SET_RESOURCE_DEBUG_NAME(texture, "Compositor instance pass variance shadow map")

					// Create the framebuffer object (FBO) instances
					Renderer::IRenderPass* renderPass = renderer.createRenderPass(1, &textureFormat);
					for (uint8_t cascadeIndex = 0; cascadeIndex < CompositorResourcePassShadowMap::MAXIMUM_NUMBER_OF_SHADOW_CASCADES; ++cascadeIndex)
					{
						const Renderer::FramebufferAttachment colorFramebufferAttachment(texture, 0, cascadeIndex);
						mVarianceFramebufferPtr[cascadeIndex] = renderer.createFramebuffer(*renderPass, &colorFramebufferAttachment);
						RENDERER_SET_RESOURCE_DEBUG_NAME(mVarianceFramebufferPtr[cascadeIndex], ("Compositor instance pass variance shadow map " + std::to_string(cascadeIndex)).c_str())
					}
					for (uint8_t cascadeIndex = CompositorResourcePassShadowMap::MAXIMUM_NUMBER_OF_SHADOW_CASCADES; cascadeIndex < CompositorResourcePassShadowMap::MAXIMUM_NUMBER_OF_SHADOW_CASCADES; ++cascadeIndex)
					{
						mVarianceFramebufferPtr[cascadeIndex] = nullptr;
					}

					// Create texture resource
					mVarianceTextureResourceId = textureResourceManager.createTextureResourceByAssetId(assetId, *texture);
				}

				{ // Intermediate depth blur shadow map
					const Renderer::TextureFormat::Enum textureFormat = Renderer::TextureFormat::R32G32B32A32F;
					Renderer::ITexture* texture = rendererRuntime.getTextureManager().createTexture2D(mShadowMapSize, mShadowMapSize, textureFormat, nullptr, Renderer::TextureFlag::SHADER_RESOURCE | Renderer::TextureFlag::RENDER_TARGET);
					RENDERER_SET_RESOURCE_DEBUG_NAME(texture, "Compositor instance pass intermediate depth blur shadow map")

					// Create the framebuffer object (FBO) instance
					const Renderer::FramebufferAttachment colorFramebufferAttachment(texture);
					mIntermediateFramebufferPtr = renderer.createFramebuffer(*renderer.createRenderPass(1, &textureFormat), &colorFramebufferAttachment);
					RENDERER_SET_RESOURCE_DEBUG_NAME(mIntermediateFramebufferPtr, "Compositor instance pass intermediate depth blur shadow map")

					// Create texture resource
					mIntermediateDepthBlurTextureResourceId = textureResourceManager.createTextureResourceByAssetId(::detail::INTERMEDIATE_DEPTH_BLUR_SHADOW_MAP_TEXTURE_ASSET_ID, *texture);
				}

				{ // Horizontal blur
					MaterialProperties materialProperties;
					materialProperties.setPropertyById(STRING_ID("VerticalBlur"), MaterialPropertyValue::fromBoolean(false), MaterialProperty::Usage::UNKNOWN, true);
					materialProperties.setPropertyById(STRING_ID("ColorMap"), MaterialPropertyValue::fromTextureAssetId(assetId), MaterialProperty::Usage::UNKNOWN, true);
					mHorizontalBlurCompositorResourcePassCompute = new CompositorResourcePassCompute(compositorResourcePassShadowMap.getCompositorTarget(), compositorResourcePassShadowMap.getBlurMaterialBlueprintAssetId(), materialProperties);
					#if defined(_DEBUG) || defined(RENDERER_RUNTIME_PROFILER)
						mHorizontalBlurCompositorResourcePassCompute->setDebugName("Horizontal blur");
					#endif
					mHorizontalBlurCompositorInstancePassCompute = new CompositorInstancePassCompute(*mHorizontalBlurCompositorResourcePassCompute, getCompositorNodeInstance());
				}

				{ // Vertical blur
					MaterialProperties materialProperties;
					materialProperties.setPropertyById(STRING_ID("VerticalBlur"), MaterialPropertyValue::fromBoolean(true), MaterialProperty::Usage::UNKNOWN, true);
					materialProperties.setPropertyById(STRING_ID("ColorMap"), MaterialPropertyValue::fromTextureAssetId(::detail::INTERMEDIATE_DEPTH_BLUR_SHADOW_MAP_TEXTURE_ASSET_ID), MaterialProperty::Usage::UNKNOWN, true);
					mVerticalBlurCompositorResourcePassCompute = new CompositorResourcePassCompute(compositorResourcePassShadowMap.getCompositorTarget(), compositorResourcePassShadowMap.getBlurMaterialBlueprintAssetId(), materialProperties);
					#if defined(_DEBUG) || defined(RENDERER_RUNTIME_PROFILER)
						mVerticalBlurCompositorResourcePassCompute->setDebugName("Vertical blur");
					#endif
					mVerticalBlurCompositorInstancePassCompute = new CompositorInstancePassCompute(*mVerticalBlurCompositorResourcePassCompute, getCompositorNodeInstance());
				}
			}
			else
			{
				// If shadow is disabled, we still need to create at least a dummy for the resulting main variance shadow map resource
				const Renderer::TextureFormat::Enum textureFormat = Renderer::TextureFormat::R32G32B32A32F;
				const float data[] = { 0.0f, 0.0f, 0.0f, 0.0f };
				Renderer::ITexture* texture = rendererRuntime.getTextureManager().createTexture2DArray(1, 1, 1, textureFormat, data, Renderer::TextureFlag::SHADER_RESOURCE);
				RENDERER_SET_RESOURCE_DEBUG_NAME(texture, "Compositor instance pass variance shadow map")

				// Create texture resource
				mVarianceTextureResourceId = textureResourceManager.createTextureResourceByAssetId(assetId, *texture);
			}
		}
		else
		{
			// This is not allowed to happen
			RENDERER_ASSERT(rendererRuntime.getContext(), false, "We should never end up in here")
		}
	}

	void CompositorInstancePassShadowMap::destroyShadowMapRenderTarget()
	{
		RENDERER_ASSERT(getCompositorNodeInstance().getCompositorWorkspaceInstance().getRendererRuntime().getContext(), isValid(mVarianceTextureResourceId), "Invalid compositor instance pass resource")

		// Depth to exponential variance
		delete mDepthToExponentialVarianceCompositorInstancePassCompute;
		mDepthToExponentialVarianceCompositorInstancePassCompute = nullptr;
		delete mDepthToExponentialVarianceCompositorResourcePassCompute;
		mDepthToExponentialVarianceCompositorResourcePassCompute = nullptr;

		// Horizontal blur
		delete mHorizontalBlurCompositorResourcePassCompute;
		mHorizontalBlurCompositorResourcePassCompute = nullptr;
		delete mHorizontalBlurCompositorInstancePassCompute;
		mHorizontalBlurCompositorInstancePassCompute = nullptr;

		// Vertical blur
		delete mVerticalBlurCompositorResourcePassCompute;
		mVerticalBlurCompositorResourcePassCompute = nullptr;
		delete mVerticalBlurCompositorInstancePassCompute;
		mVerticalBlurCompositorInstancePassCompute = nullptr;

		// Release the framebuffers and other renderer resources referenced by the framebuffers
		mDepthFramebufferPtr = nullptr;
		for (uint8_t cascadeIndex = 0; cascadeIndex < CompositorResourcePassShadowMap::MAXIMUM_NUMBER_OF_SHADOW_CASCADES; ++cascadeIndex)
		{
			mVarianceFramebufferPtr[cascadeIndex] = nullptr;
		}
		mIntermediateFramebufferPtr = nullptr;

		// Inform the texture resource manager that our render target texture is gone now
		TextureResourceManager& textureResourceManager = getCompositorNodeInstance().getCompositorWorkspaceInstance().getRendererRuntime().getTextureResourceManager();
		if (isValid(mDepthTextureResourceId))
		{
			textureResourceManager.destroyTextureResource(mDepthTextureResourceId);
			setInvalid(mDepthTextureResourceId);
		}
		textureResourceManager.destroyTextureResource(mVarianceTextureResourceId);
		setInvalid(mVarianceTextureResourceId);
		if (isValid(mIntermediateDepthBlurTextureResourceId))
		{
			textureResourceManager.destroyTextureResource(mIntermediateDepthBlurTextureResourceId);
			setInvalid(mIntermediateDepthBlurTextureResourceId);
		}
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

#include "stdafx.h"
#include "Resource/MaterialBlueprint/BufferManager/LightBufferManager.h"
#include "Resource/MaterialBlueprint/MaterialBlueprintResource.h"
#include "Resource/Texture/TextureResourceManager.h"
#include "Resource/Texture/TextureResource.h"
#include "Resource/Scene/SceneNode.h"
#include "Resource/Scene/SceneResource.h"
#include "Resource/Scene/Item/Light/LightSceneItem.h"
#include "Core/Math/Math.h"
#include "IRendererRuntime.h"

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
		// TODO(co) Add support for persistent mapped buffers. For now, the big picture has to be OK so first focus on that.
		static constexpr uint32_t LIGHT_DEFAULT_TEXTURE_BUFFER_NUMBER_OF_BYTES = 64 * 1024;	// 64 KiB
		// static constexpr uint32_t LIGHT_DEFAULT_TEXTURE_BUFFER_NUMBER_OF_BYTES = 512 * 1024;	// 512 KiB

		// TODO(co) Just for the clusters shading kickoff
		static constexpr uint32_t CLUSTER_X = 32;
		static constexpr uint32_t CLUSTER_Y = 8;
		static constexpr uint32_t CLUSTER_Z = 32;


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
	//[ Public static methods                                 ]
	//[-------------------------------------------------------]
	void LightBufferManager::getDefaultTextureAssetIds(AssetIds& assetIds)
	{
		// Define helper macro
		#define ADD_ASSET_ID(name) assetIds.push_back(ASSET_ID(name));

		// Add asset IDs
		ADD_ASSET_ID("Unrimp/Texture/DynamicByCode/LightClustersMap3D")

		// Undefine helper macro
		#undef ADD_ASSET_ID
	}


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	LightBufferManager::LightBufferManager(IRendererRuntime& rendererRuntime) :
		mRendererRuntime(rendererRuntime),
		mTextureBuffer(nullptr),
		mClusters3DTextureResourceId(getInvalid<TextureResourceId>()),
		mLightClustersAabbMinimum(-50.0f, -1.0f, -50.0f),	// TODO(co) Just for the clusters shading kickoff
		mLightClustersAabbMaximum( 50.0f, 40.0f,  50.0f),	// TODO(co) Just for the clusters shading kickoff
		mResourceGroup(nullptr)
	{
		// Create texture buffer instance
		mTextureScratchBuffer.resize(std::min(mRendererRuntime.getRenderer().getCapabilities().maximumTextureBufferSize, ::detail::LIGHT_DEFAULT_TEXTURE_BUFFER_NUMBER_OF_BYTES));
		mTextureBuffer = mRendererRuntime.getBufferManager().createTextureBuffer(static_cast<uint32_t>(mTextureScratchBuffer.size()), nullptr, Renderer::BufferFlag::SHADER_RESOURCE, Renderer::BufferUsage::DYNAMIC_DRAW);
		RENDERER_SET_RESOURCE_DEBUG_NAME(mTextureBuffer, "Light buffer manager")
		mTextureBuffer->addReference();

		{ // Create the clusters 3D texture resource
			// Create the renderer texture resource
			Renderer::ITexturePtr texturePtr(mRendererRuntime.getTextureManager().createTexture3D(::detail::CLUSTER_X, ::detail::CLUSTER_Y, ::detail::CLUSTER_Z, Renderer::TextureFormat::R32_UINT, nullptr, Renderer::TextureFlag::SHADER_RESOURCE, Renderer::TextureUsage::DYNAMIC));
			RENDERER_SET_RESOURCE_DEBUG_NAME(texturePtr, "Clusters 3D texture resource")

			// Create dynamic texture asset
			mClusters3DTextureResourceId = mRendererRuntime.getTextureResourceManager().createTextureResourceByAssetId(ASSET_ID("Unrimp/Texture/DynamicByCode/LightClustersMap3D"), *texturePtr);
		}
	}

	LightBufferManager::~LightBufferManager()
	{
		if (nullptr != mResourceGroup)
		{
			mResourceGroup->releaseReference();
		}
		mTextureBuffer->releaseReference();
		mRendererRuntime.getTextureResourceManager().destroyTextureResource(mClusters3DTextureResourceId);
	}

	void LightBufferManager::fillBuffer(const glm::dvec3& worldSpaceCameraPosition, SceneResource& sceneResource, Renderer::CommandBuffer& commandBuffer)
	{
		fillTextureBuffer(worldSpaceCameraPosition, sceneResource);
		fillClusters3DTexture(sceneResource, commandBuffer);
	}

	void LightBufferManager::fillGraphicsCommandBuffer(const MaterialBlueprintResource& materialBlueprintResource, Renderer::CommandBuffer& commandBuffer)
	{
		// Sanity check
		RENDERER_ASSERT(mRendererRuntime.getContext(), isInvalid(materialBlueprintResource.getComputeShaderBlueprintResourceId()), "Invalid compute shader blueprint resource ID")

		// Light texture buffer
		const MaterialBlueprintResource::TextureBuffer* lightTextureBuffer = materialBlueprintResource.getLightTextureBuffer();
		if (nullptr != lightTextureBuffer)
		{
			// TODO(co) We probably need to move the light buffer manager into the material blueprint resource
			// Create resource group instance, if needed
			if (nullptr == mResourceGroup)
			{
				// TODO(co) We probably should put the clusters 3D texture resource into the light buffer manager resource group as well
				// Renderer::IResource* resources[2] = { mTextureBuffer, mRendererRuntime.getTextureResourceManager().getById(mClusters3DTextureResourceId).getTexturePtr() };
				Renderer::IResource* resources[1] = { mTextureBuffer };
				mResourceGroup = materialBlueprintResource.getRootSignaturePtr()->createResourceGroup(lightTextureBuffer->rootParameterIndex, static_cast<uint32_t>(GLM_COUNTOF(resources)), resources);
				RENDERER_SET_RESOURCE_DEBUG_NAME(mResourceGroup, "Light buffer manager resource group")
				mResourceGroup->addReference();
			}

			// Set graphics resource group
			Renderer::Command::SetGraphicsResourceGroup::create(commandBuffer, lightTextureBuffer->rootParameterIndex, mResourceGroup);
		}
	}

	void LightBufferManager::fillComputeCommandBuffer(const MaterialBlueprintResource& materialBlueprintResource, Renderer::CommandBuffer& commandBuffer)
	{
		// Sanity check
		RENDERER_ASSERT(mRendererRuntime.getContext(), isValid(materialBlueprintResource.getComputeShaderBlueprintResourceId()), "Invalid compute shader blueprint resource ID")

		// Light texture buffer
		const MaterialBlueprintResource::TextureBuffer* lightTextureBuffer = materialBlueprintResource.getLightTextureBuffer();
		if (nullptr != lightTextureBuffer)
		{
			// TODO(co) We probably need to move the light buffer manager into the material blueprint resource
			// Create resource group instance, if needed
			if (nullptr == mResourceGroup)
			{
				// TODO(co) We probably should put the clusters 3D texture resource into the light buffer manager resource group as well
				// Renderer::IResource* resources[2] = { mTextureBuffer, mRendererRuntime.getTextureResourceManager().getById(mClusters3DTextureResourceId).getTexturePtr() };
				Renderer::IResource* resources[1] = { mTextureBuffer };
				mResourceGroup = materialBlueprintResource.getRootSignaturePtr()->createResourceGroup(lightTextureBuffer->rootParameterIndex, static_cast<uint32_t>(GLM_COUNTOF(resources)), resources);
				RENDERER_SET_RESOURCE_DEBUG_NAME(mResourceGroup, "Light buffer manager resource group")
				mResourceGroup->addReference();
			}

			// Set compute resource group
			Renderer::Command::SetComputeResourceGroup::create(commandBuffer, lightTextureBuffer->rootParameterIndex, mResourceGroup);
		}
	}

	glm::vec3 LightBufferManager::getLightClustersScale() const
	{
		return glm::vec3(static_cast<float>(::detail::CLUSTER_X), static_cast<float>(::detail::CLUSTER_Y), static_cast<float>(::detail::CLUSTER_Z)) / (mLightClustersAabbMaximum - mLightClustersAabbMinimum);
	}

	glm::vec3 LightBufferManager::getLightClustersBias() const
	{
		return glm::vec3(-static_cast<float>(::detail::CLUSTER_X), -static_cast<float>(::detail::CLUSTER_Y), -static_cast<float>(::detail::CLUSTER_Z)) / (mLightClustersAabbMaximum - mLightClustersAabbMinimum) * mLightClustersAabbMinimum;
	}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	void LightBufferManager::fillTextureBuffer(const glm::dvec3& worldSpaceCameraPosition, SceneResource& sceneResource)
	{
		// TODO(co) This is just a placeholder implementation until "RendererRuntime::LightBufferManager" is ready (containing e.g. reasonable optimizations)

		// Loop through all scene nodes and look for point and spot lights
		uint8_t* scratchBufferPointer = mTextureScratchBuffer.data();
		for (const SceneNode* sceneNode : sceneResource.getSceneNodes())
		{
			// Loop through all scene items attached to the current scene node
			for (ISceneItem* sceneItem : sceneNode->getAttachedSceneItems())
			{
				if (sceneItem->getSceneItemTypeId() == LightSceneItem::TYPE_ID)
				{
					LightSceneItem* lightSceneItem = static_cast<LightSceneItem*>(sceneItem);
					if (lightSceneItem->getLightType() != LightSceneItem::LightType::DIRECTIONAL && lightSceneItem->isVisible())
					{
						// Update the world space light position and the normalized view space light direction
						LightSceneItem::PackedShaderData& packedShaderData = lightSceneItem->mPackedShaderData;
						const Transform& transform = sceneNode->getGlobalTransform();
						packedShaderData.position  = transform.position - worldSpaceCameraPosition;	// Camera relative rendering: While we're using a 64 bit world space position in general, for relative positions 32 bit are sufficient
						packedShaderData.direction = transform.rotation * Math::VEC3_FORWARD;

						// Copy the light data into the texture scratch buffer
						memcpy(scratchBufferPointer, &packedShaderData, sizeof(LightSceneItem::PackedShaderData));
						scratchBufferPointer += sizeof(LightSceneItem::PackedShaderData);
					}
				}
			}
		}

		// Update the texture buffer by using our scratch buffer
		const uint32_t numberOfBytes = static_cast<uint32_t>(scratchBufferPointer - mTextureScratchBuffer.data());
		if (0 != numberOfBytes)
		{
			Renderer::MappedSubresource mappedSubresource;
			Renderer::IRenderer& renderer = mRendererRuntime.getRenderer();
			if (renderer.map(*mTextureBuffer, 0, Renderer::MapType::WRITE_DISCARD, 0, mappedSubresource))
			{
				memcpy(mappedSubresource.data, mTextureScratchBuffer.data(), numberOfBytes);
				renderer.unmap(*mTextureBuffer, 0);
			}
		}
	}

	void LightBufferManager::fillClusters3DTexture(SceneResource& sceneResource, Renderer::CommandBuffer&)
	{
		// Basing on the clustered shading demo from Emil Persson - http://humus.name/index.php?page=3D
		// "
		// We're using a 32bit integer format where each set bit enables the light of that index.
		// This supports up to 32 lights, which is enough for this demo, and probably for some games. It's possible to expand if more lights are needed,
		// for instance RGBA32_UINT for up to 128 lights in a single fetch, which is enough for many AAA titles. At some point, a list of indices becomes
		// more compact in practice, so if thousands of lights are needed, that's probably the way to go. Using a fixed bitmask has the advantage of fixed
		// size storage, simple addressing, and one indirection less in the inner loop.
		// "

		// TODO(co) This is just a placeholder implementation until "RendererRuntime::LightBufferManager" is ready
		//          - Containing e.g. reasonable optimizations
		//          - Processing on the GPU instead of CPU
		//          - Using a dynamic light clusters AABB
		uint32_t lights[::detail::CLUSTER_Z][::detail::CLUSTER_Y][::detail::CLUSTER_X] = {};
		const glm::vec3 scale = glm::vec3(static_cast<float>(::detail::CLUSTER_X), static_cast<float>(::detail::CLUSTER_Y), static_cast<float>(::detail::CLUSTER_Z)) / (mLightClustersAabbMaximum - mLightClustersAabbMinimum);
		const glm::vec3 inverseScale = 1.0f / scale;

		// Loop through all scene nodes and look for point and spot lights
		uint32_t currentLightIndex = 0;
		for (const SceneNode* sceneNode : sceneResource.getSceneNodes())
		{
			// Loop through all scene items attached to the current scene node
			for (ISceneItem* sceneItem : sceneNode->getAttachedSceneItems())
			{
				RENDERER_ASSERT(mRendererRuntime.getContext(), currentLightIndex < 32, "The current light index is out-of-bounds")
				if (sceneItem->getSceneItemTypeId() == LightSceneItem::TYPE_ID && currentLightIndex < 32)
				{
					LightSceneItem* lightSceneItem = static_cast<LightSceneItem*>(sceneItem);
					if (lightSceneItem->getLightType() != LightSceneItem::LightType::DIRECTIONAL && lightSceneItem->isVisible())
					{
						const LightSceneItem::PackedShaderData& packedShaderData = lightSceneItem->mPackedShaderData;

						const glm::vec3 p = (packedShaderData.position - mLightClustersAabbMinimum);
						const glm::vec3 p_min = (p - packedShaderData.radius) * scale;
						const glm::vec3 p_max = (p + packedShaderData.radius) * scale;

						// Cluster for the center of the light
						const int px = static_cast<int>(std::floor(p.x * scale.x));
						const int py = static_cast<int>(std::floor(p.y * scale.y));
						const int pz = static_cast<int>(std::floor(p.z * scale.z));

						// Cluster bounds for the light
						const int x0 = std::max(static_cast<int>(std::floor(p_min.x)), 0);
						const int x1 = std::min(static_cast<int>(std::ceil(p_max.x)), static_cast<int>(::detail::CLUSTER_X));
						const int y0 = std::max(static_cast<int>(std::floor(p_min.y)), 0);
						const int y1 = std::min(static_cast<int>(std::ceil(p_max.y)), static_cast<int>(::detail::CLUSTER_Y));
						const int z0 = std::max(static_cast<int>(std::floor(p_min.z)), 0);
						const int z1 = std::min(static_cast<int>(std::ceil(p_max.z)), static_cast<int>(::detail::CLUSTER_Z));

						const float squaredRadius = packedShaderData.radius * packedShaderData.radius;
						const int mask = (1 << currentLightIndex);

						// Do AABB <-> sphere tests to figure out which clusters are actually intersected by the light
						for (int z = z0; z < z1; ++z)
						{
							float dz = (pz == z) ? 0.0f : mLightClustersAabbMinimum.z + ((pz < z) ? z : z + 1) * inverseScale.z - packedShaderData.position.z;
							dz *= dz;

							for (int y = y0; y < y1; ++y)
							{
								float dy = (py == y) ? 0.0f : mLightClustersAabbMinimum.y + ((py < y) ? y : y + 1) * inverseScale.y - packedShaderData.position.y;
								dy *= dy;
								dy += dz;

								for (int x = x0; x < x1; ++x)
								{
									float dx = (px == x) ? 0.0f : mLightClustersAabbMinimum.x + ((px < x) ? x : x + 1) * inverseScale.x - packedShaderData.position.x;
									dx *= dx;
									dx += dy;

									if (dx < squaredRadius)
									{
										lights[z][y][x] |= mask;
									}
								}
							}
						}

						// Done, next light
						++currentLightIndex;
					}
				}
			}
		}

		// Upload the cluster data to a volume texture
		const Renderer::ITexturePtr& texturePtr = mRendererRuntime.getTextureResourceManager().getById(mClusters3DTextureResourceId).getTexturePtr();
		RENDERER_ASSERT(mRendererRuntime.getContext(), nullptr != texturePtr.getPointer(), "Invalid texture pointer")
		RENDERER_ASSERT(mRendererRuntime.getContext(), Renderer::ResourceType::TEXTURE_3D == texturePtr.getPointer()->getResourceType(), "Invalid texture resource type")
		Renderer::ITexture3D* texture3D = static_cast<Renderer::ITexture3D*>(texturePtr.getPointer());
		Renderer::MappedSubresource mappedSubresource;
		Renderer::IRenderer& renderer = mRendererRuntime.getRenderer();
		if (renderer.map(*texture3D, 0, Renderer::MapType::WRITE_DISCARD, 0, mappedSubresource))
		{
			memcpy(mappedSubresource.data, lights, ::detail::CLUSTER_X * ::detail::CLUSTER_Y * ::detail::CLUSTER_Z * sizeof(uint32_t));
			renderer.unmap(*texture3D, 0);
		}
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

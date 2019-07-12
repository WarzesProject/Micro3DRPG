#include "stdafx.h"
#include "Resource/RendererResourceManager.h"
#include "Core/Math/Math.h"

#include "Renderer.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	Renderer::IResourceGroup* RendererResourceManager::createResourceGroup(Renderer::IRootSignature& rootSignature, uint32_t rootParameterIndex, uint32_t numberOfResources, Renderer::IResource** resources, Renderer::ISamplerState** samplerStates)
	{
		// Create hash
		uint32_t hash = Math::calculateFNV1a32(reinterpret_cast<const uint8_t*>(&rootSignature), sizeof(Renderer::IRootSignature&));
		hash = Math::calculateFNV1a32(reinterpret_cast<const uint8_t*>(&rootParameterIndex), sizeof(uint32_t), hash);
		hash = Math::calculateFNV1a32(reinterpret_cast<const uint8_t*>(&numberOfResources), sizeof(uint32_t), hash);
		for (uint32_t i = 0; i < numberOfResources; ++i)
		{
			hash = Math::calculateFNV1a32(reinterpret_cast<const uint8_t*>(&resources[i]), sizeof(Renderer::IResource*), hash);
			if (nullptr != samplerStates && nullptr != samplerStates[i])
			{
				hash = Math::calculateFNV1a32(reinterpret_cast<const uint8_t*>(&samplerStates[i]), sizeof(Renderer::ISamplerState*), hash);
			}
			else
			{
				static const uint32_t NOTHING = 42;	// Not "static constexpr" by intent
				hash = Math::calculateFNV1a32(reinterpret_cast<const uint8_t*>(&NOTHING), sizeof(uint32_t), hash);
			}
		}
		ResourceGroups::const_iterator iterator = mResourceGroups.find(hash);
		if (mResourceGroups.cend() != iterator)
		{
			return iterator->second;
		}
		else
		{
			// Create renderer resource and add the managers reference
			Renderer::IResourceGroup* resourceGroup = rootSignature.createResourceGroup(rootParameterIndex, numberOfResources, resources, samplerStates);
			resourceGroup->addReference();
			mResourceGroups.emplace(hash, resourceGroup);
			return resourceGroup;
		}
	}

	void RendererResourceManager::garbageCollection()
	{
		// TODO(co) "RendererRuntime::RendererResourceManager": From time to time, look for orphaned renderer resources and free them. Currently a trivial approach is used which might cause hiccups. For example distribute the traversal over time.
		++mGarbageCollectionCounter;
		if (mGarbageCollectionCounter > 100)
		{
			ResourceGroups::iterator iterator = mResourceGroups.begin();
			while (iterator != mResourceGroups.end())
			{
				if (iterator->second->getRefCount() == 1)
				{
					iterator->second->releaseReference();
					iterator = mResourceGroups.erase(iterator);
				}
				else
				{
					++iterator;
				}
			}
			mGarbageCollectionCounter = 0;
		}
	}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	RendererResourceManager::~RendererResourceManager()
	{
		// Release manager renderer resource references
		for (auto& pair : mResourceGroups)
		{
			pair.second->releaseReference();
		}
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

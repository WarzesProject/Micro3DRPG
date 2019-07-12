#include "stdafx.h"
#include "Resource/IResourceListener.h"
#include "Resource/IResourceManager.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	void IResourceListener::disconnectFromResourceById(ResourceId resourceId)
	{
		assert(isValid(resourceId));
		ResourceConnections resourceConnectionsToDisconnect;
		resourceConnectionsToDisconnect.reserve(mResourceConnections.size());
		for (const ResourceConnection& resourceConnection : mResourceConnections)
		{
			if (resourceConnection.resourceId == resourceId)
			{
				resourceConnectionsToDisconnect.push_back(resourceConnection);
			}
		}
		for (const ResourceConnection& resourceConnection : resourceConnectionsToDisconnect)
		{
			resourceConnection.resourceManager->getResourceByResourceId(resourceConnection.resourceId).disconnectResourceListener(*this);
		}
	}

	void IResourceListener::disconnectFromAllResources()
	{
		// Disconnect all resource connections
		while (!mResourceConnections.empty())
		{
			const ResourceConnection& resourceConnection = mResourceConnections[0];
			resourceConnection.resourceManager->getResourceByResourceId(resourceConnection.resourceId).disconnectResourceListener(*this);
		}
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

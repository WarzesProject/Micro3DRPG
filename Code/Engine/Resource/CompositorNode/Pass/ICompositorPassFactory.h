#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Core/StringId.h"


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class CompositorTarget;
	class CompositorNodeInstance;
	class ICompositorResourcePass;
	class ICompositorInstancePass;
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
	class ICompositorPassFactory
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class CompositorWorkspaceInstance;	// Needs to create compositor pass instances // TODO(co) Remove this
		friend class CompositorTarget;				// Needs to create compositor pass instances


	//[-------------------------------------------------------]
	//[ Protected virtual RendererRuntime::ICompositorPassFactory methods ]
	//[-------------------------------------------------------]
	protected:
		[[nodiscard]] virtual ICompositorResourcePass* createCompositorResourcePass(const CompositorTarget& compositorTarget, CompositorPassTypeId compositorPassTypeId) const = 0;
		[[nodiscard]] virtual ICompositorInstancePass* createCompositorInstancePass(const ICompositorResourcePass& compositorResourcePass, const CompositorNodeInstance& compositorNodeInstance) const = 0;


	//[-------------------------------------------------------]
	//[ Protected methods                                     ]
	//[-------------------------------------------------------]
	protected:
		inline ICompositorPassFactory()
		{
			// Nothing here
		}

		inline virtual ~ICompositorPassFactory()
		{
			// Nothing here
		}

		explicit ICompositorPassFactory(const ICompositorPassFactory&) = delete;
		ICompositorPassFactory& operator=(const ICompositorPassFactory&) = delete;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

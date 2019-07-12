#include "stdafx.h"
#include "Resource/CompositorNode/Pass/DebugGui/CompositorInstancePassDebugGui.h"
#include "Resource/CompositorNode/Pass/DebugGui/CompositorResourcePassDebugGui.h"
#include "Resource/CompositorNode/CompositorNodeInstance.h"
#include "Resource/CompositorWorkspace/CompositorContextData.h"
#include "Resource/CompositorWorkspace/CompositorWorkspaceInstance.h"
#include "DebugGui/DebugGuiManager.h"
#include "Core/IProfiler.h"
#include "IRendererRuntime.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Protected virtual RendererRuntime::ICompositorInstancePass methods ]
	//[-------------------------------------------------------]
	void CompositorInstancePassDebugGui::onFillCommandBuffer([[maybe_unused]] const Renderer::IRenderTarget* renderTarget, [[maybe_unused]] const CompositorContextData& compositorContextData, [[maybe_unused]] Renderer::CommandBuffer& commandBuffer)
	{
		// Sanity check
		RENDERER_ASSERT(getCompositorNodeInstance().getCompositorWorkspaceInstance().getRendererRuntime().getContext(), nullptr != renderTarget, "The debug GUI compositor instance pass needs a valid render target")

		#ifdef RENDERER_RUNTIME_IMGUI
			// Combined scoped profiler CPU and GPU sample as well as renderer debug event command
			const IRendererRuntime& rendererRuntime = getCompositorNodeInstance().getCompositorWorkspaceInstance().getRendererRuntime();
			RENDERER_SCOPED_PROFILER_EVENT_DYNAMIC(rendererRuntime.getContext(), commandBuffer, getCompositorResourcePass().getDebugName())

			// Fill command buffer
			DebugGuiManager& debugGuiManager = rendererRuntime.getDebugGuiManager();
			RenderableManager::Renderables& renderables = mRenderableManager.getRenderables();
			compositorContextData.resetCurrentlyBoundMaterialBlueprintResource();
			if (renderables.empty())
			{
				// Fill command buffer using fixed build in renderer configuration resources
				debugGuiManager.fillGraphicsCommandBufferUsingFixedBuildInRendererConfiguration(commandBuffer);
			}
			else
			{
				// Fill command buffer, this sets the material resource blueprint
				{
					const Renderer::IVertexArrayPtr& vertexArrayPtr = debugGuiManager.getFillVertexArrayPtr();
					if (vertexArrayPtr != renderables[0].getVertexArrayPtr())
					{
						renderables[0].setVertexArrayPtr(vertexArrayPtr);
					}
				}
				mRenderQueue.addRenderablesFromRenderableManager(mRenderableManager, static_cast<const CompositorResourcePassDebugGui&>(getCompositorResourcePass()).getMaterialTechniqueId(), compositorContextData);
				if (mRenderQueue.getNumberOfDrawCalls() > 0)
				{
					mRenderQueue.fillGraphicsCommandBuffer(*renderTarget, compositorContextData, commandBuffer);

					// Fill command buffer using custom graphics material blueprint resource
					if (nullptr != compositorContextData.getCurrentlyBoundMaterialBlueprintResource())
					{
						debugGuiManager.fillGraphicsCommandBuffer(commandBuffer);
					}
				}
			}
		#else
			RENDERER_ASSERT(getCompositorNodeInstance().getCompositorWorkspaceInstance().getRendererRuntime().getContext(), false, "ImGui support is disabled")
		#endif
	}


	//[-------------------------------------------------------]
	//[ Protected virtual RendererRuntime::CompositorInstancePassCompute methods ]
	//[-------------------------------------------------------]
	void CompositorInstancePassDebugGui::createMaterialResource(MaterialResourceId parentMaterialResourceId)
	{
		// Call the base implementation
		CompositorInstancePassCompute::createMaterialResource(parentMaterialResourceId);

		// Inside this compositor pass implementation, the renderable only exists to set the material blueprint
		mRenderableManager.getRenderables()[0].setNumberOfIndices(0);
	}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	CompositorInstancePassDebugGui::CompositorInstancePassDebugGui(const CompositorResourcePassDebugGui& compositorResourcePassDebugGui, const CompositorNodeInstance& compositorNodeInstance) :
		CompositorInstancePassCompute(compositorResourcePassDebugGui, compositorNodeInstance)
	{
		// Inside this compositor pass implementation, the renderable only exists to set the material blueprint
		RenderableManager::Renderables& renderables = mRenderableManager.getRenderables();
		if (!renderables.empty())
		{
			renderables[0].setNumberOfIndices(0);
		}
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "DebugGui/DebugGuiManager.h"
#include "Core/Platform/WindowsHeader.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	class DebugGuiManagerWindows final : public DebugGuiManager
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class RendererRuntimeImpl;


	//[-------------------------------------------------------]
	//[ Public static methods                                 ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Microsoft Windows callback method to be called from the outside for input handling
		*/
		RENDERERRUNTIME_API_EXPORT static LRESULT wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


	//[-------------------------------------------------------]
	//[ Protected virtual RendererRuntime::DebugGuiManager methods ]
	//[-------------------------------------------------------]
	protected:
		virtual void initializeImGuiKeyMap() override;
		virtual void startup() override;
		virtual void onNewFrame(Renderer::IRenderTarget& renderTarget) override;


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		inline explicit DebugGuiManagerWindows(IRendererRuntime& rendererRuntime) :
			DebugGuiManager(rendererRuntime),
			mTime(0),
			mTicksPerSecond(0)
		{
			// Nothing here
		}

		inline virtual ~DebugGuiManagerWindows() override
		{
			// Nothing here
		}

		explicit DebugGuiManagerWindows(const DebugGuiManagerWindows&) = delete;
		DebugGuiManagerWindows& operator=(const DebugGuiManagerWindows&) = delete;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		INT64 mTime;
		INT64 mTicksPerSecond;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

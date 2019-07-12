#pragma once


#ifdef RENDERER_RUNTIME_GRAPHICS_DEBUGGER


	//[-------------------------------------------------------]
	//[ Includes                                              ]
	//[-------------------------------------------------------]
	#include "Renderer.h"


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
		*    Abstract graphics debugger interface
		*/
		class IGraphicsDebugger
		{


		//[-------------------------------------------------------]
		//[ Public methods                                        ]
		//[-------------------------------------------------------]
		public:
			[[nodiscard]] inline bool getCaptureNextFrame() const
			{
				return mCaptureNextFrame;
			}

			inline void captureNextFrame()
			{
				mCaptureNextFrame = true;
			}


		//[-------------------------------------------------------]
		//[ Public virtual RendererRuntime::IGraphicsDebugger methods ]
		//[-------------------------------------------------------]
		public:
			/**
			*  @brief
			*    Return whether or not the graphics debugger instance is properly initialized
			*
			*  @return
			*    "true" if the graphics debugger instance is properly initialized, else "false"
			*/
			[[nodiscard]] virtual bool isInitialized() const = 0;

			/**
			*  @brief
			*    Start frame capture
			*
			*  @param[in] nativeWindowHandle
			*    Native window handle
			*/
			virtual void startFrameCapture(Renderer::handle nativeWindowHandle) = 0;

			/**
			*  @brief
			*    End frame capture
			*
			*  @param[in] nativeWindowHandle
			*    Native window handle
			*/
			virtual void endFrameCapture(Renderer::handle nativeWindowHandle) = 0;


		//[-------------------------------------------------------]
		//[ Protected methods                                     ]
		//[-------------------------------------------------------]
		protected:
			inline IGraphicsDebugger() :
				mCaptureNextFrame(false)
			{
				// Nothing here
			}

			inline virtual ~IGraphicsDebugger()
			{
				// Nothing here
			}

			explicit IGraphicsDebugger(const IGraphicsDebugger&) = delete;
			IGraphicsDebugger& operator=(const IGraphicsDebugger&) = delete;

			inline void resetCaptureNextFrame()
			{
				mCaptureNextFrame = false;
			}


		//[-------------------------------------------------------]
		//[ Private data                                          ]
		//[-------------------------------------------------------]
		private:
			bool mCaptureNextFrame;


		};


	//[-------------------------------------------------------]
	//[ Namespace                                             ]
	//[-------------------------------------------------------]
	} // RendererRuntime


#endif

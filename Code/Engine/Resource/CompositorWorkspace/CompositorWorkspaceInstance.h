#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Core/StringId.h"
#include "Resource/IResourceListener.h"

#include "Renderer.h"


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class LightSceneItem;
	class CameraSceneItem;
	class IRendererRuntime;
	class RenderableManager;
	class CompositorNodeInstance;
	class ICompositorInstancePass;
	class CompositorInstancePassShadowMap;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef StringId AssetId;						///< Asset identifier, internally just a POD "uint32_t", string ID scheme is "<project name>/<asset directory>/<asset name>"
	typedef uint32_t CompositorWorkspaceResourceId;	///< POD compositor workspace resource identifier
	typedef StringId CompositorPassTypeId;			///< Compositor pass type identifier, internally just a POD "uint32_t"


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	/**
	*  @brief
	*    Compositor workspace instance
	*
	*  @remarks
	*    Compositors can get quite complex with a lot of individual compositor passes which several of them rendering portions of the scene.
	*    We really only want to perform expensive culling once for a compositor workspace frame rendering. Some renderable managers might never
	*    get rendered because none of the renderables is inside a render queue index range touched by the compositor passes. As a result,
	*    an compositor workspace instance keeps a list of render queue index ranges covered by the compositor instance passes. Before compositor
	*    instance passes are executed, a culling step is performed gathering all renderable managers which should currently be taken into account
	*    during rendering. The result of this culling step is that each render queue index range has renderable managers to consider assigned to them.
	*    Executed compositor instances passes only access this prepared render queue index information to fill their render queues.
	*/
	class CompositorWorkspaceInstance final : protected IResourceListener
	{


	//[-------------------------------------------------------]
	//[ Public definitions                                    ]
	//[-------------------------------------------------------]
	public:
		typedef std::vector<RenderableManager*> RenderableManagers;
		typedef std::vector<CompositorNodeInstance*> CompositorNodeInstances;

		struct RenderQueueIndexRange final
		{
			uint8_t			   minimumRenderQueueIndex;	///< Fixed during runtime
			uint8_t			   maximumRenderQueueIndex;	///< Fixed during runtime
			RenderableManagers renderableManagers;		///< Dynamic during runtime

			inline RenderQueueIndexRange(uint8_t _minimumRenderQueueIndex, uint8_t _maximumRenderQueueIndex) :
				minimumRenderQueueIndex(_minimumRenderQueueIndex),
				maximumRenderQueueIndex(_maximumRenderQueueIndex)
			{}
		};
		typedef std::vector<RenderQueueIndexRange> RenderQueueIndexRanges;


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		RENDERERRUNTIME_API_EXPORT CompositorWorkspaceInstance(IRendererRuntime& rendererRuntime, AssetId compositorWorkspaceAssetId);
		RENDERERRUNTIME_API_EXPORT virtual ~CompositorWorkspaceInstance() override;

		[[nodiscard]] inline const IRendererRuntime& getRendererRuntime() const
		{
			return mRendererRuntime;
		}

		[[nodiscard]] inline uint8_t getNumberOfMultisamples() const
		{
			return mNumberOfMultisamples;
		}

		RENDERERRUNTIME_API_EXPORT void setNumberOfMultisamples(uint8_t numberOfMultisamples);	// The number of multisamples per pixel (valid values: 1, 2, 4, 8); Changes are considered to be expensive since internal renderer resources might need to be updated when rendering the next time

		[[nodiscard]] inline float getResolutionScale() const
		{
			return mResolutionScale;
		}

		inline void setResolutionScale(float resolutionScale)	// Changes are considered to be expensive since internal renderer resources might need to be updated when rendering the next time
		{
			mResolutionScale = resolutionScale;
		}

		[[nodiscard]] inline const RenderQueueIndexRanges& getRenderQueueIndexRanges() const	// Renderable manager pointers are only considered to be safe directly after the "RendererRuntime::CompositorWorkspaceInstance::execute()" call
		{
			return mRenderQueueIndexRanges;
		}

		[[nodiscard]] RENDERERRUNTIME_API_EXPORT const RenderQueueIndexRange* getRenderQueueIndexRangeByRenderQueueIndex(uint8_t renderQueueIndex) const;	// Can be a null pointer, don't destroy the instance
		[[nodiscard]] RENDERERRUNTIME_API_EXPORT const ICompositorInstancePass* getFirstCompositorInstancePassByCompositorPassTypeId(CompositorPassTypeId compositorPassTypeId) const;
		RENDERERRUNTIME_API_EXPORT void executeVr(Renderer::IRenderTarget& renderTarget, CameraSceneItem* cameraSceneItem, const LightSceneItem* lightSceneItem);	// If "RendererRuntime::IVrManager::isRunning()" is true, virtual reality rendering is used, don't use this method if you want to render e.g. into a texture for other purposes
		RENDERERRUNTIME_API_EXPORT void execute(Renderer::IRenderTarget& renderTarget, const CameraSceneItem* cameraSceneItem, const LightSceneItem* lightSceneItem, bool singlePassStereoInstancing = false);

		[[nodiscard]] inline Renderer::IRenderTarget* getExecutionRenderTarget() const	// Only valid during compositor workspace instance execution
		{
			return mExecutionRenderTarget;
		}

		[[nodiscard]] inline const CompositorNodeInstances& getSequentialCompositorNodeInstances() const
		{
			return mSequentialCompositorNodeInstances;
		}

		[[nodiscard]] inline const Renderer::CommandBuffer& getCommandBuffer() const
		{
			return mCommandBuffer;
		}

		#ifdef RENDERER_STATISTICS
			[[nodiscard]] inline const Renderer::PipelineStatisticsQueryResult& getPipelineStatisticsQueryResult() const
			{
				return mPipelineStatisticsQueryResult;
			}
		#endif


	//[-------------------------------------------------------]
	//[ Protected virtual RendererRuntime::IResourceListener methods ]
	//[-------------------------------------------------------]
	protected:
		virtual void onLoadingStateChange(const IResource& resource) override;


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		CompositorWorkspaceInstance() = delete;
		explicit CompositorWorkspaceInstance(const CompositorWorkspaceInstance&) = delete;
		CompositorWorkspaceInstance& operator=(const CompositorWorkspaceInstance&) = delete;
		void destroySequentialCompositorNodeInstances();
		void createFramebuffersAndRenderTargetTextures(const Renderer::IRenderTarget& mainRenderTarget);
		void destroyFramebuffersAndRenderTargetTextures(bool clearManagers = false);
		void clearRenderQueueIndexRangesRenderableManagers();


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		IRendererRuntime&				 mRendererRuntime;
		uint8_t							 mNumberOfMultisamples;
		uint8_t							 mCurrentlyUsedNumberOfMultisamples;
		float							 mResolutionScale;
		uint32_t						 mRenderTargetWidth;
		uint32_t						 mRenderTargetHeight;
		Renderer::IRenderTarget*		 mExecutionRenderTarget;				///< Only valid during compositor workspace instance execution
		CompositorWorkspaceResourceId	 mCompositorWorkspaceResourceId;
		CompositorNodeInstances			 mSequentialCompositorNodeInstances;	///< We're responsible to destroy the compositor node instances if we no longer need them
		bool							 mFramebufferManagerInitialized;
		RenderQueueIndexRanges			 mRenderQueueIndexRanges;				///< The render queue index ranges layout is fixed during runtime
		Renderer::CommandBuffer			 mCommandBuffer;						///< Command buffer
		CompositorInstancePassShadowMap* mCompositorInstancePassShadowMap;		///< Can be a null pointer, don't destroy the instance
		#ifdef RENDERER_STATISTICS
			Renderer::IQueryPoolPtr					mPipelineStatisticsQueryPoolPtr;				///< Double buffered asynchronous pipeline statistics query pool, can be a null pointer
			uint32_t								mPreviousCurrentPipelineStatisticsQueryIndex;	///< Can be "RendererRuntime::getInvalid<uint32_t>()"
			uint32_t								mCurrentPipelineStatisticsQueryIndex;			///< Toggles between 0 or 1
			Renderer::PipelineStatisticsQueryResult mPipelineStatisticsQueryResult;					///< Due to double buffered asynchronous pipeline statistics query pool, this is the pipeline statistics query result of the previous frame
		#endif


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

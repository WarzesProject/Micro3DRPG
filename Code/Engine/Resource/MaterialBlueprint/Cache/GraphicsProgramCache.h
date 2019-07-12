#pragma once


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
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef uint32_t GraphicsProgramCacheId;	///< Graphics program cache identifier, result of hashing the shader cache IDs of the referenced shaders


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	class GraphicsProgramCache final
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class GraphicsProgramCacheManager;	// Is creating and managing graphics program cache instances
		friend class GraphicsPipelineStateCompiler;	// Is creating graphics program cache instances


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Return the graphics program cache ID
		*
		*  @return
		*    The graphics program cache ID
		*/
		[[nodiscard]] inline GraphicsProgramCacheId getGraphicsProgramCacheId() const
		{
			return mGraphicsProgramCacheId;
		}

		/**
		*  @brief
		*    Return graphics program
		*
		*  @return
		*    The graphics program
		*/
		[[nodiscard]] inline const Renderer::IGraphicsProgramPtr& getGraphicsProgramPtr() const
		{
			return mGraphicsProgramPtr;
		}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		inline GraphicsProgramCache(GraphicsProgramCacheId graphicsProgramCacheId, Renderer::IGraphicsProgram& graphicsProgram) :
			mGraphicsProgramCacheId(graphicsProgramCacheId),
			mGraphicsProgramPtr(&graphicsProgram)
		{
			// Nothing here
		}

		inline ~GraphicsProgramCache()
		{
			// Nothing here
		}

		explicit GraphicsProgramCache(const GraphicsProgramCache&) = delete;
		GraphicsProgramCache& operator=(const GraphicsProgramCache&) = delete;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		GraphicsProgramCacheId		  mGraphicsProgramCacheId;
		Renderer::IGraphicsProgramPtr mGraphicsProgramPtr;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

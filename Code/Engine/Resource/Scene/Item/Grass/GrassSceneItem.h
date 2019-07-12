#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Resource/Scene/Item/MaterialSceneItem.h"


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
	*    Grass scene item
	*
	*  @todo
	*    - TODO(co) Work-in-progress
	*/
	class GrassSceneItem final : public MaterialSceneItem
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class SceneFactory;	// Needs to be able to create scene item instances


	//[-------------------------------------------------------]
	//[ Public definitions                                    ]
	//[-------------------------------------------------------]
	public:
		static constexpr uint32_t TYPE_ID = STRING_ID("GrassSceneItem");
		struct GrassDataStruct
		{
			float PositionSize[4];	// Object space grass xyz-position, w = grass size
			float ColorRotation[4];	// Linear RGB grass color and rotation in radians
		};


	//[-------------------------------------------------------]
	//[ Public RendererRuntime::ISceneItem methods            ]
	//[-------------------------------------------------------]
	public:
		[[nodiscard]] inline virtual SceneItemTypeId getSceneItemTypeId() const override
		{
			return TYPE_ID;
		}


	//[-------------------------------------------------------]
	//[ Protected virtual RendererRuntime::MaterialSceneItem methods ]
	//[-------------------------------------------------------]
	protected:
		virtual void initialize() override;
		virtual void onMaterialResourceCreated() override;


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		explicit GrassSceneItem(SceneResource& sceneResource);

		inline virtual ~GrassSceneItem() override
		{
			// Nothing here
		}

		explicit GrassSceneItem(const GrassSceneItem&) = delete;
		GrassSceneItem& operator=(const GrassSceneItem&) = delete;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		uint32_t					   mMaximumNumberOfGrass;	///< Maximum number of grass
		Renderer::IStructuredBufferPtr mStructuredBufferPtr;	///< Structured buffer the data of the individual grass ("RendererRuntime::GrassSceneItem::GrassDataStruct")
		Renderer::IIndirectBufferPtr   mIndirectBufferPtr;		///< Indirect buffer holding data related to the current grass "Renderer::DrawArguments" draw call


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

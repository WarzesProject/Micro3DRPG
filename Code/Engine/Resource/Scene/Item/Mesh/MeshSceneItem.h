#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Resource/Scene/Item/ISceneItem.h"
#include "Resource/IResourceListener.h"
#include "RenderQueue/RenderableManager.h"
#include "Core/GetInvalid.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef StringId AssetId;			///< Asset identifier, internally just a POD "uint32_t", string ID scheme is "<project name>/<asset directory>/<asset name>"
	typedef uint32_t MeshResourceId;	///< POD mesh resource identifier


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	class MeshSceneItem : public ISceneItem, public IResourceListener
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class SceneFactory;	// Needs to be able to create scene item instances


	//[-------------------------------------------------------]
	//[ Public definitions                                    ]
	//[-------------------------------------------------------]
	public:
		static constexpr uint32_t TYPE_ID = STRING_ID("MeshSceneItem");


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		[[nodiscard]] inline MeshResourceId getMeshResourceId() const
		{
			return mMeshResourceId;
		}

		RENDERERRUNTIME_API_EXPORT void setMeshResourceId(MeshResourceId meshResourceId);
		RENDERERRUNTIME_API_EXPORT void setMeshResourceIdByAssetId(AssetId meshAssetId);

		[[nodiscard]] inline uint32_t getNumberOfSubMeshes() const
		{
			return static_cast<uint32_t>(mRenderableManager.getRenderables().size());
		}

		[[nodiscard]] inline MaterialResourceId getMaterialResourceIdOfSubMesh(uint32_t subMeshIndex) const
		{
			RENDERER_ASSERT(getContext(), subMeshIndex < mRenderableManager.getRenderables().size(), "Invalid sub mesh index")
			return mRenderableManager.getRenderables()[subMeshIndex].getMaterialResourceId();
		}

		RENDERERRUNTIME_API_EXPORT void setMaterialResourceIdOfSubMesh(uint32_t subMeshIndex, MaterialResourceId materialResourceId);
		RENDERERRUNTIME_API_EXPORT void setMaterialResourceIdOfAllSubMeshes(MaterialResourceId materialResourceId);


	//[-------------------------------------------------------]
	//[ Public RendererRuntime::ISceneItem methods            ]
	//[-------------------------------------------------------]
	public:
		[[nodiscard]] inline virtual SceneItemTypeId getSceneItemTypeId() const override
		{
			return TYPE_ID;
		}

		virtual void deserialize(uint32_t numberOfBytes, const uint8_t* data) override;
		virtual void onAttachedToSceneNode(SceneNode& sceneNode) override;

		inline virtual void onDetachedFromSceneNode(SceneNode& sceneNode) override
		{
			mRenderableManager.setTransform(nullptr);

			// Call the base implementation
			ISceneItem::onDetachedFromSceneNode(sceneNode);
		}

		inline virtual void setVisible(bool visible) override
		{
			mRenderableManager.setVisible(visible);
		}

		[[nodiscard]] inline virtual const RenderableManager* getRenderableManager() const override
		{
			return &mRenderableManager;
		}


	//[-------------------------------------------------------]
	//[ Protected methods                                     ]
	//[-------------------------------------------------------]
	protected:
		inline explicit MeshSceneItem(SceneResource& sceneResource) :
			ISceneItem(sceneResource),
			mMeshResourceId(getInvalid<MeshResourceId>())
		{
			// Nothing here
		}

		inline virtual ~MeshSceneItem() override
		{
			// Nothing here
		}

		explicit MeshSceneItem(const MeshSceneItem&) = delete;
		MeshSceneItem& operator=(const MeshSceneItem&) = delete;


	//[-------------------------------------------------------]
	//[ Protected virtual RendererRuntime::IResourceListener methods ]
	//[-------------------------------------------------------]
	protected:
		virtual void onLoadingStateChange(const IResource& resource) override;


	//[-------------------------------------------------------]
	//[ Private definitions                                   ]
	//[-------------------------------------------------------]
	private:
		typedef std::vector<AssetId> SubMeshMaterialAssetIds;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		MeshResourceId			mMeshResourceId;			///< Mesh resource ID, can be set to invalid value
		SubMeshMaterialAssetIds	mSubMeshMaterialAssetIds;	///< Sub-mesh material asset IDs received during deserialization
		RenderableManager		mRenderableManager;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

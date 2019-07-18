#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Runtime/FirstScene/IController.h"


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class LightSceneItem;
	class IRendererRuntime;
}


//[-------------------------------------------------------]
//[ Classes                                               ]
//[-------------------------------------------------------]
/**
*  @brief
*    Virtual reality controller
*
*  @remarks
*    - Two controllers required
*    - The first VR controller is used for teleporting. A green light indicates the position one will end up. When pressing the trigger button one teleports to this position.
*    - The GUI is placed over the second VR controller
*/
class VrController final : public IController
{


//[-------------------------------------------------------]
//[ Public methods                                        ]
//[-------------------------------------------------------]
public:
	/**
	*  @brief
	*    Constructor
	*
	*  @param[in] cameraSceneItem
	*    Camera scene item to control, instance must stay valid as long as this controller instance exists
	*/
	explicit VrController(RendererRuntime::CameraSceneItem& cameraSceneItem);

	/**
	*  @brief
	*    Destructor
	*/
	virtual ~VrController() override;

	[[nodiscard]] const RendererRuntime::LightSceneItem& getTeleportIndicationLightSceneItemSafe() const;


//[-------------------------------------------------------]
//[ Public virtual IController methods                    ]
//[-------------------------------------------------------]
public:
	virtual void onUpdate(float pastSecondsSinceLastFrame, bool inputEnabled) override;


//[-------------------------------------------------------]
//[ Private methods                                       ]
//[-------------------------------------------------------]
private:
	explicit VrController(const VrController&) = delete;
	VrController& operator=(const VrController&) = delete;


//[-------------------------------------------------------]
//[ Private data                                          ]
//[-------------------------------------------------------]
private:
	const RendererRuntime::IRendererRuntime& mRendererRuntime;
	RendererRuntime::LightSceneItem*		 mTeleportIndicationLightSceneItem;


};

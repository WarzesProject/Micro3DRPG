#include "stdafx.h"
#include "Runtime/FirstScene/VrController.h"

#include <IRendererRuntime.h>
#include <Core/Math/Math.h>
#include <Core/Math/Transform.h>
#include <Core/Math/EulerAngles.h>
#include <Resource/Scene/SceneNode.h>
#include <Resource/Scene/SceneResource.h>
#include <Resource/Scene/Item/Mesh/MeshSceneItem.h>
#include <Resource/Scene/Item/Light/LightSceneItem.h>
#include <Resource/Scene/Item/Camera/CameraSceneItem.h>
#include <Resource/MaterialBlueprint/MaterialBlueprintResourceManager.h>
#include <Resource/MaterialBlueprint/Listener/MaterialBlueprintResourceListener.h>
#include <Vr/OpenVR/VrManagerOpenVR.h>
#include <Vr/OpenVR/IVrManagerOpenVRListener.h>

#ifdef RENDERER_RUNTIME_IMGUI
	#include <imgui/imgui.h>
#endif

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4464)	// warning C4464: relative include path contains '..'
	PRAGMA_WARNING_DISABLE_MSVC(4324)	// warning C4324: '<x>': structure was padded due to alignment specifier
	#include <glm/gtx/intersect.hpp>
	#include <glm/gtx/matrix_decompose.hpp>
	#include <glm/gtc/type_ptr.hpp>
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
namespace
{
	namespace detail
	{


		//[-------------------------------------------------------]
		//[ Global definitions                                    ]
		//[-------------------------------------------------------]
		#define DEFINE_CONSTANT(name) static constexpr uint32_t name = STRING_ID(#name);
			// Pass
			DEFINE_CONSTANT(IMGUI_OBJECT_SPACE_TO_CLIP_SPACE_MATRIX)
		#undef DEFINE_CONSTANT
		static constexpr uint32_t FIRST_CONTROLLER_INDEX  = 0;
		static constexpr uint32_t SECOND_CONTROLLER_INDEX = 1;


		//[-------------------------------------------------------]
		//[ Classes                                               ]
		//[-------------------------------------------------------]
		/**
		*  @brief
		*    Virtual reality manager OpenVR listener
		*
		*  @todo
		*    - TODO(co) Support the dynamic adding and removal of VR controllers (index updates)
		*/
		class VrManagerOpenVRListener : public RendererRuntime::IVrManagerOpenVRListener
		{


		//[-------------------------------------------------------]
		//[ Public methods                                        ]
		//[-------------------------------------------------------]
		public:
			VrManagerOpenVRListener() :
				mVrManagerOpenVR(nullptr),
				mVrController(nullptr),
				mNumberOfVrControllers(0)
			{
				for (uint32_t i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i)
				{
					mVrControllerTrackedDeviceIndices[i] = RendererRuntime::getInvalid<vr::TrackedDeviceIndex_t>();
				}
			}

			inline virtual ~VrManagerOpenVRListener() override
			{
				// Nothing here
			}

			inline void setVrManagerOpenVR(const RendererRuntime::VrManagerOpenVR& vrManagerOpenVR, VrController& vrController)
			{
				mVrManagerOpenVR = &vrManagerOpenVR;
				mVrController = &vrController;
			}

			[[nodiscard]] inline uint32_t getNumberOfVrControllers() const
			{
				return mNumberOfVrControllers;
			}

			[[nodiscard]] inline vr::TrackedDeviceIndex_t getVrControllerTrackedDeviceIndices(uint32_t vrControllerIndex) const
			{
				assert(vrControllerIndex < vr::k_unMaxTrackedDeviceCount);
				return mVrControllerTrackedDeviceIndices[vrControllerIndex];
			}


		//[-------------------------------------------------------]
		//[ Private virtual RendererRuntime::IVrManagerOpenVRListener methods ]
		//[-------------------------------------------------------]
		private:
			virtual void onVrEvent(const vr::VREvent_t& vrVrEvent) override
			{
				switch (vrVrEvent.eventType)
				{
					// Handle quiting the application from Steam
					case vr::VREvent_DriverRequestedQuit:
					case vr::VREvent_Quit:
						// TODO(co)
						NOP;
						break;

					case vr::VREvent_ButtonPress:
					{
						// The first VR controller is used for teleporting
						// -> A green light indicates the position one will end up
						// -> When pressing the trigger button one teleports to this position
						if (mNumberOfVrControllers > 0 && mVrControllerTrackedDeviceIndices[FIRST_CONTROLLER_INDEX] == vrVrEvent.trackedDeviceIndex && vrVrEvent.data.controller.button == vr::k_EButton_SteamVR_Trigger && mVrController->getTeleportIndicationLightSceneItemSafe().isVisible())
						{
							mVrController->getCameraSceneItem().getParentSceneNodeSafe().setPosition(mVrController->getTeleportIndicationLightSceneItemSafe().getParentSceneNodeSafe().getGlobalTransform().position);
						}
						break;
					}
				}
			}

			virtual void onSceneNodeCreated(vr::TrackedDeviceIndex_t trackedDeviceIndex, RendererRuntime::SceneResource& sceneResource, RendererRuntime::SceneNode& sceneNode) override
			{
				if (mVrManagerOpenVR->getVrSystem()->GetTrackedDeviceClass(trackedDeviceIndex) == vr::TrackedDeviceClass_Controller)
				{
					// Attach a light to controllers, this way they can be seen easier and it's possible to illuminate the scene by using the hands
					RendererRuntime::LightSceneItem* lightSceneItem = sceneResource.createSceneItem<RendererRuntime::LightSceneItem>(sceneNode);
					if (0 == mNumberOfVrControllers && nullptr != lightSceneItem)
					{
						// Spot light for the first VR controller
						lightSceneItem->setLightTypeAndRadius(RendererRuntime::LightSceneItem::LightType::SPOT, 5.0f);
						lightSceneItem->setColor(glm::vec3(10.0f, 10.0f, 10.0f));
						lightSceneItem->setInnerOuterAngle(glm::radians(20.0f), glm::radians(30.0f));
						lightSceneItem->setNearClipDistance(0.05f);
					}

					// Remember the VR controller tracked device index
					mVrControllerTrackedDeviceIndices[mNumberOfVrControllers] = trackedDeviceIndex;
					++mNumberOfVrControllers;
				}
			}


		//[-------------------------------------------------------]
		//[ Private methods                                       ]
		//[-------------------------------------------------------]
		private:
			explicit VrManagerOpenVRListener(const VrManagerOpenVRListener&) = delete;
			VrManagerOpenVRListener& operator=(const VrManagerOpenVRListener&) = delete;


		//[-------------------------------------------------------]
		//[ Private data                                          ]
		//[-------------------------------------------------------]
		private:
			const RendererRuntime::VrManagerOpenVR*	mVrManagerOpenVR;
			VrController*							mVrController;
			uint32_t								mNumberOfVrControllers;
			vr::TrackedDeviceIndex_t				mVrControllerTrackedDeviceIndices[vr::k_unMaxTrackedDeviceCount];


		};

		class MaterialBlueprintResourceListener : public RendererRuntime::MaterialBlueprintResourceListener
		{


		//[-------------------------------------------------------]
		//[ Public methods                                        ]
		//[-------------------------------------------------------]
		public:
			inline MaterialBlueprintResourceListener() :
				mVrManagerOpenVR(nullptr),
				mVrManagerOpenVRListener(nullptr),
				mVrController(nullptr)
			{
				// Nothing here
			}

			inline virtual ~MaterialBlueprintResourceListener() override
			{
				// Nothing here
			}

			inline void setVrManagerOpenVR(const RendererRuntime::VrManagerOpenVR& vrManagerOpenVR, const VrManagerOpenVRListener& vrManagerOpenVRListener, VrController& vrController)
			{
				mVrManagerOpenVR = &vrManagerOpenVR;
				mVrManagerOpenVRListener = &vrManagerOpenVRListener;
				mVrController = &vrController;
			}


		//[-------------------------------------------------------]
		//[ Private virtual RendererRuntime::IMaterialBlueprintResourceListener methods ]
		//[-------------------------------------------------------]
		private:
			[[nodiscard]] virtual bool fillPassValue(uint32_t referenceValue, uint8_t* buffer, uint32_t numberOfBytes) override
			{
				// The GUI is placed over the second VR controller
				#ifdef RENDERER_RUNTIME_IMGUI
					if (::detail::IMGUI_OBJECT_SPACE_TO_CLIP_SPACE_MATRIX == referenceValue && mVrManagerOpenVRListener->getNumberOfVrControllers() > SECOND_CONTROLLER_INDEX)
					{
						assert(sizeof(float) * 4 * 4 == numberOfBytes);
						const ImGuiIO& imGuiIo = ImGui::GetIO();
						const glm::quat rotationOffset = RendererRuntime::EulerAngles::eulerToQuaternion(glm::vec3(glm::degrees(0.0f), glm::degrees(180.0f), 0.0f));
						const glm::mat4 guiScaleMatrix = glm::scale(RendererRuntime::Math::MAT4_IDENTITY, glm::vec3(1.0f / imGuiIo.DisplaySize.x, 1.0f / imGuiIo.DisplaySize.y, 1.0f));
						const glm::mat4& devicePoseMatrix = mVrManagerOpenVR->getDevicePoseMatrix(mVrManagerOpenVRListener->getVrControllerTrackedDeviceIndices(SECOND_CONTROLLER_INDEX));
						// TODO(co) 64 bit support
						const glm::mat4& cameraPositionMatrix = glm::translate(RendererRuntime::Math::MAT4_IDENTITY, glm::vec3(-mVrController->getCameraSceneItem().getParentSceneNodeSafe().getGlobalTransform().position));
						const glm::mat4 objectSpaceToClipSpaceMatrix = getPassData().cameraRelativeWorldSpaceToClipSpaceMatrixReversedZ[0] * cameraPositionMatrix * devicePoseMatrix * glm::mat4_cast(rotationOffset) * guiScaleMatrix;
						memcpy(buffer, glm::value_ptr(objectSpaceToClipSpaceMatrix), numberOfBytes);

						// Value filled
						return true;
					}
					else
				#endif
				{
					// Call the base implementation
					return RendererRuntime::MaterialBlueprintResourceListener::fillPassValue(referenceValue, buffer, numberOfBytes);
				}
			}


		//[-------------------------------------------------------]
		//[ Private methods                                       ]
		//[-------------------------------------------------------]
		private:
			explicit MaterialBlueprintResourceListener(const MaterialBlueprintResourceListener&) = delete;
			MaterialBlueprintResourceListener& operator=(const MaterialBlueprintResourceListener&) = delete;


		//[-------------------------------------------------------]
		//[ Private data                                          ]
		//[-------------------------------------------------------]
		private:
			const RendererRuntime::VrManagerOpenVR*	mVrManagerOpenVR;
			const VrManagerOpenVRListener*			mVrManagerOpenVRListener;
			VrController*							mVrController;


		};


		//[-------------------------------------------------------]
		//[ Global variables                                      ]
		//[-------------------------------------------------------]
		static VrManagerOpenVRListener defaultVrManagerOpenVRListener;
		static MaterialBlueprintResourceListener materialBlueprintResourceListener;


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
	} // detail
}


//[-------------------------------------------------------]
//[ Public methods                                        ]
//[-------------------------------------------------------]
VrController::VrController(RendererRuntime::CameraSceneItem& cameraSceneItem) :
	IController(cameraSceneItem),
	mRendererRuntime(cameraSceneItem.getSceneResource().getRendererRuntime()),
	mTeleportIndicationLightSceneItem(nullptr)
{
	// Register our listeners
	if (mRendererRuntime.getVrManager().getVrManagerTypeId() == RendererRuntime::VrManagerOpenVR::TYPE_ID)
	{
		RendererRuntime::VrManagerOpenVR& vrManagerOpenVR = static_cast<RendererRuntime::VrManagerOpenVR&>(mRendererRuntime.getVrManager());
		::detail::defaultVrManagerOpenVRListener.setVrManagerOpenVR(vrManagerOpenVR, *this);
		vrManagerOpenVR.setVrManagerOpenVRListener(&::detail::defaultVrManagerOpenVRListener);
		::detail::materialBlueprintResourceListener.setVrManagerOpenVR(vrManagerOpenVR, ::detail::defaultVrManagerOpenVRListener, *this);
		mRendererRuntime.getMaterialBlueprintResourceManager().setMaterialBlueprintResourceListener(&::detail::materialBlueprintResourceListener);
	}

	{ // Create the teleport indication light scene item
		RendererRuntime::SceneResource& sceneResource = cameraSceneItem.getSceneResource();
		RendererRuntime::SceneNode* sceneNode = sceneResource.createSceneNode(RendererRuntime::Transform::IDENTITY);
		assert(nullptr != sceneNode);
		mTeleportIndicationLightSceneItem = sceneResource.createSceneItem<RendererRuntime::LightSceneItem>(*sceneNode);
		assert(nullptr != mTeleportIndicationLightSceneItem);
		mTeleportIndicationLightSceneItem->setColor(glm::vec3(0.0f, 1.0f, 0.0f));
		mTeleportIndicationLightSceneItem->setVisible(false);
	}
}

VrController::~VrController()
{
	// TODO(co) Destroy the teleport indication light scene item? (not really worth the effort here)

	// Unregister our listeners
	if (mRendererRuntime.getVrManager().getVrManagerTypeId() == RendererRuntime::VrManagerOpenVR::TYPE_ID)
	{
		static_cast<RendererRuntime::VrManagerOpenVR&>(mRendererRuntime.getVrManager()).setVrManagerOpenVRListener(nullptr);
		mRendererRuntime.getMaterialBlueprintResourceManager().setMaterialBlueprintResourceListener(nullptr);
	}
}

const RendererRuntime::LightSceneItem& VrController::getTeleportIndicationLightSceneItemSafe() const
{
	assert(nullptr != mTeleportIndicationLightSceneItem);
	return *mTeleportIndicationLightSceneItem;
}


//[-------------------------------------------------------]
//[ Public virtual IController methods                    ]
//[-------------------------------------------------------]
void VrController::onUpdate(float, bool)
{
	// The first VR controller is used for teleporting
	// -> A green light indicates the position one will end up
	// -> When pressing the trigger button one teleports to this position
	if (mRendererRuntime.getVrManager().getVrManagerTypeId() == RendererRuntime::VrManagerOpenVR::TYPE_ID && ::detail::defaultVrManagerOpenVRListener.getNumberOfVrControllers() >= 1 && nullptr != mTeleportIndicationLightSceneItem)
	{
		const RendererRuntime::VrManagerOpenVR& vrManagerOpenVR = static_cast<RendererRuntime::VrManagerOpenVR&>(mRendererRuntime.getVrManager());
		const bool hasFocus = vrManagerOpenVR.getVrSystem()->IsInputAvailable();
		bool teleportIndicationLightSceneItemVisible = hasFocus;

		// Do only show the teleport indication light scene item visible if the input focus is captured by our process
		if (hasFocus)
		{
			// Get VR controller transform data
			const glm::mat4& devicePoseMatrix = vrManagerOpenVR.getDevicePoseMatrix(::detail::defaultVrManagerOpenVRListener.getVrControllerTrackedDeviceIndices(::detail::FIRST_CONTROLLER_INDEX));
			glm::vec3 scale;
			glm::quat rotation;
			glm::vec3 translation;
			glm::vec3 skew;
			glm::vec4 perspective;
			glm::decompose(devicePoseMatrix, scale, rotation, translation, skew, perspective);

			// Construct ray
			const glm::dvec3 rayOrigin = glm::dvec3(translation) + getCameraSceneItem().getParentSceneNodeSafe().getGlobalTransform().position;
			const glm::dvec3 rayDirection = rotation * RendererRuntime::Math::VEC3_FORWARD;

			// Simple ray-plane intersection
			static constexpr double MAXIMUM_TELEPORT_DISTANCE = 10.0;
			double distance = 0.0;
			if (glm::intersectRayPlane(rayOrigin, rayDirection, RendererRuntime::Math::DVEC3_ZERO, RendererRuntime::Math::DVEC3_UP, distance) && !std::isnan(distance) && distance <= MAXIMUM_TELEPORT_DISTANCE)
			{
				mTeleportIndicationLightSceneItem->getParentSceneNode()->setPosition(rayOrigin + rayDirection * distance);
			}
			else
			{
				teleportIndicationLightSceneItemVisible = false;
			}
		}

		// Set teleport indication light scene item visibility
		mTeleportIndicationLightSceneItem->setVisible(teleportIndicationLightSceneItemVisible);
	}
}

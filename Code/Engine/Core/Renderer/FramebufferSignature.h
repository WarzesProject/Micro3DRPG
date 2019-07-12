#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Export.h"
#include "Core/StringId.h"
#include "Core/GetInvalid.h"

#include <cstring>	// For "memcpy()"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef StringId AssetId;					///< Asset identifier, internally just a POD "uint32_t", string ID scheme is "<project name>/<asset directory>/<asset name>"
	typedef uint32_t FramebufferSignatureId;	///< Framebuffer signature identifier, result of hashing framebuffer properties


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	struct FramebufferSignatureAttachment final
	{
		AssetId  textureAssetId;
		uint32_t mipmapIndex;
		uint32_t layerIndex;	///< "slice" in Direct3D terminology, depending on the texture type it's a 2D texture array layer, 3D texture slice or cube map face
		inline FramebufferSignatureAttachment() :
			textureAssetId(getInvalid<AssetId>()),
			mipmapIndex(0),
			layerIndex(0)
		{
			// Nothing here
		}
		inline FramebufferSignatureAttachment(const FramebufferSignatureAttachment& framebufferSignatureAttachment) :
			textureAssetId(framebufferSignatureAttachment.textureAssetId),
			mipmapIndex(framebufferSignatureAttachment.mipmapIndex),
			layerIndex(framebufferSignatureAttachment.layerIndex)
		{
			// Nothing here
		}
		inline FramebufferSignatureAttachment(AssetId _textureAssetId, uint32_t _mipmapIndex = 0, uint32_t _layerIndex = 0) :
			textureAssetId(_textureAssetId),
			mipmapIndex(_mipmapIndex),
			layerIndex(_layerIndex)
		{
			// Nothing here
		}
	};

	/**
	*  @brief
	*    Framebuffer signature
	*/
	class FramebufferSignature final
	{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Default constructor
		*/
		inline FramebufferSignature() :
			mNumberOfColorAttachments(0),
			mFramebufferSignatureId(getInvalid<FramebufferSignatureId>())
		{
			// Nothing here
		}

		/**
		*  @brief
		*    Constructor
		*
		*  @param[in] numberOfColorAttachments
		*    Number of color attachments [0, 8]
		*  @param[in] colorFramebufferSignatureAttachments
		*    Color framebuffer signature attachment
		*  @param[in] depthStencilFramebufferSignatureAttachment
		*    Depth stencil framebuffer signature attachment
		*/
		RENDERERRUNTIME_API_EXPORT FramebufferSignature(uint8_t numberOfColorAttachments, const FramebufferSignatureAttachment colorFramebufferSignatureAttachments[8], const FramebufferSignatureAttachment& depthStencilFramebufferSignatureAttachment);

		/**
		*  @brief
		*    Copy constructor
		*
		*  @param[in] framebufferSignature
		*    Framebuffer signature to copy from
		*/
		inline explicit FramebufferSignature(const FramebufferSignature& framebufferSignature) :
			mNumberOfColorAttachments(framebufferSignature.mNumberOfColorAttachments),
			mColorFramebufferSignatureAttachments{framebufferSignature.mColorFramebufferSignatureAttachments[0], framebufferSignature.mColorFramebufferSignatureAttachments[1], framebufferSignature.mColorFramebufferSignatureAttachments[2], framebufferSignature.mColorFramebufferSignatureAttachments[3], framebufferSignature.mColorFramebufferSignatureAttachments[4], framebufferSignature.mColorFramebufferSignatureAttachments[5], framebufferSignature.mColorFramebufferSignatureAttachments[6], framebufferSignature.mColorFramebufferSignatureAttachments[7]},
			mDepthStencilFramebufferSignatureAttachment(framebufferSignature.mDepthStencilFramebufferSignatureAttachment),
			mFramebufferSignatureId(framebufferSignature.mFramebufferSignatureId)
		{
			// Nothing here
		}

		/**
		*  @brief
		*    Destructor
		*/
		inline ~FramebufferSignature()
		{
			// Nothing here
		}

		/**
		*  @brief
		*    Copy operator
		*/
		inline FramebufferSignature& operator=(const FramebufferSignature& framebufferSignature)
		{
			mNumberOfColorAttachments = framebufferSignature.mNumberOfColorAttachments;
			memcpy(mColorFramebufferSignatureAttachments, framebufferSignature.mColorFramebufferSignatureAttachments, sizeof(FramebufferSignatureAttachment) * 8);
			mDepthStencilFramebufferSignatureAttachment = framebufferSignature.mDepthStencilFramebufferSignatureAttachment;
			mFramebufferSignatureId = framebufferSignature.mFramebufferSignatureId;

			// Done
			return *this;
		}

		//[-------------------------------------------------------]
		//[ Getter for input data                                 ]
		//[-------------------------------------------------------]
		[[nodiscard]] inline uint8_t getNumberOfColorAttachments() const
		{
			return mNumberOfColorAttachments;
		}

		[[nodiscard]] inline const FramebufferSignatureAttachment& getColorFramebufferSignatureAttachment(uint8_t index) const
		{
			assert(index < mNumberOfColorAttachments);
			return mColorFramebufferSignatureAttachments[index];
		}

		[[nodiscard]] inline const FramebufferSignatureAttachment& getDepthStencilFramebufferSignatureAttachment() const
		{
			return mDepthStencilFramebufferSignatureAttachment;
		}

		//[-------------------------------------------------------]
		//[ Getter for derived data                               ]
		//[-------------------------------------------------------]
		[[nodiscard]] inline FramebufferSignatureId getFramebufferSignatureId() const
		{
			return mFramebufferSignatureId;
		}


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		// Input data
		uint8_t						   mNumberOfColorAttachments;
		FramebufferSignatureAttachment mColorFramebufferSignatureAttachments[8];
		FramebufferSignatureAttachment mDepthStencilFramebufferSignatureAttachment;
		// Derived data
		FramebufferSignatureId mFramebufferSignatureId;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

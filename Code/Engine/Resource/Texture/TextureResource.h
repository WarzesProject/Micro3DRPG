#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Resource/IResource.h"

#include "Renderer.h"


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class TextureResource;
	template <class ELEMENT_TYPE, typename ID_TYPE, uint32_t MAXIMUM_NUMBER_OF_ELEMENTS> class PackedElementManager;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef uint32_t TextureResourceId;	///< POD texture resource identifier


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	/**
	*  @brief
	*    Texture resource class
	*/
	class TextureResource final : public IResource
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class TextureResourceManager;
		friend class ITextureResourceLoader;
		friend PackedElementManager<TextureResource, TextureResourceId, 2048>;	// Type definition of template class


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		[[nodiscard]] inline bool isRgbHardwareGammaCorrection() const
		{
			return mRgbHardwareGammaCorrection;
		}

		[[nodiscard]] inline const Renderer::ITexturePtr& getTexturePtr() const
		{
			return mTexture;
		}

		inline void setTexture(Renderer::ITexture* texture)
		{
			// Sanity check
			assert((LoadingState::LOADED == getLoadingState() || LoadingState::UNLOADED == getLoadingState()) && "Texture resource change while in-flight inside the resource streamer");

			// Set new renderer texture
			if (nullptr != mTexture)
			{
				setLoadingState(LoadingState::UNLOADED);
			}
			mTexture = texture;
			setLoadingState(LoadingState::LOADED);
		}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		inline TextureResource() :
			mRgbHardwareGammaCorrection(false)
		{
			// Nothing here
		}

		inline virtual ~TextureResource() override
		{
			// Sanity checks
			assert(nullptr == mTexture.getPointer());
		}

		explicit TextureResource(const TextureResource&) = delete;
		TextureResource& operator=(const TextureResource&) = delete;

		inline TextureResource& operator=(TextureResource&& textureResource)
		{
			// Call base implementation
			IResource::operator=(std::move(textureResource));

			// Swap data
			std::swap(mRgbHardwareGammaCorrection, textureResource.mRgbHardwareGammaCorrection);
			std::swap(mTexture,					   textureResource.mTexture);

			// Done
			return *this;
		}

		//[-------------------------------------------------------]
		//[ "RendererRuntime::PackedElementManager" management    ]
		//[-------------------------------------------------------]
		inline void initializeElement(TextureResourceId textureResourceId)
		{
			// Sanity checks
			assert(nullptr == mTexture.getPointer());

			// Call base implementation
			IResource::initializeElement(textureResourceId);
		}

		inline void deinitializeElement()
		{
			// Reset everything
			mTexture = nullptr;

			// Call base implementation
			IResource::deinitializeElement();
		}


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		bool				  mRgbHardwareGammaCorrection;	///< If true, sRGB texture formats will be used meaning the GPU will return linear space colors instead of gamma space colors when fetching texels inside a shader (the alpha channel always remains linear)
		Renderer::ITexturePtr mTexture;						///< Texture, can be a null pointer


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

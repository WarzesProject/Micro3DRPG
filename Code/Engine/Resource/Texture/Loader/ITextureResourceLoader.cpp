#include "stdafx.h"
#include "Resource/Texture/Loader/ITextureResourceLoader.h"
#include "Resource/Texture/TextureResource.h"
#include "IRendererRuntime.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::IResourceLoader methods ]
	//[-------------------------------------------------------]
	void ITextureResourceLoader::initialize(const Asset& asset, bool reload, IResource& resource)
	{
		IResourceLoader::initialize(asset, reload);
		mTextureResource = static_cast<TextureResource*>(&resource);
	}

	bool ITextureResourceLoader::onDispatch()
	{
		// Create the renderer texture instance
		mTextureResource->mTexture = (mRendererRuntime.getRenderer().getCapabilities().nativeMultithreading ? mTexture : createRendererTexture());

		// Fully loaded
		return true;
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

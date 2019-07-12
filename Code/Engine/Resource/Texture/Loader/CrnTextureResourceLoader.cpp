#include "stdafx.h"
#include "Resource/Texture/Loader/CrnTextureResourceLoader.h"
#include "Resource/Texture/TextureResourceManager.h"
#include "Resource/Texture/TextureResource.h"
#include "Core/File/IFile.h"
#include "IRendererRuntime.h"

#ifndef RENDERER_CRN_INCLUDED
	#define RENDERER_CRN_INCLUDED

	// Disable warnings in external headers, we can't fix them
	PRAGMA_WARNING_PUSH
		PRAGMA_WARNING_DISABLE_MSVC(4018)	// warning C4018: '<': signed/unsigned mismatch
		PRAGMA_WARNING_DISABLE_MSVC(4061)	// warning C4061: enumerator 'cCRNFmtTotal' in switch of enum 'crn_format' is not explicitly handled by a case label
		PRAGMA_WARNING_DISABLE_MSVC(4242)	// warning C4242: '=': conversion from 'crnd::uint32' to 'crnd::uint8', possible loss of data
		PRAGMA_WARNING_DISABLE_MSVC(4244)	// warning C4244: 'initializing': conversion from 'crnd::uint' to 'crnd::uint8', possible loss of data
		PRAGMA_WARNING_DISABLE_MSVC(4302)	// warning C4302: 'type cast': truncation from 'crnd::uint8 *' to 'crnd::uint32'
		PRAGMA_WARNING_DISABLE_MSVC(4311)	// warning C4311: 'type cast': pointer truncation from 'crnd::uint8 *' to 'crnd::uint32'
		PRAGMA_WARNING_DISABLE_MSVC(4365)	// warning C4365: '<x>': conversion from '<y>' to '<z>', signed/unsigned mismatch
		PRAGMA_WARNING_DISABLE_MSVC(4548)	// warning C4548: expression before comma has no effect; expected expression with side-effect
		PRAGMA_WARNING_DISABLE_MSVC(4555)	// warning C4555: expression has no effect; expected expression with side-effect
		PRAGMA_WARNING_DISABLE_MSVC(4668)	// warning C4668: '<x>' is not defined as a preprocessor macro, replacing with '<y>' for '<z>'
		PRAGMA_WARNING_DISABLE_CLANG("-Wunused-value")	// warning: expression result unused [-Wunused-value]
		PRAGMA_WARNING_DISABLE_CLANG("-Warray-bounds")	// warning: array index 1 is past the end of the array (which contains 1 element) [-Warray-bounds]
		PRAGMA_WARNING_DISABLE_GCC("-Wunused-value")	// warning: expression result unused [-Wunused-value]
			#include <crunch/inc/crn_decomp.h>
			#include <crunch/inc/dds_defs.h>
		PRAGMA_WARNING_POP
#endif

#include <algorithm>


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
namespace
{
	namespace detail
	{


		//[-------------------------------------------------------]
		//[ Global functions                                      ]
		//[-------------------------------------------------------]
		void* crunchRealloc(void* p, size_t size, size_t* pActual_size, bool, void* pUser_data)
		{
			if (nullptr != pActual_size)
			{
				*pActual_size = size;
			}
			return static_cast<Renderer::IAllocator*>(pUser_data)->reallocate(p, 0, size, CRNLIB_MIN_ALLOC_ALIGNMENT);
		}

		size_t crunchMsize(void*, void*)
		{
			// Not supported, used only if "CRNLIB_MEM_STATS" preprocessor definition is set
			return 0;
		}


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
	} // detail
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::IResourceLoader methods ]
	//[-------------------------------------------------------]
	bool CrnTextureResourceLoader::onDeserialization(IFile& file)
	{
		// Load the source image file into memory: Get file size and file data
		mNumberOfUsedFileDataBytes = static_cast<uint32_t>(file.getNumberOfBytes());
		if (mNumberOfFileDataBytes < mNumberOfUsedFileDataBytes)
		{
			mNumberOfFileDataBytes = mNumberOfUsedFileDataBytes;
			delete [] mFileData;
			mFileData = new uint8_t[mNumberOfFileDataBytes];
		}
		file.read(mFileData, mNumberOfUsedFileDataBytes);

		// Done
		return true;
	}

	void CrnTextureResourceLoader::onProcessing()
	{
		// TODO(co) Error handling

		// Decompress/transcode CRN to DDS
		crnd::crn_texture_info crnTextureInfo;
		if (!crnd::crnd_get_texture_info(mFileData, mNumberOfUsedFileDataBytes, &crnTextureInfo))
		{
			RENDERER_ASSERT(mRendererRuntime.getContext(), false, "crnd_get_texture_info() failed")
			return;
		}
		mWidth  = crnTextureInfo.m_width;
		mHeight = crnTextureInfo.m_height;
		mCubeMap = (crnTextureInfo.m_faces > 1);

		// Sanity check
		RENDERER_ASSERT(mRendererRuntime.getContext(), !mCubeMap || mWidth == mHeight, "The width and height of a cube map must be identical")

		// Get the renderer texture format
		switch (crnTextureInfo.m_format)
		{
			// DXT1 compression (known as BC1 in DirectX 10, RGB compression: 8:1, 8 bytes per block)
			case cCRNFmtDXT1:
				mTextureFormat = static_cast<uint8_t>(mTextureResource->isRgbHardwareGammaCorrection() ? Renderer::TextureFormat::BC1_SRGB : Renderer::TextureFormat::BC1);
				break;

			// DXT3 compression (known as BC2 in DirectX 10, RGBA compression: 4:1, 16 bytes per block)
			case cCRNFmtDXT3:
				mTextureFormat = static_cast<uint8_t>(mTextureResource->isRgbHardwareGammaCorrection() ? Renderer::TextureFormat::BC2_SRGB : Renderer::TextureFormat::BC2);
				break;

			// DXT5 compression (known as BC3 in DirectX 10, RGBA compression: 4:1, 16 bytes per block)
			case cCRNFmtDXT5:
			case cCRNFmtDXT5_CCxY:
			case cCRNFmtDXT5_xGxR:
			case cCRNFmtDXT5_xGBR:
			case cCRNFmtDXT5_AGBR:
				mTextureFormat = static_cast<uint8_t>(mTextureResource->isRgbHardwareGammaCorrection() ? Renderer::TextureFormat::BC3_SRGB : Renderer::TextureFormat::BC3);
				break;

			// 2 component texture compression (luminance & alpha compression 4:1 -> normal map compression, also known as 3DC/ATI2N, known as BC5 in DirectX 10, 16 bytes per block)
			case cCRNFmtDXN_XY:
			case cCRNFmtDXN_YX:
				mTextureFormat = Renderer::TextureFormat::BC5;
				break;

			case cCRNFmtDXT5A:
			case cCRNFmtETC1:
			case cCRNFmtETC2:
			case cCRNFmtETC2A:
			case cCRNFmtETC1S:
			case cCRNFmtETC2AS:
			case cCRNFmtTotal:
			case cCRNFmtForceDWORD:
			default:
				// Error!
				// TODO(co)
				RENDERER_ASSERT(mRendererRuntime.getContext(), false, "Invalid format")
				return;
		}

		// Does the data contain mipmaps?
		mDataContainsMipmaps = (crnTextureInfo.m_levels > 1);

		crnd::crnd_unpack_context crndUnpackContext = crnd::crnd_unpack_begin(mFileData, mNumberOfUsedFileDataBytes);
		if (nullptr == crndUnpackContext)
		{
			RENDERER_ASSERT(mRendererRuntime.getContext(), false, "crnd_unpack_begin() failed")
			return;
		}

		// Handle optional top mipmap removal
		// TODO(co) Possible optimization of optional top mipmap removal: Don't load in the skipped mipmaps into memory in the first place ("mFileData")
		int startLevelIndex = mRendererRuntime.getTextureResourceManager().getNumberOfTopMipmapsToRemove();
		if (startLevelIndex >= static_cast<int>(crnTextureInfo.m_levels))
		{
			startLevelIndex = static_cast<int>(crnTextureInfo.m_levels) - 1;
		}

		// Optional top mipmap removal security checks
		// -> Ensure we don't go below 4x4 to not get into troubles with 4x4 blocked based compression
		// -> Ensure the base mipmap we tell the renderer about is a multiple of four. Even if the original base mipmap is a multiple of four, one of the lower mipmaps might not be.
		while (startLevelIndex > 0 && (std::max(1U, mWidth >> startLevelIndex) < 4 || std::max(1U, mHeight >> startLevelIndex) < 4))
		{
			--startLevelIndex;
		}
		while (startLevelIndex > 0 && (0 != (std::max(1U, mWidth >> startLevelIndex) % 4) || (0 != std::max(1U, mHeight >> startLevelIndex) % 4)))
		{
			--startLevelIndex;
		}

		// Allocate resulting image data
		const crn_uint32 numberOfBytesPerDxtBlock = crnd::crnd_get_bytes_per_dxt_block(crnTextureInfo.m_format);
		{
			mNumberOfUsedImageDataBytes = 0;
			for (crn_uint32 levelIndex = static_cast<crn_uint32>(startLevelIndex); levelIndex < crnTextureInfo.m_levels; ++levelIndex)
			{
				const crn_uint32 width = std::max(1U, mWidth >> levelIndex);
				const crn_uint32 height = std::max(1U, mHeight >> levelIndex);
				const crn_uint32 blocksX = std::max(1U, (width + 3) >> 2);
				const crn_uint32 blocksY = std::max(1U, (height + 3) >> 2);
				const crn_uint32 rowPitch = blocksX * numberOfBytesPerDxtBlock;
				const crn_uint32 totalFaceSize = rowPitch * blocksY;
				mNumberOfUsedImageDataBytes += totalFaceSize;
			}
			mNumberOfUsedImageDataBytes *= crnTextureInfo.m_faces;

			if (mNumberOfImageDataBytes < mNumberOfUsedImageDataBytes)
			{
				mNumberOfImageDataBytes = mNumberOfUsedImageDataBytes;
				delete [] mImageData;
				mImageData = new uint8_t[mNumberOfImageDataBytes];
			}
		}

		// Data layout: The renderer interface expects: CRN and KTX files are organized in mip-major order, like this:
		//   Mip0: Face0, Face1, Face2, Face3, Face4, Face5
		//   Mip1: Face0, Face1, Face2, Face3, Face4, Face5
		//   etc.

		{ // Now transcode all face and mipmap levels into memory, one mip level at a time
			void* decompressedImages[cCRNMaxFaces];
			uint8_t* currentImageData = mImageData;
			for (crn_uint32 levelIndex = static_cast<crn_uint32>(startLevelIndex); levelIndex < crnTextureInfo.m_levels; ++levelIndex)
			{
				// Compute the face's width, height, number of DXT blocks per row/col, etc.
				const crn_uint32 width = std::max(1U, mWidth >> levelIndex);
				const crn_uint32 height = std::max(1U, mHeight >> levelIndex);
				const crn_uint32 blocksX = std::max(1U, (width + 3) >> 2);
				const crn_uint32 blocksY = std::max(1U, (height + 3) >> 2);
				const crn_uint32 rowPitch = blocksX * numberOfBytesPerDxtBlock;
				const crn_uint32 totalFaceSize = rowPitch * blocksY;

				// Update the face pointer array needed by "crnd_unpack_level()"
				for (crn_uint32 faceIndex = 0; faceIndex < crnTextureInfo.m_faces; ++faceIndex)
				{
					decompressedImages[faceIndex] = currentImageData;
					currentImageData += totalFaceSize;
				}

				// Now transcode the level to raw DXTn
				if (!crnd::crnd_unpack_level(crndUnpackContext, decompressedImages, totalFaceSize, rowPitch, levelIndex))
				{
					// Free allocated memory
					crnd::crnd_unpack_end(crndUnpackContext);
					RENDERER_ASSERT(mRendererRuntime.getContext(), false, "Failed transcoding texture")
					return;
				}
			}
		}

		// Free allocated memory
		crnd::crnd_unpack_end(crndUnpackContext);

		// In case we removed top level mipmaps, we need to update the texture dimension
		if (0 != startLevelIndex)
		{
			mWidth = std::max(1U, mWidth >> startLevelIndex);
			mHeight = std::max(1U, mHeight >> startLevelIndex);
		}

		// Can we create the renderer resource asynchronous as well?
		if (mRendererRuntime.getRenderer().getCapabilities().nativeMultithreading)
		{
			mTexture = createRendererTexture();
		}
	}


	//[-------------------------------------------------------]
	//[ Protected RendererRuntime::ITextureResourceLoader methods ]
	//[-------------------------------------------------------]
	Renderer::ITexture* CrnTextureResourceLoader::createRendererTexture()
	{
		Renderer::ITexture* texture = nullptr;
		const uint32_t flags = (mDataContainsMipmaps ? (Renderer::TextureFlag::DATA_CONTAINS_MIPMAPS | Renderer::TextureFlag::SHADER_RESOURCE) : Renderer::TextureFlag::SHADER_RESOURCE);
		if (mCubeMap)
		{
			// Cube texture
			texture = mRendererRuntime.getTextureManager().createTextureCube(mWidth, mHeight, static_cast<Renderer::TextureFormat::Enum>(mTextureFormat), mImageData, flags, Renderer::TextureUsage::IMMUTABLE);
		}
		else if (1 == mWidth || 1 == mHeight)
		{
			// 1D texture
			texture = mRendererRuntime.getTextureManager().createTexture1D((1 == mWidth) ? mHeight : mWidth, static_cast<Renderer::TextureFormat::Enum>(mTextureFormat), mImageData, flags, Renderer::TextureUsage::IMMUTABLE);
		}
		else
		{
			// 2D texture
			texture = mRendererRuntime.getTextureManager().createTexture2D(mWidth, mHeight, static_cast<Renderer::TextureFormat::Enum>(mTextureFormat), mImageData, flags, Renderer::TextureUsage::IMMUTABLE);
		}
		RENDERER_SET_RESOURCE_DEBUG_NAME(texture, getAsset().virtualFilename)
		return texture;
	}


	//[-------------------------------------------------------]
	//[ Protected methods                                     ]
	//[-------------------------------------------------------]
	CrnTextureResourceLoader::CrnTextureResourceLoader(IResourceManager& resourceManager, IRendererRuntime& rendererRuntime) :
		ITextureResourceLoader(resourceManager, rendererRuntime),
		mWidth(0),
		mHeight(0),
		mTextureFormat(0),
		mCubeMap(false),
		mDataContainsMipmaps(false),
		mNumberOfFileDataBytes(0),
		mNumberOfUsedFileDataBytes(0),
		mFileData(nullptr),
		mNumberOfImageDataBytes(0),
		mNumberOfUsedImageDataBytes(0),
		mImageData(nullptr)
	{
		static std::atomic<bool> crunchAllocatorSet = false;
		if (!crunchAllocatorSet)
		{
			crnd::g_pRealloc   = ::detail::crunchRealloc;
			crnd::g_pMSize	   = ::detail::crunchMsize;
			crnd::g_pUser_data = &rendererRuntime.getContext().getAllocator();
			crunchAllocatorSet = true;
		}
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

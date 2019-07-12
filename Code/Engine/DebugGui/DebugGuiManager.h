#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Export.h"
#include "Renderer.h"
#include "Core/Manager.h"
#include "Core/StringId.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4365)	// warning C4365: 'argument': conversion from 'long' to 'unsigned int', signed/unsigned mismatch
	PRAGMA_WARNING_DISABLE_MSVC(5026)	// warning C5026: 'std::_Generic_error_category': move constructor was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(5027)	// warning C5027: 'std::_Generic_error_category': move assignment operator was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(4571)	// warning C4571: Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
	PRAGMA_WARNING_DISABLE_MSVC(4625)	// warning C4625: 'std::codecvt_base': copy constructor was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(4626)	// warning C4626: 'std::codecvt<char16_t,char,_Mbstatet>': assignment operator was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(4668)	// warning C4668: '_M_HYBRID_X86_ARM64' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
	PRAGMA_WARNING_DISABLE_MSVC(4774)	// warning C4774: 'sprintf_s' : format string expected in argument 3 is not a string literal
	#include <string>
	#include <vector>
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
struct ImGuiContext;
namespace RendererRuntime
{
	class IRendererRuntime;
	class CompositorWorkspaceInstance;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef StringId			 AssetId;	///< Asset identifier, internally just a POD "uint32_t", string ID scheme is "<project name>/<asset directory>/<asset name>"
	typedef std::vector<AssetId> AssetIds;


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	/**
	*  @brief
	*    Debug GUI manager using ImGui ( https://github.com/ocornut/imgui )
	*
	*  @remarks
	*    Supports two command buffer fill modes:
	*    - Using fixed build in renderer configuration, including shaders
	*    - Using a material resource blueprint set by the caller
	*/
	class DebugGuiManager : private Manager
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class RendererRuntimeImpl;


	//[-------------------------------------------------------]
	//[ Public static methods                                 ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Set ImGui allocator functions
		*
		*  @param[in] allocator
		*    Allocator to use
		*/
		RENDERERRUNTIME_API_EXPORT static void setImGuiAllocatorFunctions(Renderer::IAllocator& allocator);

		/**
		*  @brief
		*    Return the asset IDs of automatically generated dynamic default texture assets
		*
		*  @param[out] assetIds
		*    Receives the asset IDs of automatically generated dynamic default texture assets, the list is not cleared before new entries are added
		*
		*  @remarks
		*    The debug GUI manager automatically generates some dynamic default texture assets one can reference e.g. inside material blueprint resources:
		*    - "Unrimp/Texture/DynamicByCode/ImGuiGlyphMap2D"
		*/
		static void getDefaultTextureAssetIds(AssetIds& assetIds);


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		RENDERERRUNTIME_API_EXPORT void newFrame(Renderer::IRenderTarget& renderTarget, CompositorWorkspaceInstance* compositorWorkspaceInstance = nullptr);
		[[nodiscard]] RENDERERRUNTIME_API_EXPORT const Renderer::IVertexArrayPtr& getFillVertexArrayPtr();
		RENDERERRUNTIME_API_EXPORT void fillGraphicsCommandBuffer(Renderer::CommandBuffer& commandBuffer);
		RENDERERRUNTIME_API_EXPORT void fillGraphicsCommandBufferUsingFixedBuildInRendererConfiguration(Renderer::CommandBuffer& commandBuffer);

		// Helper
		inline bool hasOpenMetricsWindow() const
		{
			return mOpenMetricsWindow;
		}

		inline void openMetricsWindow()
		{
			mOpenMetricsWindow = true;
		}


	//[-------------------------------------------------------]
	//[ Protected virtual RendererRuntime::DebugGuiManager methods ]
	//[-------------------------------------------------------]
	protected:
		virtual void initializeImGuiKeyMap() = 0;
		virtual void startup();
		virtual void onNewFrame(Renderer::IRenderTarget& renderTarget) = 0;


	//[-------------------------------------------------------]
	//[ Protected methods                                     ]
	//[-------------------------------------------------------]
	protected:
		explicit DebugGuiManager(IRendererRuntime& rendererRuntime);
		virtual ~DebugGuiManager();
		explicit DebugGuiManager(const DebugGuiManager&) = delete;
		DebugGuiManager& operator=(const DebugGuiManager&) = delete;
		void createFixedBuildInRendererConfigurationResources();


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		IRendererRuntime&		mRendererRuntime;		///< Renderer runtime instance, do not destroy the instance
		ImGuiContext*			mImGuiContext;			///< ImGui context, always valid
		std::string				mAbsoluteIniFilename;	///< Absolute UTF-8 ImGui ini-filename, class member since ImGui just keeps a pointer to this string instance
		std::string				mAbsoluteLogFilename;	///< Absolute UTF-8 ImGui log-filename, class member since ImGui just keeps a pointer to this string instance
		bool					mIsRunning;				///< The debug GUI manager will be initialized lazy when "RendererRuntime::DebugGuiManager::newFrame()" is called for the first time
		Renderer::ITexture2DPtr	mTexture2D;
		// Fixed build in renderer configuration resources
		Renderer::IRootSignaturePtr			mRootSignature;
		Renderer::IGraphicsProgramPtr		mGraphicsProgram;
		Renderer::IGraphicsPipelineStatePtr	mGraphicsPipelineState;
		Renderer::IUniformBufferPtr			mVertexShaderUniformBuffer;
		Renderer::handle					mObjectSpaceToClipSpaceMatrixUniformHandle;
		Renderer::IResourceGroupPtr			mResourceGroup;		///< Resource group, can be a null pointer
		Renderer::IResourceGroupPtr			mSamplerStateGroup;	///< Sampler state resource group, can be a null pointer
		// Vertex and index buffer
		Renderer::IVertexBufferPtr	mVertexBufferPtr;
		uint32_t					mNumberOfAllocatedVertices;
		Renderer::IIndexBufferPtr	mIndexBufferPtr;
		uint32_t					mNumberOfAllocatedIndices;
		Renderer::IVertexArrayPtr	mVertexArrayPtr;
		// Helper
		bool mOpenMetricsWindow;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

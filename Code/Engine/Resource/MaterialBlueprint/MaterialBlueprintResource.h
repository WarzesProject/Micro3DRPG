#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Resource/IResource.h"
#include "Resource/Material/MaterialProperties.h"
#include "Resource/ShaderBlueprint/GraphicsShaderType.h"
#include "Resource/ShaderBlueprint/Cache/ShaderProperties.h"
#include "Resource/MaterialBlueprint/Cache/GraphicsPipelineStateCacheManager.h"
#include "Resource/MaterialBlueprint/Cache/ComputePipelineStateCacheManager.h"

#include "Renderer.h"


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class IFile;
	class PassBufferManager;
	class MaterialBufferManager;
	template <class ELEMENT_TYPE, typename ID_TYPE, uint32_t MAXIMUM_NUMBER_OF_ELEMENTS> class PackedElementManager;
	template <class TYPE, class LOADER_TYPE, typename ID_TYPE, uint32_t MAXIMUM_NUMBER_OF_ELEMENTS> class ResourceManagerTemplate;
	class MaterialBlueprintResourceLoader;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef StringId AssetId;						///< Asset identifier, internally just a POD "uint32_t", string ID scheme is "<project name>/<asset directory>/<asset name>"
	typedef uint32_t TextureResourceId;				///< POD texture resource identifier
	typedef uint32_t ShaderBlueprintResourceId;		///< POD shader blueprint resource identifier
	typedef uint32_t VertexAttributesResourceId;	///< POD vertex attributes resource identifier
	typedef uint32_t MaterialBlueprintResourceId;	///< POD material blueprint resource identifier
	typedef StringId ShaderPropertyId;				///< Shader property identifier, internally just a POD "uint32_t", result of hashing the property name


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	/**
	*  @brief
	*    Material blueprint resource
	*
	*  @remarks
	*    == Regarding shader combination explosion ==
	*    The texture manager automatically generates some dynamic default texture assets like "Unrimp/Texture/DynamicByCode/IdentityAlbedoMap2D" or
	*    "Unrimp/Texture/DynamicByCode/IdentityNormalMap2D" one can reference e.g. inside material blueprint resources. Especially the identity texture maps
	*    can be used as default material property value. While it's tempting to add shader combination material properties like "UseAlbedoMap",
	*    "UseNormalMap" etc. one has to keep the problem of shader combination explosion in mind. Especially in more complex material blueprints the
	*    number of shader combinations can quickly reach a point were it's practically impossible to e.g. generate a shader cache for shipped products
	*    or in case of Mac OS X (no OpenGL binary shader support) generate the required shaders during program start. The problem can be fought with
	*    complex heuristics to filter out unused or rarely used shader combinations, this is an art form of itself. Please note that shader combination
	*    explosion is a real existing serious problem which especially inside the product shipping phase can create stressful situations. Additionally,
	*    if the shader complexity of different materials vary too extreme, the framerate might get instable. So, when designing material blueprints do
	*    carefully thing about which shader combinations you really need.
	*
	*  @note
	*    - Automatic handling of packing rules for uniform variables (see "Reference for HLSL - Shader Models vs Shader Profiles - Shader Model 4 - Packing Rules for Constant Variables" at https://msdn.microsoft.com/en-us/library/windows/desktop/bb509632%28v=vs.85%29.aspx )
	*    - When writing new material blueprint resources, you might want to take the packing rules for uniform variables into account for an efficient data layout
	*    - A material blueprint can have a compute or a graphics pipeline state, but never both at one and the same time
	*/
	class MaterialBlueprintResource final : public IResource
	{


	//[-------------------------------------------------------]
	//[ Friends                                               ]
	//[-------------------------------------------------------]
		friend class MaterialBlueprintResourceLoader;
		friend class MaterialBlueprintResourceManager;
		friend class MaterialResourceLoader;	// TODO(co) Decent material resource list management inside the material blueprint resource (link, unlink etc.) - remove this
		friend class MaterialResourceManager;	// TODO(co) Remove
		friend class MaterialBufferManager;		// TODO(co) Remove. Decent material technique list management inside the material blueprint resource (link, unlink etc.)
		friend PackedElementManager<MaterialBlueprintResource, MaterialBlueprintResourceId, 64>;										// Type definition of template class
		friend ResourceManagerTemplate<MaterialBlueprintResource, MaterialBlueprintResourceLoader, MaterialBlueprintResourceId, 64>;	// Type definition of template class


	//[-------------------------------------------------------]
	//[ Public definitions                                    ]
	//[-------------------------------------------------------]
	public:
		static constexpr int32_t MANDATORY_SHADER_PROPERTY = std::numeric_limits<int32_t>::max();	///< Visual importance value for mandatory shader properties (such properties are not removed when finding a fallback pipeline state)

		/**
		*  @brief
		*    Uniform/texture buffer usage
		*/
		enum class BufferUsage : uint8_t
		{
			UNKNOWN = 0,	///< Unknown buffer usage, supports the following "RendererRuntime::MaterialProperty::Usage": "UNKNOWN_REFERENCE", "GLOBAL_REFERENCE" as well as properties with simple values
			PASS,			///< Pass buffer usage, supports the following "RendererRuntime::MaterialProperty::Usage": "PASS_REFERENCE", "GLOBAL_REFERENCE" as well as properties with simple values
			MATERIAL,		///< Material buffer usage, supports the following "RendererRuntime::MaterialProperty::Usage": "MATERIAL_REFERENCE", "GLOBAL_REFERENCE" as well as properties with simple values
			INSTANCE,		///< Instance buffer usage, supports the following "RendererRuntime::MaterialProperty::Usage": "INSTANCE_REFERENCE", "GLOBAL_REFERENCE" as well as properties with simple values
			LIGHT			///< Light buffer usage for texture buffer only
		};

		typedef std::vector<MaterialProperty> UniformBufferElementProperties;

		struct UniformBuffer final
		{
			uint32_t					   rootParameterIndex;			///< Root parameter index = resource group index
			BufferUsage					   bufferUsage;
			uint32_t					   numberOfElements;
			UniformBufferElementProperties uniformBufferElementProperties;
			uint32_t					   uniformBufferNumberOfBytes;	///< Includes handling of packing rules for uniform variables (see "Reference for HLSL - Shader Models vs Shader Profiles - Shader Model 4 - Packing Rules for Constant Variables" at https://msdn.microsoft.com/en-us/library/windows/desktop/bb509632%28v=vs.85%29.aspx )
		};
		typedef std::vector<UniformBuffer> UniformBuffers;

		struct TextureBuffer final
		{
			uint32_t			  rootParameterIndex;	///< Root parameter index = resource group index
			BufferUsage			  bufferUsage;
			MaterialPropertyValue materialPropertyValue;

			TextureBuffer() :
				rootParameterIndex(getInvalid<uint32_t>()),
				bufferUsage(BufferUsage::UNKNOWN),
				materialPropertyValue(MaterialPropertyValue::fromUnknown())
			{
				// Nothing here
			}

			TextureBuffer(uint32_t rootParameterIndex, BufferUsage bufferUsage, const MaterialPropertyValue& _materialPropertyValue) :
				rootParameterIndex(rootParameterIndex),
				bufferUsage(bufferUsage),
				materialPropertyValue(MaterialProperty(getInvalid<MaterialPropertyId>(), getMaterialPropertyUsageFromBufferUsage(bufferUsage), _materialPropertyValue))
			{
				// Nothing here
			}

		};
		typedef std::vector<TextureBuffer> TextureBuffers;

		struct SamplerState final
		{
			Renderer::SamplerState	   rendererSamplerState;
			uint32_t				   rootParameterIndex;	///< Root parameter index = resource group index
			Renderer::ISamplerStatePtr samplerStatePtr;
		};
		typedef std::vector<SamplerState> SamplerStates;

		struct Texture final
		{
			// Loaded from material blueprint
			uint32_t		  rootParameterIndex;	///< Root parameter index = resource group index
			MaterialProperty  materialProperty;
			AssetId			  fallbackTextureAssetId;
			bool			  rgbHardwareGammaCorrection;
			uint32_t		  samplerStateIndex;	///< Index of the material blueprint sampler state resource to use, can be invalid (e.g. texel fetch instead of sampling might be used)

			// Derived data
			TextureResourceId textureResourceId;

			// Constructors
			Texture() :
				rootParameterIndex(getInvalid<uint32_t>()),
				rgbHardwareGammaCorrection(false),
				samplerStateIndex(getInvalid<uint32_t>()),
				textureResourceId(getInvalid<TextureResourceId>())
			{
				// Nothing here
			}
		};
		typedef std::vector<Texture> Textures;


	//[-------------------------------------------------------]
	//[ Public static methods                                 ]
	//[-------------------------------------------------------]
	public:
		[[nodiscard]] static MaterialProperty::Usage getMaterialPropertyUsageFromBufferUsage(BufferUsage bufferUsage);


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Return the graphics pipeline state cache manager
		*
		*  @return
		*    The graphics pipeline state cache manager
		*/
		[[nodiscard]] inline GraphicsPipelineStateCacheManager& getGraphicsPipelineStateCacheManager()
		{
			return mGraphicsPipelineStateCacheManager;
		}

		/**
		*  @brief
		*    Return the compute pipeline state cache manager
		*
		*  @return
		*    The compute pipeline state cache manager
		*/
		[[nodiscard]] inline ComputePipelineStateCacheManager& getComputePipelineStateCacheManager()
		{
			return mComputePipelineStateCacheManager;
		}

		/**
		*  @brief
		*    Return the material blueprint properties
		*
		*  @return
		*    The material blueprint properties
		*/
		[[nodiscard]] inline const MaterialProperties& getMaterialProperties() const
		{
			return mMaterialProperties;
		}

		/**
		*  @brief
		*    Return the visual importance of a requested shader property
		*
		*  @return
		*    The visual importance of the requested shader property, lower visual importance value = lower probability that someone will miss the shader property,
		*    can be "RendererRuntime::MaterialBlueprintResource::MANDATORY_SHADER_PROPERTY" for mandatory shader properties (such properties are not removed when finding a fallback pipeline state)
		*/
		[[nodiscard]] inline int32_t getVisualImportanceOfShaderProperty(ShaderPropertyId shaderPropertyId) const
		{
			return mVisualImportanceOfShaderProperties.getPropertyValueUnsafe(shaderPropertyId);
		}

		/**
		*  @brief
		*    Return the maximum integer value (inclusive) of a shader property
		*
		*  @return
		*    The maximum integer value (inclusive) of the requested shader property
		*/
		[[nodiscard]] inline int32_t getMaximumIntegerValueOfShaderProperty(ShaderPropertyId shaderPropertyId) const
		{
			return mMaximumIntegerValueOfShaderProperties.getPropertyValueUnsafe(shaderPropertyId);
		}

		/**
		*  @brief
		*    Optimize the given shader properties
		*
		*  @param[in] shaderProperties
		*    Shader properties to optimize
		*  @param[out] optimizedShaderProperties
		*    Optimized shader properties, cleared before new entries are added
		*
		*  @remarks
		*    Performed optimizations:
		*    - Removes all shader properties which have a zero value
		*    - Removes all shader properties which are unknown to the material blueprint
		*
		*  @note
		*    - This method should only be used at high level to reduce the shader properties to a bare minimum as soon as possible
		*/
		RENDERERRUNTIME_API_EXPORT void optimizeShaderProperties(const ShaderProperties& shaderProperties, ShaderProperties& optimizedShaderProperties) const;

		/**
		*  @brief
		*    Return the root signature
		*
		*  @return
		*    The root signature, can be a null pointer, do not destroy the instance
		*/
		[[nodiscard]] inline const Renderer::IRootSignaturePtr& getRootSignaturePtr() const
		{
			return mRootSignaturePtr;
		}

		//[-------------------------------------------------------]
		//[ Graphics pipeline state                               ]
		//[-------------------------------------------------------]
		/**
		*  @brief
		*    Return the graphics pipeline state
		*
		*  @return
		*    The graphics pipeline state
		*/
		[[nodiscard]] inline const Renderer::GraphicsPipelineState& getGraphicsPipelineState() const
		{
			return mGraphicsPipelineState;
		}

		/**
		*  @brief
		*    Return a vertex attributes resource ID
		*
		*  @return
		*    The requested vertex attributes resource ID, can be invalid
		*/
		[[nodiscard]] inline VertexAttributesResourceId getVertexAttributesResourceId() const
		{
			return mVertexAttributesResourceId;
		}

		/**
		*  @brief
		*    Return a graphics shader blueprint resource ID
		*
		*  @param[in] graphicsShaderType
		*    Graphics shader type
		*
		*  @return
		*    The requested graphics shader blueprint resource ID, can be invalid
		*/
		[[nodiscard]] inline ShaderBlueprintResourceId getGraphicsShaderBlueprintResourceId(GraphicsShaderType graphicsShaderType) const
		{
			return mGraphicsShaderBlueprintResourceId[static_cast<uint8_t>(graphicsShaderType)];
		}

		//[-------------------------------------------------------]
		//[ Compute pipeline state                                ]
		//[-------------------------------------------------------]
		/**
		*  @brief
		*    Return the compute shader blueprint resource ID
		*
		*  @return
		*    The requested compute shader blueprint resource ID, can be invalid
		*
		*  @note
		*    - If a material blueprint resource has a valid compute shader blueprint resource ID there's no graphics pipeline
		*/
		[[nodiscard]] inline ShaderBlueprintResourceId getComputeShaderBlueprintResourceId() const
		{
			return mComputeShaderBlueprintResourceId;
		}

		//[-------------------------------------------------------]
		//[ Resource                                              ]
		//[-------------------------------------------------------]
		/**
		*  @brief
		*    Return the uniform buffers
		*
		*  @return
		*    The uniform buffers
		*/
		[[nodiscard]] inline const UniformBuffers& getUniformBuffers() const
		{
			return mUniformBuffers;
		}

		/**
		*  @brief
		*    Return the texture buffers
		*
		*  @return
		*    The texture buffers
		*/
		[[nodiscard]] inline const TextureBuffers& getTextureBuffers() const
		{
			return mTextureBuffers;
		}

		/**
		*  @brief
		*    Return the sampler states
		*
		*  @return
		*    The sampler states
		*/
		[[nodiscard]] inline const SamplerStates& getSamplerStates() const
		{
			return mSamplerStates;
		}

		/**
		*  @brief
		*    Return the textures
		*
		*  @return
		*    The textures
		*/
		[[nodiscard]] inline const Textures& getTextures() const
		{
			return mTextures;
		}

		//[-------------------------------------------------------]
		//[ Ease-of-use direct access                             ]
		//[-------------------------------------------------------]
		/**
		*  @brief
		*    Return the pass uniform buffer
		*
		*  @return
		*    The pass uniform buffer, can be a null pointer, don't destroy the instance
		*/
		[[nodiscard]] inline const UniformBuffer* getPassUniformBuffer() const
		{
			return mPassUniformBuffer;
		}

		/**
		*  @brief
		*    Return the material uniform buffer
		*
		*  @return
		*    The material uniform buffer, can be a null pointer, don't destroy the instance
		*/
		[[nodiscard]] inline const UniformBuffer* getMaterialUniformBuffer() const
		{
			return mMaterialUniformBuffer;
		}

		/**
		*  @brief
		*    Return the instance uniform buffer
		*
		*  @return
		*    The instance uniform buffer, can be a null pointer, don't destroy the instance
		*/
		[[nodiscard]] inline const UniformBuffer* getInstanceUniformBuffer() const
		{
			return mInstanceUniformBuffer;
		}

		/**
		*  @brief
		*    Return the instance texture buffer
		*
		*  @return
		*    The instance texture buffer, can be a null pointer, don't destroy the instance
		*/
		[[nodiscard]] inline const TextureBuffer* getInstanceTextureBuffer() const
		{
			return mInstanceTextureBuffer;
		}

		/**
		*  @brief
		*    Return the light texture buffer
		*
		*  @return
		*    The light texture buffer, can be a null pointer, don't destroy the instance
		*/
		[[nodiscard]] inline const TextureBuffer* getLightTextureBuffer() const
		{
			return mLightTextureBuffer;
		}

		//[-------------------------------------------------------]
		//[ Buffer manager                                        ]
		//[-------------------------------------------------------]
		/**
		*  @brief
		*    Return the pass buffer manager
		*
		*  @return
		*    The pass buffer manager, can be a null pointer, don't destroy the instance
		*/
		[[nodiscard]] inline PassBufferManager* getPassBufferManager() const
		{
			return mPassBufferManager;
		}

		/**
		*  @brief
		*    Return the material buffer manager
		*
		*  @return
		*    The material buffer manager, can be a null pointer, don't destroy the instance
		*/
		[[nodiscard]] inline MaterialBufferManager* getMaterialBufferManager() const
		{
			return mMaterialBufferManager;
		}

		//[-------------------------------------------------------]
		//[ Misc                                                  ]
		//[-------------------------------------------------------]
		// TODO(co) Asynchronous loading completion, we might want to move this into "RendererRuntime::IResource"
		RENDERERRUNTIME_API_EXPORT void enforceFullyLoaded();

		/**
		*  @brief
		*    Bind the graphics material blueprint resource into the given command buffer
		*
		*  @param[out] commandBuffer
		*    Command buffer to fill
		*/
		RENDERERRUNTIME_API_EXPORT void fillGraphicsCommandBuffer(Renderer::CommandBuffer& commandBuffer);

		/**
		*  @brief
		*    Bind the compute material blueprint resource into the given command buffer
		*
		*  @param[out] commandBuffer
		*    Command buffer to fill
		*/
		RENDERERRUNTIME_API_EXPORT void fillComputeCommandBuffer(Renderer::CommandBuffer& commandBuffer);

		/**
		*  @brief
		*    Create graphics and compute pipeline state cache instances for this material blueprint
		*
		*  @param[in] mandatoryOnly
		*    Do only create mandatory combinations?
		*
		*  @remarks
		*    Use mandatory only to ensure that for every material blueprint there's a pipeline state cache for the most basic pipeline state signature
		*    -> With this in place, a fallback pipeline state signature can always be found for pipeline state cache misses
		*    -> Without this, we'll end up with runtime hiccups for pipeline state cache misses
		*    In order to reduce visual artifacts, a material blueprint can define a set of shader combination properties for which pipeline
		*    state caches must also be created. Inside the JSON material blueprint files, those properties are marked by
		*    "VisualImportance": "MANDATORY". Good examples for such shader properties are albedo map or GPU skinning. It's the responsibility
		*    of the material blueprint author to keep the number of such shader properties to a bare minimum.
		*
		*    When setting mandatory only to "false", all possible combinations will be created. This might take a while, depending on the number of
		*    combinations. The creation of all possible combinations is usually done at tool-time before shipping a product. Runtime pipeline state
		*    compilation should only be the last resort for performance reasons, even if it's asynchronous.
		*
		*  @note
		*    - The material blueprint resource must be fully loaded for this to work
		*/
		RENDERERRUNTIME_API_EXPORT void createPipelineStateCaches(bool mandatoryOnly);


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		MaterialBlueprintResource();
		virtual ~MaterialBlueprintResource() override;
		explicit MaterialBlueprintResource(const MaterialBlueprintResource&) = delete;
		MaterialBlueprintResource& operator=(const MaterialBlueprintResource&) = delete;
		void onDefaultTextureFilteringChanged(Renderer::FilterMode defaultFilterMode, uint8_t maximumDefaultAnisotropy);

		//[-------------------------------------------------------]
		//[ Pipeline state object cache                           ]
		//[-------------------------------------------------------]
		void clearPipelineStateObjectCache();
		void loadPipelineStateObjectCache(IFile& file);
		[[nodiscard]] bool doesPipelineStateObjectCacheNeedSaving() const;
		void savePipelineStateObjectCache(IFile& file);

		//[-------------------------------------------------------]
		//[ "RendererRuntime::PackedElementManager" management    ]
		//[-------------------------------------------------------]
		void initializeElement(MaterialBlueprintResourceId materialBlueprintResourceId);
		void deinitializeElement();


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		GraphicsPipelineStateCacheManager mGraphicsPipelineStateCacheManager;
		ComputePipelineStateCacheManager  mComputePipelineStateCacheManager;
		MaterialProperties				  mMaterialProperties;
		ShaderProperties				  mVisualImportanceOfShaderProperties;		///< Every shader property known to the material blueprint has a visual importance entry in here
		ShaderProperties				  mMaximumIntegerValueOfShaderProperties;	///< The maximum integer value (inclusive) of a shader property
		Renderer::IRootSignaturePtr		  mRootSignaturePtr;						///< Root signature, can be a null pointer
		// Graphics pipeline state
		Renderer::GraphicsPipelineState	mGraphicsPipelineState;
		VertexAttributesResourceId		mVertexAttributesResourceId;
		ShaderBlueprintResourceId		mGraphicsShaderBlueprintResourceId[NUMBER_OF_GRAPHICS_SHADER_TYPES];
		// Compute pipeline state
		ShaderBlueprintResourceId mComputeShaderBlueprintResourceId;
		// Resource
		UniformBuffers mUniformBuffers;
		TextureBuffers mTextureBuffers;
		SamplerStates  mSamplerStates;
		Textures	   mTextures;
		// Resource groups
		Renderer::IResourceGroupPtr mSamplerStateGroup;
		// Ease-of-use direct access
		UniformBuffer* mPassUniformBuffer;		///< Can be a null pointer, don't destroy the instance
		UniformBuffer* mMaterialUniformBuffer;	///< Can be a null pointer, don't destroy the instance
		UniformBuffer* mInstanceUniformBuffer;	///< Can be a null pointer, don't destroy the instance
		TextureBuffer* mInstanceTextureBuffer;	///< Can be a null pointer, don't destroy the instance
		TextureBuffer* mLightTextureBuffer;		///< Can be a null pointer, don't destroy the instance
		// Buffer manager
		PassBufferManager*	   mPassBufferManager;		///< Pass buffer manager, can be a null pointer, destroy the instance if you no longer need it
		MaterialBufferManager* mMaterialBufferManager;	///< Material buffer manager, can be a null pointer, destroy the instance if you no longer need it


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

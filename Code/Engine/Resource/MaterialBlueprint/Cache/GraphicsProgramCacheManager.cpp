#include "stdafx.h"
#include "Resource/MaterialBlueprint/Cache/GraphicsProgramCacheManager.h"
#include "Resource/MaterialBlueprint/Cache/GraphicsProgramCache.h"
#include "Resource/MaterialBlueprint/MaterialBlueprintResourceManager.h"
#include "Resource/MaterialBlueprint/MaterialBlueprintResource.h"
#include "Resource/ShaderBlueprint/Cache/ShaderCache.h"
#include "Resource/ShaderBlueprint/ShaderBlueprintResourceManager.h"
#include "Resource/VertexAttributes/VertexAttributesResourceManager.h"
#include "Resource/VertexAttributes/VertexAttributesResource.h"
#include "Core/Math/Math.h"
#include "IRendererRuntime.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public static methods                                 ]
	//[-------------------------------------------------------]
	GraphicsProgramCacheId GraphicsProgramCacheManager::generateGraphicsProgramCacheId(const GraphicsPipelineStateSignature& graphicsPipelineStateSignature)
	{
		GraphicsProgramCacheId graphicsProgramCacheId = Math::FNV1a_INITIAL_HASH_32;
		for (uint8_t i = 0; i < NUMBER_OF_GRAPHICS_SHADER_TYPES; ++i)
		{
			const ShaderCombinationId shaderCombinationId = graphicsPipelineStateSignature.getShaderCombinationId(static_cast<GraphicsShaderType>(i));
			if (isValid(shaderCombinationId))
			{
				graphicsProgramCacheId = Math::calculateFNV1a32(reinterpret_cast<const uint8_t*>(&shaderCombinationId), sizeof(ShaderCombinationId), graphicsProgramCacheId);
			}
		}

		// Done
		return graphicsProgramCacheId;
	}


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	GraphicsProgramCache* GraphicsProgramCacheManager::getGraphicsProgramCacheByGraphicsPipelineStateSignature(const GraphicsPipelineStateSignature& graphicsPipelineStateSignature)
	{
		// Does the graphics program cache already exist?
		GraphicsProgramCache* graphicsProgramCache = nullptr;
		const GraphicsProgramCacheId graphicsProgramCacheId = generateGraphicsProgramCacheId(graphicsPipelineStateSignature);
		std::unique_lock<std::mutex> mutexLock(mMutex);
		GraphicsProgramCacheById::const_iterator iterator = mGraphicsProgramCacheById.find(graphicsProgramCacheId);
		if (iterator != mGraphicsProgramCacheById.cend())
		{
			graphicsProgramCache = iterator->second;
		}
		else
		{
			// Create the renderer program: Decide which shader language should be used (for example "GLSL" or "HLSL")
			const MaterialBlueprintResource& materialBlueprintResource = mGraphicsPipelineStateCacheManager.getMaterialBlueprintResource();
			const Renderer::IRootSignaturePtr& rootSignaturePtr = materialBlueprintResource.getRootSignaturePtr();
			Renderer::IShaderLanguage& shaderLanguage = rootSignaturePtr->getRenderer().getDefaultShaderLanguage();
			const IRendererRuntime& rendererRuntime = materialBlueprintResource.getResourceManager<MaterialBlueprintResourceManager>().getRendererRuntime();

			// Create the shaders
			ShaderCacheManager& shaderCacheManager = rendererRuntime.getShaderBlueprintResourceManager().getShaderCacheManager();
			Renderer::IShader* shaders[NUMBER_OF_GRAPHICS_SHADER_TYPES] = {};
			for (uint8_t i = 0; i < NUMBER_OF_GRAPHICS_SHADER_TYPES; ++i)
			{
				ShaderCache* shaderCache = shaderCacheManager.getGraphicsShaderCache(graphicsPipelineStateSignature, materialBlueprintResource, shaderLanguage, static_cast<GraphicsShaderType>(i));
				if (nullptr != shaderCache)
				{
					shaders[i] = shaderCache->getShaderPtr();
				}
				else
				{
					// No error, just means there's no shader cache because e.g. there's no shader of the requested type
				}
			}

			// Create the graphics program
			Renderer::IGraphicsProgram* graphicsProgram = shaderLanguage.createGraphicsProgram(*rootSignaturePtr,
				rendererRuntime.getVertexAttributesResourceManager().getById(materialBlueprintResource.getVertexAttributesResourceId()).getVertexAttributes(),
				static_cast<Renderer::IVertexShader*>(shaders[static_cast<int>(GraphicsShaderType::Vertex)]),
				static_cast<Renderer::ITessellationControlShader*>(shaders[static_cast<int>(GraphicsShaderType::TessellationControl)]),
				static_cast<Renderer::ITessellationEvaluationShader*>(shaders[static_cast<int>(GraphicsShaderType::TessellationEvaluation)]),
				static_cast<Renderer::IGeometryShader*>(shaders[static_cast<int>(GraphicsShaderType::Geometry)]),
				static_cast<Renderer::IFragmentShader*>(shaders[static_cast<int>(GraphicsShaderType::Fragment)]));
			RENDERER_SET_RESOURCE_DEBUG_NAME(graphicsProgram, "Graphics program cache manager")

			// Create the new graphics program cache instance
			if (nullptr != graphicsProgram)
			{
				graphicsProgramCache = new GraphicsProgramCache(graphicsProgramCacheId, *graphicsProgram);
				mGraphicsProgramCacheById.emplace(graphicsProgramCacheId, graphicsProgramCache);
			}
			else
			{
				// TODO(co) Error handling
				RENDERER_ASSERT(materialBlueprintResource.getResourceManager<MaterialBlueprintResourceManager>().getRendererRuntime().getContext(), false, "Invalid graphics program")
			}
		}

		// Done
		return graphicsProgramCache;
	}

	void GraphicsProgramCacheManager::clearCache()
	{
		std::unique_lock<std::mutex> mutexLock(mMutex);
		for (auto& graphicsProgramCacheElement : mGraphicsProgramCacheById)
		{
			delete graphicsProgramCacheElement.second;
		}
		mGraphicsProgramCacheById.clear();
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

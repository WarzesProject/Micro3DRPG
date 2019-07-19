#include "stdafx.h"
#include "BaseExample.h"
#include "AppRenderer.h"
#include "Framework/Color4.h"

void BaseExample::onInitialization()
{
	// Get and check the renderer instance
	Renderer::IRendererPtr renderer(getRenderer());
	if (nullptr != renderer)
	{
		// Create the buffer manager
		mBufferManager = renderer->createBufferManager();

		{ // Create the root signature
			// Setup
			Renderer::RootSignatureBuilder rootSignature;
			rootSignature.initialize(0, nullptr, 0, nullptr, Renderer::RootSignatureFlags::ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

			// Create the instance
			mRootSignature = renderer->createRootSignature(rootSignature);
		}

		// Vertex input layout
		static constexpr Renderer::VertexAttribute vertexAttributesLayout[] =
		{
			{ // Attribute 0
				// Data destination
				Renderer::VertexAttributeFormat::FLOAT_2,	// vertexAttributeFormat (Renderer::VertexAttributeFormat)
				"Position",									// name[32] (char)
				"POSITION",									// semanticName[32] (char)
				0,											// semanticIndex (uint32_t)
				// Data source
				0,											// inputSlot (uint32_t)
				0,											// alignedByteOffset (uint32_t)
				sizeof(float) * 2,							// strideInBytes (uint32_t)
				0											// instancesPerElement (uint32_t)
			}
		};
		const Renderer::VertexAttributes vertexAttributes(static_cast<uint32_t>(GLM_COUNTOF(vertexAttributesLayout)), vertexAttributesLayout);

		{ // Create vertex array object (VAO)
			// Create the vertex buffer object (VBO)
			// -> Clip space vertex positions, left/bottom is (-1,-1) and right/top is (1,1)
			static constexpr float VERTEX_POSITION[] =
			{					// Vertex ID	Triangle on screen
				 0.0f, 1.0f,	// 0				0
				 1.0f, 0.0f,	// 1			   .   .
				-0.5f, 0.0f		// 2			  2.......1
			};
			Renderer::IVertexBufferPtr vertexBuffer(mBufferManager->createVertexBuffer(sizeof(VERTEX_POSITION), VERTEX_POSITION));
			RENDERER_SET_RESOURCE_DEBUG_NAME(vertexBuffer, "Triangle VBO")

				// Create vertex array object (VAO)
				// -> The vertex array object (VAO) keeps a reference to the used vertex buffer object (VBO)
				// -> This means that there's no need to keep an own vertex buffer object (VBO) reference
				// -> When the vertex array object (VAO) is destroyed, it automatically decreases the
				//    reference of the used vertex buffer objects (VBO). If the reference counter of a
				//    vertex buffer object (VBO) reaches zero, it's automatically destroyed.
				const Renderer::VertexArrayVertexBuffer vertexArrayVertexBuffers[] = { vertexBuffer };
			mVertexArray = mBufferManager->createVertexArray(vertexAttributes, static_cast<uint32_t>(GLM_COUNTOF(vertexArrayVertexBuffers)), vertexArrayVertexBuffers);
			RENDERER_SET_RESOURCE_DEBUG_NAME(mVertexArray, "Triangle VAO")
		}

		{
			// Create the graphics program
			Renderer::IGraphicsProgramPtr graphicsProgram;
			{
				// Get the shader source code (outsourced to keep an overview)
				const char* vertexShaderSourceCode = nullptr;
				const char* fragmentShaderSourceCode = nullptr;
#ifdef RENDERER_VULKAN
				if (renderer->getNameId() == Renderer::NameId::VULKAN)
				{


					//[-------------------------------------------------------]
					//[ Vertex shader source code                             ]
					//[-------------------------------------------------------]
					// One vertex shader invocation per vertex
					vertexShaderSourceCode = R"(#version 450 core	// OpenGL 4.5

// Attribute input/output
layout(location = 0) in vec2 Position;	// Clip space vertex position as input, left/bottom is (-1,-1) and right/top is (1,1)
layout(location = 0) out gl_PerVertex
{
	vec4 gl_Position;
};

// Programs
void main()
{
	// Pass through the clip space vertex position, left/bottom is (-1,-1) and right/top is (1,1)
	gl_Position = vec4(Position, 0.5, 1.0);
}
)";


					//[-------------------------------------------------------]
					//[ Fragment shader source code                           ]
					//[-------------------------------------------------------]
					// One fragment shader invocation per fragment
					fragmentShaderSourceCode = R"(#version 450 core	// OpenGL 4.5

// Attribute input/output
layout(location = 0) out vec4 OutputColor;	// Output variable for fragment color

// Programs
void main()
{
	// Return white
	OutputColor = vec4(1.0, 1.0, 1.0, 1.0);
}
)";


					//[-------------------------------------------------------]
					//[ Shader end                                            ]
					//[-------------------------------------------------------]
				}
				else
#endif

#ifdef RENDERER_OPENGL
					if (renderer->getNameId() == Renderer::NameId::OPENGL)
					{


						//[-------------------------------------------------------]
						//[ Vertex shader source code                             ]
						//[-------------------------------------------------------]
						// One vertex shader invocation per vertex
						vertexShaderSourceCode = R"(#version 410 core	// OpenGL 4.1

// Attribute input/output
in  vec2 Position;	// Clip space vertex position as input, left/bottom is (-1,-1) and right/top is (1,1)
out gl_PerVertex
{
	vec4 gl_Position;
};

// Programs
void main()
{
	// Pass through the clip space vertex position, left/bottom is (-1,-1) and right/top is (1,1)
	gl_Position = vec4(Position, 0.5, 1.0);
}
)";


						//[-------------------------------------------------------]
						//[ Fragment shader source code                           ]
						//[-------------------------------------------------------]
						// One fragment shader invocation per fragment
						fragmentShaderSourceCode = R"(#version 410 core	// OpenGL 4.1

// Attribute input/output
out vec4 OutputColor;	// Output variable for fragment color

// Programs
void main()
{
	// Return white
	OutputColor = vec4(1.0, 1.0, 1.0, 1.0);
}
)";


						//[-------------------------------------------------------]
						//[ Shader end                                            ]
						//[-------------------------------------------------------]
					}
					else
#endif

#ifdef RENDERER_OPENGLES3
						if (renderer->getNameId() == Renderer::NameId::OPENGLES3)
						{


							//[-------------------------------------------------------]
							//[ Vertex shader source code                             ]
							//[-------------------------------------------------------]
							// One vertex shader invocation per vertex
							vertexShaderSourceCode = R"(#version 300 es	// OpenGL ES 3.0

// Attribute input/output
in highp vec2 Position;	// Clip space vertex position as input, left/bottom is (-1,-1) and right/top is (1,1)

// Programs
void main()
{
	// Pass through the clip space vertex position, left/bottom is (-1,-1) and right/top is (1,1)
	gl_Position = vec4(Position, 0.5, 1.0);
}
)";


							//[-------------------------------------------------------]
							//[ Fragment shader source code                           ]
							//[-------------------------------------------------------]
							// One fragment shader invocation per fragment
							fragmentShaderSourceCode = R"(#version 300 es	// OpenGL ES 3.0
precision highp float;

// Attribute input/output
out highp vec4 OutputColor;	// Output variable for fragment color

// Programs
void main()
{
	// Return white
	OutputColor = vec4(1.0, 1.0, 1.0, 1.0);
}
)";


							//[-------------------------------------------------------]
							//[ Shader end                                            ]
							//[-------------------------------------------------------]
						}
						else
#endif

#if defined(RENDERER_DIRECT3D9) || defined(RENDERER_DIRECT3D10) || defined(RENDERER_DIRECT3D11) || defined(RENDERER_DIRECT3D12)
							if (renderer->getNameId() == Renderer::NameId::DIRECT3D9 || renderer->getNameId() == Renderer::NameId::DIRECT3D10 || renderer->getNameId() == Renderer::NameId::DIRECT3D11 || renderer->getNameId() == Renderer::NameId::DIRECT3D12)
							{


								//[-------------------------------------------------------]
								//[ Vertex shader source code                             ]
								//[-------------------------------------------------------]
								// One vertex shader invocation per vertex
								vertexShaderSourceCode = R"(
// Attribute input/output
struct VS_OUTPUT
{
	float4 Position : SV_POSITION;	// Clip space vertex position as output, left/bottom is (-1,-1) and right/top is (1,1)
};

// Programs
VS_OUTPUT main(float2 Position : POSITION)	// Clip space vertex position as input, left/bottom is (-1,-1) and right/top is (1,1)
{
	// Pass through the clip space vertex position, left/bottom is (-1,-1) and right/top is (1,1)
	VS_OUTPUT output;
	output.Position = float4(Position, 0.5f, 1.0f);
	return output;
}
)";


								//[-------------------------------------------------------]
								//[ Fragment shader source code                           ]
								//[-------------------------------------------------------]
								// One fragment shader invocation per fragment
								// "pixel shader" in Direct3D terminology
								fragmentShaderSourceCode = R"(
// Programs
float4 main(float4 Position : SV_POSITION) : SV_TARGET
{
	// Return white
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
)";


								//[-------------------------------------------------------]
								//[ Shader end                                            ]
								//[-------------------------------------------------------]
							}
							else
#endif

#ifdef RENDERER_NULL
								if (renderer->getNameId() == Renderer::NameId::NULL_DUMMY)
								{
									vertexShaderSourceCode = fragmentShaderSourceCode = "42";
								}
								else
#endif
								{
									// Error! (unsupported renderer)
									RENDERER_LOG(renderer->getContext(), CRITICAL, "Error: Unsupported renderer")
								}


				// Create the vertex shader
				Renderer::IShaderLanguage& shaderLanguage = renderer->getDefaultShaderLanguage();
				Renderer::IVertexShader* vertexShader = shaderLanguage.createVertexShaderFromSourceCode(vertexAttributes, vertexShaderSourceCode);
				RENDERER_SET_RESOURCE_DEBUG_NAME(vertexShader, "Triangle VS")

					// Create the fragment shader
					Renderer::IFragmentShader* fragmentShader = shaderLanguage.createFragmentShaderFromSourceCode(fragmentShaderSourceCode);
				RENDERER_SET_RESOURCE_DEBUG_NAME(fragmentShader, "Triangle FS")

					// Create the graphics program
					graphicsProgram = shaderLanguage.createGraphicsProgram(*mRootSignature, vertexAttributes, vertexShader, fragmentShader);
				RENDERER_SET_RESOURCE_DEBUG_NAME(graphicsProgram, "Triangle graphics program")
			}

			// Create the graphics pipeline state object (PSO)
			if (nullptr != graphicsProgram)
			{
				mGraphicsPipelineState = renderer->createGraphicsPipelineState(Renderer::GraphicsPipelineStateBuilder(mRootSignature, graphicsProgram, vertexAttributes, getMainRenderTarget()->getRenderPass()));
				RENDERER_SET_RESOURCE_DEBUG_NAME(mGraphicsPipelineState, "Triangle PSO")
			}
		}

		// Since we're always submitting the same commands to the renderer, we can fill the command buffer once during initialization and then reuse it multiple times during runtime
		fillCommandBuffer();
	}
}

void BaseExample::onDeinitialization()
{
	// Release the used resources
	mVertexArray = nullptr;
	mGraphicsPipelineState = nullptr;
	mRootSignature = nullptr;
	mCommandBuffer.clear();
	mBufferManager = nullptr;
}

void BaseExample::onDraw()
{
	// Get and check the renderer instance
	Renderer::IRendererPtr renderer(getRenderer());
	if (nullptr != renderer)
	{
		// Submit command buffer to the renderer backend
		mCommandBuffer.submitToRenderer(*renderer);
	}
}

void BaseExample::fillCommandBuffer()
{
	// Sanity checks
	assert(mCommandBuffer.isEmpty());
	assert(nullptr != mRootSignature);
	assert(nullptr != mGraphicsPipelineState);
	assert(nullptr != mVertexArray);

	// Scoped debug event
	COMMAND_SCOPED_DEBUG_EVENT_FUNCTION(mCommandBuffer)

		// Clear the graphics color buffer of the current render target with gray, do also clear the depth buffer
		Renderer::Command::ClearGraphics::create(mCommandBuffer, Renderer::ClearFlag::COLOR_DEPTH, Color4::GRAY);

	// Set the used graphics root signature
	Renderer::Command::SetGraphicsRootSignature::create(mCommandBuffer, mRootSignature);

	// Set the used graphics pipeline state object (PSO)
	Renderer::Command::SetGraphicsPipelineState::create(mCommandBuffer, mGraphicsPipelineState);

	// Input assembly (IA): Set the used vertex array
	Renderer::Command::SetGraphicsVertexArray::create(mCommandBuffer, mVertexArray);

	// Set debug marker
	// -> Debug methods: When using Direct3D <11.1, these methods map to the Direct3D 9 PIX functions
	//    (D3DPERF_* functions, also works directly within VisualStudio 2017 out-of-the-box)
	COMMAND_SET_DEBUG_MARKER(mCommandBuffer, "Everyone ready for the upcoming triangle?")

	{
		// Scoped debug event
		COMMAND_SCOPED_DEBUG_EVENT(mCommandBuffer, "Drawing the fancy triangle")

			// Render the specified geometric primitive, based on an array of vertices
			Renderer::Command::DrawGraphics::create(mCommandBuffer, 3);
	}
}

Renderer::IRenderer* BaseExample::getRenderer() const
{
	return (nullptr != mApplicationFrontend) ? mApplicationFrontend->getRenderer() : nullptr;
}

Renderer::IRenderTarget* BaseExample::getMainRenderTarget() const
{
	return (nullptr != mApplicationFrontend) ? mApplicationFrontend->getMainRenderTarget() : nullptr;
}

void BaseExample::exit()
{
	if (nullptr != mApplicationFrontend)
	{
		mApplicationFrontend->exit();
	}
}
#ifdef RENDERER_VULKAN
if (renderer.getNameId() == Renderer::NameId::VULKAN)
{


//[-------------------------------------------------------]
//[ Vertex shader source code                             ]
//[-------------------------------------------------------]
// One vertex shader invocation per vertex
vertexShaderSourceCode = R"(#version 450 core	// OpenGL 4.5

// Attribute input/output
layout(location = 0) in vec2 Position;	// Object space vertex position as input, left/bottom is (0,0) and right/top is (1,1)
layout(location = 0) out gl_PerVertex
{
	vec4 gl_Position;
};
layout(location = 1) in  vec2 TexCoord;		// Normalized texture coordinate as input
layout(location = 1) out vec2 TexCoordVs;	// Normalized texture coordinate as output
layout(location = 2) in  vec4 Color;		// Color as input
layout(location = 2) out vec4 ColorVs;		// Color as output

// Uniforms
layout(std140, set = 0, binding = 0) uniform UniformBlockDynamicVs
{
	mat4 ObjectSpaceToClipSpaceMatrix;
};

// Programs
void main()
{
	// Calculate the clip space vertex position, lower/left is (-1,-1) and upper/right is (1,1)
	gl_Position = ObjectSpaceToClipSpaceMatrix * vec4(Position, 0.5, 1.0);

	// Pass through the vertex texture coordinate
	TexCoordVs = TexCoord;

	// Pass through the vertex color
	ColorVs = Color;
}
)";


//[-------------------------------------------------------]
//[ Fragment shader source code                           ]
//[-------------------------------------------------------]
// One fragment shader invocation per fragment
fragmentShaderSourceCode = R"(#version 450 core	// OpenGL 4.5

// Attribute input/output
layout(location = 1) in  vec2 TexCoordVs;	// Normalized texture coordinate as input
layout(location = 2) in  vec4 ColorVs;
layout(location = 0) out vec4 OutputColor;	// Output variable for fragment color

// Uniforms
layout(set = 0, binding = 1) uniform sampler2D GlyphMap;	// Glyph atlas texture map

// Programs
void main()
{
	// Fetch the texel at the given texture coordinate and return its color
	OutputColor = ColorVs * texture(GlyphMap, TexCoordVs).rrrr;
}
)";


//[-------------------------------------------------------]
//[ Shader end                                            ]
//[-------------------------------------------------------]
}
else
#endif

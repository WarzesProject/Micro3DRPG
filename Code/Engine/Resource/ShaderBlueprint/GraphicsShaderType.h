#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Core/Platform/PlatformTypes.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4668)	// warning C4668: '_M_HYBRID_X86_ARM64' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
	#include <inttypes.h>	// For uint32_t, uint64_t etc.
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	enum class GraphicsShaderType : uint8_t
	{
		Vertex,					///< VS
		TessellationControl,	///< TCS, "hull shader" in Direct3D terminology
		TessellationEvaluation,	///< TES, "domain shader" in Direct3D terminology
		Geometry,				///< GS
		Fragment				///< FS, "pixel shader" in Direct3D terminology
	};
	static constexpr uint8_t NUMBER_OF_GRAPHICS_SHADER_TYPES = static_cast<uint8_t>(GraphicsShaderType::Fragment) + 1;	///< Total number of graphics shader types


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

#pragma once

#include "Core/Platform/PlatformTypes.h"

// Is this project build as shared library?
#ifdef SHARED_LIBRARIES
	// Build as shared library
	#ifdef RENDERER_RUNTIME_EXPORTS
		// Export
		#define RENDERERRUNTIME_API_EXPORT		GENERIC_API_EXPORT
		#define RENDERERRUNTIME_FUNCTION_EXPORT	GENERIC_FUNCTION_EXPORT
	#else
		// Import
		#define RENDERERRUNTIME_API_EXPORT		GENERIC_API_IMPORT
		#define RENDERERRUNTIME_FUNCTION_EXPORT
	#endif
#else
	// Build as static library
	#define RENDERERRUNTIME_API_EXPORT
	#define RENDERERRUNTIME_FUNCTION_EXPORT
#endif
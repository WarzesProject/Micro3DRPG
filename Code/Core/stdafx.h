#pragma once

//=============================================================================
// engine first header
//=============================================================================
#include "PlatformCompiler.h"
#include "Macros.h"
#include "EngineConfig.h"

//=============================================================================
// STL header and config
//=============================================================================
#include <cstdint>

//=============================================================================
// Platform header and config
//=============================================================================
//#if SE_PLATFORM_WINDOWS
//#	undef _WIN32_WINNT
//#	undef WINVER
//#	define _WIN32_WINNT SE_PLATFORM_WINDOWS
//#	define WINVER _WIN32_WINNT
//
//#	define WIN32_LEAN_AND_MEAN
//#	define WIN32_EXTRA_LEAN
//#	define NOMINMAX
//#endif

//=============================================================================
// 3rd-party header
//=============================================================================
#if SE_COMPILER_MSVC
#	pragma warning(push, 1)
#endif

#define GLM_FORCE_CXX17
#define GLM_FORCE_INLINE
#define GLM_FORCE_SIMD_AVX2
#define GLM_FORCE_XYZW_ONLY
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL

#if SE_COMPILER_MSVC
#	pragma warning(pop)
#endif
//=============================================================================
// engine other header
//=============================================================================
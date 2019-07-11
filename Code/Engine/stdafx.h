#pragma once

//=============================================================================
// engine first header
//=============================================================================
#include "Core/PlatformCompiler.h"
#include "Core/Macros.h"
#include "Core/EngineConfig.h"

//=============================================================================
// STL header and config
//=============================================================================
#include <cstdint>

//=============================================================================
// Platform header and config
//=============================================================================
#if SE_PLATFORM_WINDOWS
#	undef _WIN32_WINNT
#	undef WINVER
#	define _WIN32_WINNT 0x0603
#	define WINVER _WIN32_WINNT
#	define WIN32_LEAN_AND_MEAN
#	define WIN32_EXTRA_LEAN
#	define NOMINMAX
#endif

//=============================================================================
// 3rd-party header
//=============================================================================
#if SE_COMPILER_MSVC
#	pragma warning(push, 1)
#endif

#if SE_COMPILER_MSVC
#	pragma warning(pop)
#endif
//=============================================================================
// engine other header
//=============================================================================
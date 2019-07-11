#pragma once

// Architecture
#define SE_ARCH_32BIT              0
#define SE_ARCH_64BIT              0

// Debug
#define SE_DEBUG                   0

// Compiler
#define SE_COMPILER_MSVC           0

// Platform
#define SE_PLATFORM_WINDOWS        0

//=============================================================================
// Architecture
//=============================================================================
#if defined(__x86_64__)    || \
	defined(_M_X64)        || \
	defined(__aarch64__)   || \
	defined(__64BIT__)     || \
	defined(__mips64)      || \
	defined(__powerpc64__) || \
	defined(__ppc64__)     || \
	defined(__LP64__)
#	undef  SE_ARCH_64BIT
#	define SE_ARCH_64BIT 64
#else
#	undef  SE_ARCH_32BIT
#	define SE_ARCH_32BIT 32
#endif

//=============================================================================
// DEBUG MODE
//=============================================================================
// Win32 compilers use _DEBUG for specifying debug builds.
// for MinGW, we set DEBUG
#if defined(_DEBUG) || defined(DEBUG)
#	undef  SE_DEBUG
#	define SE_DEBUG 1
#endif

//=============================================================================
// Compiler
//=============================================================================
#if defined(_MSC_VER)
#	undef  SE_COMPILER_MSVC
#	define SE_COMPILER_MSVC _MSC_VER
#else
#	error "Unknown compiler"
#endif

//=============================================================================
// Platform
//=============================================================================
#if defined(_WIN32) || defined(_WIN64)
#	undef  SE_PLATFORM_WINDOWS
#	define SE_PLATFORM_WINDOWS 1
#else
#	error "Unknown platform"
#endif
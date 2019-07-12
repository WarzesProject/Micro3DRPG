#pragma once


//[-------------------------------------------------------]
//[ C++ compiler keywords                                 ]
//[-------------------------------------------------------]
#ifdef _WIN32
	/**
	*  @brief
	*    Force the compiler to inline something
	*
	*  @note
	*    - Do only use this when you really have to, usually it's best to let the compiler decide by using the standard keyword "inline"
	*/
	#define FORCEINLINE __forceinline

	/**
	*  @brief
	*    Restrict alias
	*/
	#define RESTRICT __restrict
	#define RESTRICT_RETURN __restrict

	/**
	*  @brief
	*    No operation macro ("_asm nop"/"__nop()")
	*/
	#define NOP __nop()

	/**
	*  @brief
	*    Debug break operation macro
	*/
	#define DEBUG_BREAK __debugbreak()

	/**
	*  @brief
	*    Platform specific "#pragma warning(push)" (Microsoft Windows Visual Studio)
	*/
	#define PRAGMA_WARNING_PUSH __pragma(warning(push))

	/**
	*  @brief
	*    Platform specific "#pragma warning(pop)" (Microsoft Windows Visual Studio)
	*/
	#define PRAGMA_WARNING_POP __pragma(warning(pop))

	/**
	*  @brief
	*    Platform specific "#pragma warning(disable: <x>)" (Microsoft Windows Visual Studio)
	*/
	#define PRAGMA_WARNING_DISABLE_MSVC(id) __pragma(warning(disable: id))

	/**
	*  @brief
	*    Platform specific "#pragma clang diagnostic ignored <x>" (Clang)
	*/
	#define PRAGMA_WARNING_DISABLE_CLANG(id)

	/**
	*  @brief
	*    Platform specific "#pragma GCC diagnostic ignored <x>" (GCC)
	*/
	#define PRAGMA_WARNING_DISABLE_GCC(id)
#elif LINUX
	/**
	*  @brief
	*    Force the compiler to inline something
	*
	*  @note
	*    - Do only use this when you really have to, usually it's best to let the compiler decide by using the standard keyword "inline"
	*/
	#define FORCEINLINE __attribute__((always_inline))

	/**
	*  @brief
	*    Restrict alias
	*/
	#define RESTRICT __restrict__
	#define RESTRICT_RETURN

	/**
	*  @brief
	*    No operation macro ("_asm nop"/__nop())
	*/
	#define NOP asm ("nop");

	/**
	*  @brief
	*    Debug break operation macro
	*/
	#define DEBUG_BREAK __builtin_trap()

	#ifdef __clang__
		/**
		*  @brief
		*    Platform specific "#pragma clang diagnostic push" (Clang)
		*/
		#define PRAGMA_WARNING_PUSH _Pragma("clang diagnostic push")

		/**
		*  @brief
		*    Platform specific "#pragma clang diagnostic pop" (Clang)
		*/
		#define PRAGMA_WARNING_POP _Pragma("clang diagnostic pop")

		/**
		*  @brief
		*    Platform specific "#pragma warning(disable: <x>)" (Microsoft Windows Visual Studio)
		*/
		#define PRAGMA_WARNING_DISABLE_MSVC(id)

		/**
		*  @brief
		*    Platform specific "#pragma GCC diagnostic ignored <x>" (GCC)
		*/
		#define PRAGMA_WARNING_DISABLE_GCC(id)

		/**
		*  @brief
		*    Platform specific "#pragma clang diagnostic ignored <x>" (Clang)
		*/
		// We need stringify because _Pragma expects an string literal
		#define PRAGMA_STRINGIFY(a) #a
		#define PRAGMA_WARNING_DISABLE_CLANG(id) _Pragma(PRAGMA_STRINGIFY(clang diagnostic ignored id) )
	#elif __GNUC__
		// gcc
		/**
		*  @brief
		*    Platform specific "#pragma GCC diagnostic push" (GCC)
		*/
		#define PRAGMA_WARNING_PUSH _Pragma("GCC diagnostic push")

		/**
		*  @brief
		*    Platform specific "#pragma warning(pop)" (GCC)
		*/
		#define PRAGMA_WARNING_POP _Pragma("GCC diagnostic pop")

		/**
		*  @brief
		*    Platform specific "#pragma warning(disable: <x>)" (Microsoft Windows Visual Studio)
		*/
		#define PRAGMA_WARNING_DISABLE_MSVC(id)

		/**
		*  @brief
		*    Platform specific "#pragma GCC diagnostic ignored <x>" (GCC)
		*/
		// We need stringify because _Pragma expects an string literal
		#define PRAGMA_STRINGIFY(a) #a
		#define PRAGMA_WARNING_DISABLE_GCC(id) _Pragma(PRAGMA_STRINGIFY(GCC diagnostic ignored id) )

		/**
		*  @brief
		*    Platform specific "#pragma clang diagnostic ignored <x>" (Clang)
		*/
		#define PRAGMA_WARNING_DISABLE_CLANG(id)
	#else
		#error "Unsupported compiler"
	#endif
#else
	#error "Unsupported platform"
#endif


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4668)	// warning C4668: '_M_HYBRID_X86_ARM64' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
	#include <inttypes.h>	// For uint32_t, uint64_t etc.
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Data types                                            ]
//[-------------------------------------------------------]
#ifdef _WIN32
	#ifdef ARCHITECTURE_X64
		typedef unsigned __int64 handle;	// Replacement for nasty Microsoft Windows stuff leading to header chaos
	#else
		typedef unsigned __int32 handle;	// Replacement for nasty Microsoft Windows stuff leading to header chaos
	#endif
	#ifndef NULL_HANDLE
		#define NULL_HANDLE 0
	#endif
#elif LINUX
	#ifdef ARCHITECTURE_X64
		typedef uint64_t handle;
	#else
		typedef uint32_t handle;
	#endif
	#ifndef NULL_HANDLE
		#define NULL_HANDLE 0
	#endif
#else
	#error "Unsupported platform"
#endif

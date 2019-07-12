// TODO(co) The C++17 filesystem is still experimental and hence not handled in an uniform way cross platform. Until C++17 has been released on all targeted platforms, we need to use those helper.
#pragma once

#include "Export.h"


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#ifdef _WIN32
	// Disable warnings in external headers, we can't fix them
	__pragma(warning(push))
		__pragma(warning(disable: 4365))	// warning C4365: 'return': conversion from 'int' to 'std::_Rand_urng_from_func::result_type', signed/unsigned mismatch
		__pragma(warning(disable: 4530))	// warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
		__pragma(warning(disable: 4548))	// warning C4548: expression before comma has no effect; expected expression with side-effect
		__pragma(warning(disable: 4571))	// warning C4571: Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
		__pragma(warning(disable: 4625))	// warning C4625: 'std::messages_base': copy constructor was implicitly defined as deleted
		__pragma(warning(disable: 4626))	// warning C4626: 'std::messages<char>': assignment operator was implicitly defined as deleted
		__pragma(warning(disable: 4774))	// warning C4774: '_scprintf' : format string expected in argument 1 is not a string literal
		__pragma(warning(disable: 5026))	// warning C5026: 'std::messages_base': move constructor was implicitly defined as deleted
		__pragma(warning(disable: 5027))	// warning C5027: 'std::messages_base': move assignment operator was implicitly defined as deleted
		#include <filesystem>
	__pragma(warning(pop))
	namespace std_filesystem = std::filesystem;
#elif LINUX
	#include <experimental/filesystem>
	namespace std_filesystem = std::experimental::filesystem;
#else
	#error "Unsupported platform"
#endif


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	class FileSystemHelper final
	{


	//[-------------------------------------------------------]
	//[ Public static methods                                 ]
	//[-------------------------------------------------------]
	public:
		[[nodiscard]] RENDERERRUNTIME_API_EXPORT static std_filesystem::path lexicallyNormal(const std_filesystem::path& path);


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		FileSystemHelper(const FileSystemHelper&) = delete;
		FileSystemHelper& operator=(const FileSystemHelper&) = delete;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

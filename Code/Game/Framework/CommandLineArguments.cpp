#include "stdafx.h"
#include "Framework/CommandLineArguments.h"
#ifdef _WIN32
	#include <Engine/Core/Platform/WindowsHeader.h>

	#ifndef UNICODE
		PRAGMA_WARNING_PUSH
			PRAGMA_WARNING_DISABLE_MSVC(4365)	// warning C4365: 'initializing': conversion from 'int' to '::size_t', signed/unsigned mismatch
			PRAGMA_WARNING_DISABLE_MSVC(4774)	// warning C4774: '_scprintf' : format string expected in argument 1 is not a string literal
			#include <sstream>
			#include <iterator>
			#include <algorithm>
		PRAGMA_WARNING_POP
	#endif
#endif
#include <Windows.h>
#include <shellapi.h>


//[-------------------------------------------------------]
//[ Public methods                                        ]
//[-------------------------------------------------------]
CommandLineArguments::CommandLineArguments()
{
#if _WIN32
	#ifdef UNICODE
		int wargc = 0;
		wchar_t** wargv = ::CommandLineToArgvW(GetCommandLineW(), &wargc);
		if (wargc > 0)
		{
			// argv[0] is the path+name of the program
			// -> Ignore it
			mArguments.reserve(static_cast<size_t>(wargc - 1));
			std::vector<std::wstring_view> lines(wargv + 1, wargv + wargc);
			for (std::vector<std::wstring_view>::iterator iterator = lines.begin(); iterator != lines.end(); ++iterator)
			{
				// Convert UTF-16 string to UTF-8
				std::string utf8Line;
				utf8Line.resize(static_cast<size_t>(::WideCharToMultiByte(CP_UTF8, 0, iterator->data(), static_cast<int>(iterator->size()), nullptr, 0, nullptr, nullptr)));
				::WideCharToMultiByte(CP_UTF8, 0, iterator->data(), static_cast<int>(iterator->size()), utf8Line.data(), static_cast<int>(utf8Line.size()), nullptr, nullptr);

				// Backup argument
				mArguments.push_back(utf8Line);
			}
		}
		::LocalFree(wargv);
	#else
		std::string_view cmdLine(::GetCommandLineA());
		std::istringstream ss(cmdLine);
		std::istream_iterator<std::string_view> iss(ss);

		// The first token is the path+name of the program
		// -> Ignore it
		++iss;
		std::copy(iss,
			 std::istream_iterator<std::string_view>(),
			 std::back_inserter<std::vector<std::string_view>>(mArguments));
	#endif
#endif
}

#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Framework/PlatformTypes.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4365)	// warning C4365: 'argument': conversion from 'long' to 'unsigned int', signed/unsigned mismatch
	PRAGMA_WARNING_DISABLE_MSVC(4574)	// warning C4574: '_HAS_ITERATOR_DEBUGGING' is defined to be '0': did you mean to use '#if _HAS_ITERATOR_DEBUGGING'?
	PRAGMA_WARNING_DISABLE_MSVC(4571)	// warning C4571: Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
	PRAGMA_WARNING_DISABLE_MSVC(4625)	// warning C4625: 'std::codecvt_base': copy constructor was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(4626)	// warning C4626: 'std::codecvt<char16_t,char,_Mbstatet>': assignment operator was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(4668)	// warning C4668: '_M_HYBRID_X86_ARM64' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
	PRAGMA_WARNING_DISABLE_MSVC(4774)	// warning C4774: 'sprintf_s' : format string expected in argument 3 is not a string literal
	PRAGMA_WARNING_DISABLE_MSVC(5026)	// warning C5026: 'std::_Generic_error_category': move constructor was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(5027)	// warning C5027: 'std::_Generic_error_category': move assignment operator was implicitly defined as deleted
	#include <vector>
	#include <string>
	#include <string_view>
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Classes                                               ]
//[-------------------------------------------------------]
/**
*  @brief
*    Holds the command line arguments of an program (as UTF-8 strings)
*/
class CommandLineArguments final
{


//[-------------------------------------------------------]
//[ Public definitions                                    ]
//[-------------------------------------------------------]
public:
	typedef std::vector<std::string> Arguments;


//[-------------------------------------------------------]
//[ Public methods                                        ]
//[-------------------------------------------------------]
public:
	/**
	*  @brief
	*    Default constructor
	*
	*  @note
	*    - Uses "GetCommandLineW()" under Microsoft Windows to get the command line parameters for the program
	*/
	CommandLineArguments();

	/**
	*  @brief
	*    Constructor
	*
	*  @remarks
	*    Reads the command line parameters via the parameters "argc" and "argv"
	*  @param[in] argc
	*    Count of arguments pointed by "argv"
	*  @param[in] argv
	*    List of arguments
	*/
	inline CommandLineArguments(int argc, char** argv) :
		mArguments(argv + 1, argv + argc)
	{
		// Nothing here
	}

	/**
	*  @brief
	*    Return the arguments
	*
	*  @return
	*    The arguments
	*/
	[[nodiscard]] inline const Arguments& getArguments() const
	{
		return mArguments;
	}

	/**
	*  @brief
	*    Return the amount of arguments
	*
	*  @return
	*    The amount of arguments hold by this instance
	*/
	[[nodiscard]] inline uint32_t getCount() const
	{
		return static_cast<uint32_t>(mArguments.size());
	}

	/**
	*  @brief
	*    Return the argument at given index
	*
	*  @param[in] index
	*    The index of the argument to be returned
	*
	*  @return
	*    The argument at the given index or an empty string when index is out of range
	*/
	[[nodiscard]] inline std::string getArgumentAtIndex(uint32_t index) const
	{
		return (index >= mArguments.size()) ? "" : mArguments[index];
	}


//[-------------------------------------------------------]
//[ Private data                                          ]
//[-------------------------------------------------------]
private:
	Arguments mArguments;	///< List of arguments as UTF-8 strings


};

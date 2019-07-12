#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Core/Platform/PlatformTypes.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4668)	// warning C4668: '_M_HYBRID_X86_ARM64' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
	#include <limits>	// For "std::numeric_limits()"
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	/**
	*  @brief
	*    Return invalid default value for a given type
	*/
	template <typename TYPE>
	[[nodiscard]] TYPE getInvalid()
	{
		return std::numeric_limits<TYPE>::max();
	}

	/**
	*  @brief
	*    Return invalid default value for a type provided by a variable
	*/
	template <typename TYPE>
	[[nodiscard]] TYPE getInvalid(const TYPE&)
	{
		return std::numeric_limits<TYPE>::max();
	}

	/**
	*  @brief
	*    Set the given variable to invalid default value
	*/
	template <typename TYPE>
	void setInvalid(TYPE& value)
	{
		value = getInvalid<TYPE>();
	}

	/**
	*  @brief
	*    Check whether or not the given variable has not the invalid default value
	*/
	template <typename TYPE>
	[[nodiscard]] bool isValid(const TYPE& value)
	{
		return (getInvalid<TYPE>() != value);
	}

	/**
	*  @brief
	*    Check whether or not the given variable has the invalid default value
	*/
	template <typename TYPE>
	[[nodiscard]] bool isInvalid(const TYPE& value)
	{
		return (getInvalid<TYPE>() == value);
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime

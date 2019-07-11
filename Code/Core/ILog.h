#pragma once

namespace Renderer
{

	/**
	*  @brief
	*    Abstract log interface
	*
	*  @note
	*    - The implementation must be multithreading safe since the renderer is allowed to internally use multiple threads
	*/
	class ILog
	{

		// Public definitions
	public:
		/**
		*  @brief
		*    Log message type
		*/
		enum class Type
		{
			TRACE,					///< Trace, also known as verbose logging
			DEBUG,					///< Debug
			INFORMATION,			///< Information
			WARNING,				///< General warning
			PERFORMANCE_WARNING,	///< Performance related warning
			COMPATIBILITY_WARNING,	///< Compatibility related warning
			CRITICAL				///< Critical
		};

		// Public virtual Renderer::ILog methods
	public:
		/**
		*  @brief
		*    Print a formatted log message
		*
		*  @param[in] type
		*    Log message type
		*  @param[in] attachment
		*    Optional attachment (for example build shader source code), can be a null pointer
		*  @param[in] file
		*    File as ASCII string
		*  @param[in] line
		*    Line number
		*  @param[in] format
		*    "snprintf"-style formatted UTF-8 log message
		*
		*  @return
		*    "true" to request debug break, else "false"
		*/
		[[nodiscard]] virtual bool print(Type type, const char* attachment, const char* file, uint32_t line, const char* format, ...) = 0;

		// Protected methods
	protected:
		inline ILog()
		{}

		inline virtual ~ILog()
		{}

		explicit ILog(const ILog&) = delete;
		ILog& operator=(const ILog&) = delete;

	};
}

// Macros & definitions
/**
*  @brief
*    Ease-of-use log macro
*
*  @param[in] context
*    Renderer context to ask for the log interface
*  @param[in] type
*    Log message type
*  @param[in] format
*    "snprintf"-style formatted UTF-8 log message
*
*  @note
*    - Example: RENDERER_LOG(mContext, DEBUG, "Direct3D 11 renderer backend startup")
*    - See http://cnicholson.net/2009/02/stupid-c-tricks-adventures-in-assert/ - "2.  Wrap your macros in do { … } while(0)." for background information about the do-while wrap
*/
#define RENDERER_LOG(context, type, format, ...) \
		do \
		{ \
			if ((context).getLog().print(Renderer::ILog::Type::type, nullptr, __FILE__, static_cast<uint32_t>(__LINE__), format, ##__VA_ARGS__)) \
			{ \
				DEBUG_BREAK; \
			} \
		} while (0);
#define RENDERER_LOG_ONCE(context, type, format, ...) \
		do \
		{ \
			static bool loggedOnce = false; \
			if (!loggedOnce) \
			{ \
				loggedOnce = true; \
				if ((context).getLog().print(Renderer::ILog::Type::type, nullptr, __FILE__, static_cast<uint32_t>(__LINE__), format, ##__VA_ARGS__)) \
				{ \
					DEBUG_BREAK; \
				} \
			} \
		} while (0);
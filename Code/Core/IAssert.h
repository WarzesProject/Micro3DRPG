#pragma once

namespace Renderer
{


	/**
	*  @brief
	*    Abstract assert interface
	*
	*  @note
	*    - The implementation must be multithreading safe since the renderer is allowed to internally use multiple threads
	*/
	class IAssert
	{

		// Public virtual Renderer::IAssert methods
	public:
		/**
		*  @brief
		*    Handle assert
		*
		*  @param[in] expression
		*    Expression as ASCII string
		*  @param[in] file
		*    File as ASCII string
		*  @param[in] line
		*    Line number
		*  @param[in] format
		*    "snprintf"-style formatted UTF-8 assert message
		*
		*  @return
		*    "true" to request debug break, else "false"
		*/
		[[nodiscard]] virtual bool handleAssert(const char* expression, const char* file, uint32_t line, const char* format, ...) = 0;

		// Protected methods
	protected:
		inline IAssert()
		{}

		inline virtual ~IAssert()
		{}

		explicit IAssert(const IAssert&) = delete;
		IAssert& operator=(const IAssert&) = delete;

	};
}

// Macros & definitions
/**
*  @brief
*    Ease-of-use assert macro
*
*  @param[in] context
*    Renderer context to ask for the assert interface
*  @param[in] expression
*    Expression which must be true, else the assert triggers
*  @param[in] format
*    "snprintf"-style formatted UTF-8 assert message
*
*  @note
*    - Example: RENDERER_ASSERT(mContext, isInitialized, "Direct3D 11 renderer backend assert failed")
*    - See http://cnicholson.net/2009/02/stupid-c-tricks-adventures-in-assert/ - "2.  Wrap your macros in do { … } while(0)." for background information about the do-while wrap
*/
#ifdef RENDERER_DEBUG
#define RENDERER_ASSERT(context, expression, format, ...) \
			do \
			{ \
				if (!(expression) && (context).getAssert().handleAssert(#expression, __FILE__, static_cast<uint32_t>(__LINE__), format, ##__VA_ARGS__)) \
				{ \
					DEBUG_BREAK; \
				} \
			} while (0);
#define RENDERER_ASSERT_ONCE(context, expression, format, ...) \
			do \
			{ \
				static bool loggedOnce = false; \
				if (!loggedOnce && !(expression)) \
				{ \
					loggedOnce = true; \
					if ((context).getAssert().handleAssert(#expression, __FILE__, static_cast<uint32_t>(__LINE__), format, ##__VA_ARGS__)) \
					{ \
						DEBUG_BREAK; \
					} \
				} \
			} while (0);
#else
#define RENDERER_ASSERT(context, expression, format, ...)
#define RENDERER_ASSERT_ONCE(context, expression, format, ...)
#endif
#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Framework/PlatformTypes.h"
#include "Framework/IApplicationRenderer.h"
#include "Framework/IApplicationRendererRuntime.h"

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4365)	// warning C4365: 'argument': conversion from 'long' to 'unsigned int', signed/unsigned mismatch
	PRAGMA_WARNING_DISABLE_MSVC(4571)	// warning C4571: Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
	PRAGMA_WARNING_DISABLE_MSVC(4574)	// warning C4574: '_HAS_ITERATOR_DEBUGGING' is defined to be '0': did you mean to use '#if _HAS_ITERATOR_DEBUGGING'?
	PRAGMA_WARNING_DISABLE_MSVC(4625)	// warning C4625: 'std::codecvt_base': copy constructor was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(4626)	// warning C4626: 'std::codecvt<char16_t,char,_Mbstatet>': assignment operator was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(4668)	// warning C4668: '_M_HYBRID_X86_ARM64' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
	PRAGMA_WARNING_DISABLE_MSVC(4774)	// warning C4774: 'sprintf_s' : format string expected in argument 3 is not a string literal
	PRAGMA_WARNING_DISABLE_MSVC(5026)	// warning C5026: 'std::_Generic_error_category': move constructor was implicitly defined as deleted
	PRAGMA_WARNING_DISABLE_MSVC(5027)	// warning C5027: 'std::_Generic_error_category': move assignment operator was implicitly defined as deleted
	#include <map>
	#include <set>
	#include <vector>
	#include <string>
	#include <string_view>
PRAGMA_WARNING_POP


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
class CommandLineArguments;


//[-------------------------------------------------------]
//[ Classes                                               ]
//[-------------------------------------------------------]
class ExampleRunner final
{


//[-------------------------------------------------------]
//[ Public definitions                                    ]
//[-------------------------------------------------------]
public:
	typedef int(*RunnerMethod)(ExampleRunner&, const char*);
	typedef std::map<std::string_view, RunnerMethod>	   AvailableExamples;
	typedef std::set<std::string_view>					   AvailableRenderers;
	typedef std::vector<std::string_view>				   SupportedRenderers;
	typedef std::map<std::string_view, SupportedRenderers> ExampleToSupportedRenderers;


//[-------------------------------------------------------]
//[ Public methods                                        ]
//[-------------------------------------------------------]
public:
	ExampleRunner();

	inline ~ExampleRunner()
	{}

	[[nodiscard]] inline const AvailableRenderers& getAvailableRenderers() const
	{
		return mAvailableRenderers;
	}

	[[nodiscard]] inline const ExampleToSupportedRenderers& getExampleToSupportedRenderers() const
	{
		return mExampleToSupportedRenderers;
	}

	[[nodiscard]] inline const std::string& getDefaultRendererName() const
	{
		return mDefaultRendererName;
	}

	[[nodiscard]] inline const std::string& getDefaultExampleName() const
	{
		return mDefaultExampleName;
	}

	[[nodiscard]] inline const std::string& getCurrentRendererName() const
	{
		return mCurrentRendererName;
	}

	[[nodiscard]] inline const std::string& getCurrentExampleName() const
	{
		return mCurrentExampleName;
	}

	[[nodiscard]] int run(const CommandLineArguments& commandLineArguments);

	/**
	*  @brief
	*    Ask the example runner politely to switch to another example as soon as possible
	*
	*  @param[in] exampleName
	*    Example name, must be valid
	*  @param[in] rendererName
	*    Renderer name, if null pointer the default renderer will be used
	*/
	void switchExample(const char* exampleName, const char* rendererName = nullptr);


//[-------------------------------------------------------]
//[ Private static methods                                ]
//[-------------------------------------------------------]
private:
	template <class ExampleClass>
	[[nodiscard]] static int runRenderExample(ExampleRunner& exampleRunner, const char* rendererName)
	{
		ExampleClass exampleClass;
		exampleClass.mExampleRunner = &exampleRunner;
		return IApplicationRenderer(rendererName, exampleClass).run();
	}

	template <class ExampleClass>
	[[nodiscard]] static int runRenderRuntimeExample(ExampleRunner& exampleRunner, const char* rendererName)
	{
		ExampleClass exampleClass;
		exampleClass.mExampleRunner = &exampleRunner;
		return IApplicationRendererRuntime(rendererName, exampleClass).run();
	}

	template <class ExampleClass>
	[[nodiscard]] static int runBasicExample(ExampleRunner& exampleRunner, const char* rendererName)
	{
		ExampleClass exampleClass(exampleRunner, rendererName);
		return exampleClass.run();
	}


//[-------------------------------------------------------]
//[ Private methods                                       ]
//[-------------------------------------------------------]
private:
	[[nodiscard]] bool parseCommandLineArguments(const CommandLineArguments& commandLineArguments);
	void printUsage(const AvailableExamples& availableExamples, const AvailableRenderers& availableRenderers);
	void showError(const std::string& errorMessage);
	[[nodiscard]] int runExample(const std::string_view& rendererName, const std::string_view& exampleName);

	template<typename T>
	void addExample(const std::string_view& name, RunnerMethod runnerMethod, const T& supportedRendererList)
	{
		mAvailableExamples.insert(std::pair<std::string_view, RunnerMethod>(name, runnerMethod));
		SupportedRenderers supportedRenderers;
		for (const std::string_view& supportedRenderer : supportedRendererList)
		{
			supportedRenderers.push_back(supportedRenderer);
		}
		mExampleToSupportedRenderers.insert(std::pair<std::string_view, SupportedRenderers>(name, std::move(supportedRenderers)));
	}


//[-------------------------------------------------------]
//[ Private data                                          ]
//[-------------------------------------------------------]
private:
	AvailableExamples 			mAvailableExamples;
	AvailableRenderers			mAvailableRenderers;
	ExampleToSupportedRenderers	mExampleToSupportedRenderers;
	std::string					mDefaultRendererName;
	std::string					mDefaultExampleName;
	std::string					mCurrentRendererName;
	std::string					mCurrentExampleName;
	std::string					mNextRendererName;
	std::string					mNextExampleName;


};

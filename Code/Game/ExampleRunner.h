#pragma once

#include "Framework/PlatformTypes.h"
#include "Framework/IApplicationRenderer.h"
#include "Framework/IApplicationRendererRuntime.h"

class CommandLineArguments;

class ExampleRunner final
{
public:
	typedef int(*RunnerMethod)(ExampleRunner&, const char*);
	typedef std::map<std::string_view, RunnerMethod>	   AvailableExamples;
	typedef std::set<std::string_view>					   AvailableRenderers;
	typedef std::vector<std::string_view>				   SupportedRenderers;
	typedef std::map<std::string_view, SupportedRenderers> ExampleToSupportedRenderers;
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

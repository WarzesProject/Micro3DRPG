﻿#include "stdafx.h"
#include "ExampleRunner.h"
#include "CommandLineArguments.h"
#include "Game.h"
#include "BaseExample.h"
#include "AppRenderer.h"

#define SAMPLE 0

//-----------------------------------------------------------------------------
#if SE_COMPILER_MSVC
// 3rdparty Lib
#	pragma comment(lib, "SDL2.lib")
#	pragma comment(lib, "SDL2main.lib")
// Engine Lib
#	pragma comment(lib, "Core.lib")
#	pragma comment(lib, "Engine.lib")
#	pragma comment(lib, "3rdparty.lib")
#endif
//-----------------------------------------------------------------------------
// Global variables
extern "C"
{
	// NVIDIA: Force usage of NVidia GPU in case there is an integrated graphics unit as well, if we don't do this we risk getting the integrated graphics unit and hence a horrible performance
	// -> See "Enabling High Performance Graphics Rendering on Optimus Systems" http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;

	// AMD: Force usage of AMD GPU in case there is an integrated graphics unit as well, if we don't do this we risk getting the integrated graphics unit and hence a horrible performance
	// -> Named "Dynamic Switchable Graphics", found no official documentation, only https://community.amd.com/message/1307599#comment-1307599 - "Can an OpenGL app default to the discrete GPU on an Enduro system?"
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
//-----------------------------------------------------------------------------
int programEntryPoint(const CommandLineArguments &commandLineArguments)
{
#if SAMPLE
	return ExampleRunner().run(commandLineArguments);
#else
	BaseExample exampleClass;
	return AppRenderer("Direct3D11", exampleClass).run();
#endif
}
//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	// For memory leak detection
#if SE_DEBUG
	// "_CrtDumpMemoryLeaks()" reports false positive memory leak with static variables, so use a memory difference instead
	_CrtMemState crtMemState = { 0 };
	_CrtMemCheckpoint(&crtMemState);
#endif

	// Call the platform independent program entry point
	// -> Uses internally "GetCommandLine()" to fetch the command line arguments
	const int result = programEntryPoint(CommandLineArguments());

	// For memory leak detection
#if SE_DEBUG
	_CrtMemDumpAllObjectsSince(&crtMemState);
#endif

	return result;
}
//-----------------------------------------------------------------------------
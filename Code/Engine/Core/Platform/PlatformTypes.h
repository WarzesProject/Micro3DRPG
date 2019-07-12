#pragma once

//[-------------------------------------------------------]
//[ Project independent generic export                    ]
//[-------------------------------------------------------]
#ifdef _WIN32
	// To export classes, methods and variables
	#define GENERIC_API_EXPORT	__declspec(dllexport)
#elif LINUX
	// To export classes, methods and variables
	#define GENERIC_API_EXPORT __attribute__ ((visibility("default")))
#else
	#error "Unsupported platform"
#endif


//[-------------------------------------------------------]
//[ Project independent generic import                    ]
//[-------------------------------------------------------]
#ifdef _WIN32
	// To import classes, methods and variables
	#define GENERIC_API_IMPORT __declspec(dllimport)
#elif LINUX
	// To import classes, methods and variables
	#define GENERIC_API_IMPORT
#else
	#error "Unsupported platform"
#endif

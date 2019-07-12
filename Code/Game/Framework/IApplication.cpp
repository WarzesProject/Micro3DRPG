#include "stdafx.h"
#include "Framework/IApplication.h"
#ifdef SDL2_FOUND
	#include "Framework/ApplicationImplSdl2.h"
#elif defined _WIN32
	#include "Framework/ApplicationImplWindows.h"
#elif defined __ANDROID__
	#warning TODO(co) The Android support is work-in-progress
#elif defined LINUX
	#include "Framework/Linux/ApplicationImplLinux.h"
#endif


//[-------------------------------------------------------]
//[ Public methods                                        ]
//[-------------------------------------------------------]
IApplication::~IApplication()
{
	// Destroy the application instance
	delete mApplicationImpl;
}

int IApplication::run()
{
	// Call application implementation initialization method
	mApplicationImpl->onInitialization();
	if (onInitialization())
	{
		// Main loop - Process OS messages (non-blocking) first
		while (!mExit && !mApplicationImpl->processMessages())
		{
			// Update the application logic
			onUpdate();

			// Redraw request
			redraw();
		}

		// Call application implementation de-initialization method
		onDeinitialization();
		mApplicationImpl->onDeinitialization();
	}

	// Done, no error
	return 0;
}


//[-------------------------------------------------------]
//[ Public virtual IApplication methods                   ]
//[-------------------------------------------------------]
void IApplication::onDeinitialization()
{
	// Added the call to the implementation here so that a derived class can do the base deinit at a stage which fits better
	mApplicationImpl->onDeinitialization();
}


//[-------------------------------------------------------]
//[ Protected methods                                     ]
//[-------------------------------------------------------]
IApplication::IApplication(const char* windowTitle) :
	mApplicationImpl(nullptr),
	mExit(false)
{
	// We're using "this" in here, so we are not allowed to write the following within the initializer list
	#ifdef SDL2_FOUND
		mApplicationImpl = new ApplicationImplSdl2(*this, windowTitle);
	#elif defined _WIN32
		mApplicationImpl = new ApplicationImplWindows(*this, windowTitle);
	#elif defined __ANDROID__
		#warning TODO(co) The Android support is work-in-progress
	#elif defined LINUX
		mApplicationImpl = new ApplicationImplLinux(*this, windowTitle);
	#else
		#error "Unsupported platform"
	#endif
}

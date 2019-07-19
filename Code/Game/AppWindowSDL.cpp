#include "stdafx.h"
#include "AppWindowSDL.h"
#include "AppRenderer.h"

AppWindowSDL::AppWindowSDL(AppRenderer &application, const char* windowTitle) :
	mApplication(application),
	m_sdlWindow(nullptr)
#ifdef RENDERER_RUNTIME_IMGUI
	, m_imGuiMousePressed{ false, false, false }
#endif
{
	strncpy(mWindowTitle, windowTitle, 64);
}

void AppWindowSDL::onInitialization()
{
	if (SDL_Init(0) == 0)
		m_sdlWindow = SDL_CreateWindow(mWindowTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 768, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);

	if (nullptr != m_sdlWindow)
		SDL_ShowWindow(m_sdlWindow);

#ifdef RENDERER_RUNTIME_IMGUI
	initializeImGuiKeyMap();
#endif
}

void AppWindowSDL::onDeinitialization()
{
	if (nullptr != m_sdlWindow)
	{
		SDL_DestroyWindow(m_sdlWindow);
		m_sdlWindow = nullptr;
	}
	SDL_Quit();
}

bool AppWindowSDL::processMessages()
{
	bool quit = false;

	SDL_Event sdlEvent;
	while (SDL_PollEvent(&sdlEvent) != 0)
	{
		switch (sdlEvent.type)
		{
		case SDL_QUIT:
			quit = true;
			break;

		case SDL_WINDOWEVENT:
			switch (sdlEvent.window.event)
			{
			case SDL_WINDOWEVENT_EXPOSED:
				mApplication.onDrawRequest();
				break;

			case SDL_WINDOWEVENT_SIZE_CHANGED:
				mApplication.onResize();
				break;
			}
			break;

		case SDL_KEYDOWN:
			if (!sdlEvent.key.repeat && SDLK_RETURN == sdlEvent.key.keysym.sym && (SDL_GetModState() & KMOD_ALT))
			{
				mApplication.onToggleFullscreenState();
			}
			else if (!sdlEvent.key.repeat && SDLK_ESCAPE == sdlEvent.key.keysym.sym)
			{
				mApplication.onEscapeKey();
			}
			break;
		}
#ifdef RENDERER_RUNTIME_IMGUI
		if (ImGui::GetCurrentContext() != nullptr)
		{
			processImGuiSdl2Event(sdlEvent);
		}
#endif
	}
#ifdef RENDERER_RUNTIME_IMGUI
	if (ImGui::GetCurrentContext() != nullptr)
	{
		updateImGuiMousePositionAndButtons();
	}
#endif

	return quit;
}

void AppWindowSDL::getWindowSize(int& width, int& height) const
{
	if (nullptr != m_sdlWindow)
		SDL_GL_GetDrawableSize(m_sdlWindow, &width, &height);
	else
	{
		width = 0;
		height = 0;
	}
}

handle AppWindowSDL::getNativeWindowHandle() const
{
	handle nativeWindowHandle = NULL_HANDLE;
	if (nullptr != m_sdlWindow)
	{
		SDL_SysWMinfo sdlSysWMinfo;
		SDL_VERSION(&sdlSysWMinfo.version);
		if (SDL_GetWindowWMInfo(m_sdlWindow, &sdlSysWMinfo))
		{
			switch (sdlSysWMinfo.subsystem)
			{
#if defined _WIN32
			case SDL_SYSWM_UNKNOWN:
				assert(false);
				break;

			case SDL_SYSWM_WINDOWS:
				nativeWindowHandle = reinterpret_cast<handle>(sdlSysWMinfo.info.win.window);
				break;

			case SDL_SYSWM_X11:
			case SDL_SYSWM_DIRECTFB:
			case SDL_SYSWM_COCOA:
			case SDL_SYSWM_UIKIT:
			case SDL_SYSWM_WAYLAND:
			case SDL_SYSWM_MIR:
			case SDL_SYSWM_WINRT:
			case SDL_SYSWM_ANDROID:
			case SDL_SYSWM_VIVANTE:
			case SDL_SYSWM_OS2:
				assert(false);
				break;
#elif defined __ANDROID__
				#warning TODO(co) The Android support is work - in - progress
#elif defined LINUX
			case SDL_SYSWM_UNKNOWN:
			case SDL_SYSWM_WINDOWS:
				assert(false);
				break;

			case SDL_SYSWM_X11:
				nativeWindowHandle = reinterpret_cast<handle>(sdlSysWMinfo.info.x11.window);
				break;

			case SDL_SYSWM_DIRECTFB:
			case SDL_SYSWM_COCOA:
			case SDL_SYSWM_UIKIT:
				assert(false);
				break;

			case SDL_SYSWM_WAYLAND:
				nativeWindowHandle = reinterpret_cast<handle>(sdlSysWMinfo.info.wl.surface);
				break;

			case SDL_SYSWM_MIR:
			case SDL_SYSWM_WINRT:
			case SDL_SYSWM_ANDROID:
			case SDL_SYSWM_VIVANTE:
			case SDL_SYSWM_OS2:
				assert(false);
				break;
#else
#error "Unsupported platform"
#endif
			}
		}
	}

	return nativeWindowHandle;
}

void AppWindowSDL::redraw()
{
	mApplication.onDrawRequest();
}

// Basing on https://github.com/ocornut/imgui/blob/master/examples/imgui_impl_sdl.cpp
#ifdef RENDERER_RUNTIME_IMGUI
void AppWindowSDL::initializeImGuiKeyMap()
{
	// Keyboard mapping: ImGui will use those indices to peek into the "ImGuiIO::KeyDown[]" array that we will update during the application lifetime
	ImGuiIO& imGuiIo = ImGui::GetIO();
	imGuiIo.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB;
	imGuiIo.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
	imGuiIo.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
	imGuiIo.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
	imGuiIo.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
	imGuiIo.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
	imGuiIo.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
	imGuiIo.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
	imGuiIo.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
	imGuiIo.KeyMap[ImGuiKey_Insert] = SDL_SCANCODE_INSERT;
	imGuiIo.KeyMap[ImGuiKey_Delete] = SDL_SCANCODE_DELETE;
	imGuiIo.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;
	imGuiIo.KeyMap[ImGuiKey_Space] = SDL_SCANCODE_SPACE;
	imGuiIo.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
	imGuiIo.KeyMap[ImGuiKey_Escape] = SDL_SCANCODE_ESCAPE;
	imGuiIo.KeyMap[ImGuiKey_A] = SDL_SCANCODE_A;
	imGuiIo.KeyMap[ImGuiKey_C] = SDL_SCANCODE_C;
	imGuiIo.KeyMap[ImGuiKey_V] = SDL_SCANCODE_V;
	imGuiIo.KeyMap[ImGuiKey_X] = SDL_SCANCODE_X;
	imGuiIo.KeyMap[ImGuiKey_Y] = SDL_SCANCODE_Y;
	imGuiIo.KeyMap[ImGuiKey_Z] = SDL_SCANCODE_Z;
}

void AppWindowSDL::processImGuiSdl2Event(const SDL_Event& sdlEvent)
{
	assert(ImGui::GetCurrentContext() != nullptr);
	ImGuiIO& imGuiIo = ImGui::GetIO();
	switch (sdlEvent.type)
	{
	case SDL_MOUSEWHEEL:
	{
		if (sdlEvent.wheel.x > 0)
		{
			imGuiIo.MouseWheelH += 1.0f;
		}
		if (sdlEvent.wheel.x < 0)
		{
			imGuiIo.MouseWheelH -= 1.0f;
		}
		if (sdlEvent.wheel.y > 0)
		{
			imGuiIo.MouseWheel += 1.0f;
		}
		if (sdlEvent.wheel.y < 0)
		{
			imGuiIo.MouseWheel -= 1.0f;
		}
		break;
	}

	case SDL_MOUSEBUTTONDOWN:
	{
		switch (sdlEvent.button.button)
		{
		case SDL_BUTTON_LEFT:
			m_imGuiMousePressed[0] = true;
			break;

		case SDL_BUTTON_RIGHT:
			m_imGuiMousePressed[1] = true;
			break;

		case SDL_BUTTON_MIDDLE:
			m_imGuiMousePressed[2] = true;
			break;
		}
		break;
	}

	case SDL_TEXTINPUT:
	{
		imGuiIo.AddInputCharactersUTF8(sdlEvent.text.text);
		break;
	}

	case SDL_KEYDOWN:
	case SDL_KEYUP:
	{
		const int key = sdlEvent.key.keysym.scancode;
		assert(key >= 0 && key < IM_ARRAYSIZE(imGuiIo.KeysDown));
		imGuiIo.KeysDown[key] = (SDL_KEYDOWN == sdlEvent.type);
		imGuiIo.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
		imGuiIo.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
		imGuiIo.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
		imGuiIo.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
		break;
	}
	}
}

void AppWindowSDL::updateImGuiMousePositionAndButtons()
{
	assert(ImGui::GetCurrentContext() != nullptr);
	ImGuiIO& imGuiIo = ImGui::GetIO();
	int mousePositionX, mousePositionY;
	const Uint32 sdlMouseButtons = SDL_GetMouseState(&mousePositionX, &mousePositionY);
	imGuiIo.MouseDown[0] = (m_imGuiMousePressed[0] || (sdlMouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0);	// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame
	imGuiIo.MouseDown[1] = (m_imGuiMousePressed[1] || (sdlMouseButtons & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0);
	imGuiIo.MouseDown[2] = (m_imGuiMousePressed[2] || (sdlMouseButtons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0);
	m_imGuiMousePressed[0] = m_imGuiMousePressed[1] = m_imGuiMousePressed[2] = false;
	imGuiIo.MousePos = (SDL_GetWindowFlags(m_sdlWindow) & SDL_WINDOW_INPUT_FOCUS) ? ImVec2(static_cast<float>(mousePositionX), static_cast<float>(mousePositionY)) : ImVec2(-FLT_MAX, -FLT_MAX);
}
#endif
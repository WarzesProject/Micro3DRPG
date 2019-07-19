#pragma once

#include "WindowSDL.h"
#include <Engine/Renderer.h>
#include "GameApp.h"

struct GameConfig
{

};

class Game
{
	friend class GameApp;
public:
	Game(const GameConfig &config);

	int Run();


	std::function<bool()> gameInit;
	std::function<void()> gameClose;
	std::function<void()> gameUpdate;
	std::function<bool()> gameDoesCompleteOwnDrawing;
	std::function<void()> gameDraw;

	static Game& Get();


private:
	bool init();
	void onResize();
	void onToggleFullscreenState();
	void onDrawRequest();

	WindowSDL m_windows;
	Renderer::Context *mRendererContext = nullptr;
	Renderer::RendererInstance *mRendererInstance = nullptr;
	Renderer::IRenderer *mRenderer = nullptr;
	Renderer::ISwapChain *mMainSwapChain = nullptr;
	Renderer::CommandBuffer mCommandBuffer;

	GameApp m_app;
};
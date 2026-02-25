#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <print>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_shadercross/SDL_shadercross.h>

#include "application/application.hpp"
#include "shader/shader.hpp"
#include "utility/unique_pointer.hpp"

SDL_AppResult SDL_AppInit(void** const appstate, int const, char** const)
{
	SDL_InitSubSystem(SDL_INIT_VIDEO);
	*appstate = new fro::Application();
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* const appstate)
{
	static_cast<fro::Application*>(appstate)->tick();
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* const appstate, SDL_Event* const event)
{
	return
		static_cast<fro::Application*>(appstate)->process_event(*event)
			? SDL_APP_CONTINUE
			: SDL_APP_SUCCESS;
}

void SDL_AppQuit(void* const appstate, SDL_AppResult const)
{
	delete static_cast<fro::Application*>(appstate);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}
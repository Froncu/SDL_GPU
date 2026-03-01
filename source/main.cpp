#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include "application/application.hpp"
#include "pch.hpp"
#include "shader/shader.hpp"
#include "utility/unique_pointer.hpp"

SDL_AppResult SDL_AppInit(void** const app_state, int const, char** const) try
{
	SDL_InitSubSystem(SDL_INIT_VIDEO);
	*app_state = new fro::Application();
	return SDL_APP_CONTINUE;
}
catch (std::exception& exception)
{
	SDL_Log(exception.what());
	return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppIterate(void* const app_state) try
{
	static_cast<fro::Application*>(app_state)->tick();
	return SDL_APP_CONTINUE;
}
catch (std::exception& exception)
{
	SDL_Log(exception.what());
	return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppEvent(void* const app_state, SDL_Event* const event) try
{
	return
		static_cast<fro::Application*>(app_state)->process_event(*event)
			? SDL_APP_CONTINUE
			: SDL_APP_SUCCESS;
}
catch (std::exception& exception)
{
	SDL_Log(exception.what());
	return SDL_APP_FAILURE;
}

void SDL_AppQuit(void* const app_state, SDL_AppResult const) try
{
	delete static_cast<fro::Application*>(app_state);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}
catch (std::exception& exception)
{
	SDL_Log(exception.what());
}
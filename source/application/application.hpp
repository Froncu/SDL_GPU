#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <array>

#include <SDL3/SDL.h>

#include "shader/shader.hpp"
#include "utility/unique_pointer.hpp"

namespace fro
{
	using Index = std::uint32_t;
	using Vertex = std::pair<float, float>;

	class Application final
	{
		public:
			Application();
			Application(Application const&) = delete;
			Application(Application&&) = delete;

			~Application();

			Application& operator=(Application const&) = delete;
			Application& operator=(Application&&) = delete;

			void tick();
			[[nodiscard]] bool process_event(SDL_Event& event);

		private:
			[[noreturn]] static void error();

			UniquePointer<SDL_Window> const window_{
				SDL_CreateWindow("SDL_GPU", 1280, 720, SDL_WINDOW_RESIZABLE),
				SDL_DestroyWindow
			};

			fro::UniquePointer<SDL_GPUDevice> const gpu_device_{
				SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr),
				SDL_DestroyGPUDevice
			};

			Shader const vertex_shader_{ *gpu_device_, "resources/shaders/vertex.vert" };
			Shader const fragment_shader_{ *gpu_device_, "resources/shaders/fragment.frag" };

			SDL_GPUColorTargetDescription const color_target_description_{
				.format{
					[](SDL_GPUDevice& gpu_device, SDL_Window& window)
					{
						if (not SDL_ClaimWindowForGPUDevice(&gpu_device, &window))
							error();

						if (not SDL_SetGPUSwapchainParameters(&gpu_device, &window,
							SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_IMMEDIATE))
							error();

						return SDL_GetGPUSwapchainTextureFormat(&gpu_device, &window);
					}(*gpu_device_, *window_)
				},
			};

			SDL_GPUVertexAttribute const vertex_attributes{
				.format{ SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2 },
			};

			SDL_GPUVertexBufferDescription const vertex_buffer_desciption{
				.pitch{ sizeof(Vertex) },
				.input_rate{ SDL_GPU_VERTEXINPUTRATE_VERTEX }
			};

			SDL_GPUGraphicsPipelineCreateInfo const pipeline_create_info_{
				.vertex_shader{ &vertex_shader_.native_shader() },
				.fragment_shader{ &fragment_shader_.native_shader() },
				.vertex_input_state{
					.vertex_buffer_descriptions{ &vertex_buffer_desciption },
					.num_vertex_buffers{ 1 },
					.vertex_attributes{ &vertex_attributes },
					.num_vertex_attributes{ 1 }
				},
				.primitive_type{ SDL_GPU_PRIMITIVETYPE_TRIANGLELIST },
				.rasterizer_state{
					.fill_mode{ SDL_GPU_FILLMODE_FILL },
					.cull_mode{ SDL_GPU_CULLMODE_BACK },
					.front_face{ SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE }
				},
				.target_info{
					.color_target_descriptions{ &color_target_description_ },
					.num_color_targets{ 1 }
				}
			};

			fro::UniquePointer<SDL_GPUGraphicsPipeline> const pipeline_{
				SDL_CreateGPUGraphicsPipeline(gpu_device_.get(), &pipeline_create_info_),
				std::bind(SDL_ReleaseGPUGraphicsPipeline, gpu_device_.get(), std::placeholders::_1)
			};

			std::array<Index, 6> indices_{ 0, 1, 2, 2, 3, 0 };
			std::array<Vertex, 4> vertices_{ { { -0.5f, -0.5f }, { 0.5f, -0.5f }, { 0.5f, 0.5f }, { -0.5f, 0.5f } } };
	};
}

#endif
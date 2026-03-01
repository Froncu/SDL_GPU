#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <array>

#include <glm/glm.hpp>
#include <SDL3/SDL.h>

#include "shader/shader.hpp"
#include "utility/unique_pointer.hpp"

namespace fro
{
	using Index = std::uint32_t;

	struct Vertex final
	{
		float x, y;
		float r, g, b;
	};

	struct Transforms
	{
		glm::mat4 view_projection;
		glm::mat4 model;
	};

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

			std::array<SDL_GPUVertexAttribute const, 2> const attributes_{
				{
					{
						.location{ 0 },
						.buffer_slot{ 0 },
						.format{ SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2 },
						.offset{ 0 }
					},
					{
						.location{ 1 },
						.buffer_slot{ 0 },
						.format{ SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3 },
						.offset{ sizeof(float) * 2 }
					}
				}
			};

			std::array<SDL_GPUVertexBufferDescription const, 1> const vertex_buffer_desciptions_{
				{
					{
						.pitch{ sizeof(Vertex) },
						.input_rate{ SDL_GPU_VERTEXINPUTRATE_VERTEX }
					}
				}
			};

			SDL_GPUGraphicsPipelineCreateInfo const pipeline_create_info_{
				.vertex_shader{ &vertex_shader_.native_shader() },
				.fragment_shader{ &fragment_shader_.native_shader() },
				.vertex_input_state{
					.vertex_buffer_descriptions{ vertex_buffer_desciptions_.data() },
					.num_vertex_buffers{ static_cast<Uint32>(vertex_buffer_desciptions_.size()) },
					.vertex_attributes{ attributes_.data() },
					.num_vertex_attributes{ static_cast<Uint32>(attributes_.size()) }
				},
				.primitive_type{ SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP },
				.rasterizer_state{
					.fill_mode{ SDL_GPU_FILLMODE_FILL },
					.cull_mode{ SDL_GPU_CULLMODE_BACK },
					.front_face{ SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE }
				},
				.depth_stencil_state{
					.compare_op{ SDL_GPU_COMPAREOP_LESS },
					.enable_depth_test{ true },
					.enable_depth_write{ true },
				},
				.target_info{
					.color_target_descriptions{ &color_target_description_ },
					.num_color_targets{ 1 },
					.depth_stencil_format{ SDL_GPU_TEXTUREFORMAT_D32_FLOAT },
					.has_depth_stencil_target{ true },
				}
			};

			fro::UniquePointer<SDL_GPUGraphicsPipeline> const pipeline_{
				SDL_CreateGPUGraphicsPipeline(gpu_device_.get(), &pipeline_create_info_),
				std::bind(SDL_ReleaseGPUGraphicsPipeline, gpu_device_.get(), std::placeholders::_1)
			};

			std::array<Index, 4> indices_{ 0, 1, 3, 2 };
			UniquePointer<SDL_GPUBuffer> index_buffer_{};
			SDL_GPUBufferBinding index_buffer_binding_{};

			std::array<Vertex, 4> vertices_{
				{
					{ -0.5f, -0.5f, 1.0f, 0.0f, 0.0f }, // red
					{ 0.5f, -0.5f, 0.0f, 1.0f, 0.0f }, // green
					{ 0.5f, 0.5f, 0.0f, 0.0f, 1.0f }, // blue
					{ -0.5f, 0.5f, 1.0f, 1.0f, 0.0f }, // yellow
				}
			};
			UniquePointer<SDL_GPUBuffer> vertex_buffer_{};
			SDL_GPUBufferBinding vertex_buffer_binding_{};

			glm::mat4 camera_{ 1.0f };

			SDL_GPUTextureCreateInfo const depth_texture_create_info{
				.type{ SDL_GPU_TEXTURETYPE_2D },
				.format{ SDL_GPU_TEXTUREFORMAT_D32_FLOAT },
				.usage{ SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET },
				.width{ 1280 },
				.height{ 720 },
				.layer_count_or_depth{ 1 },
				.num_levels{ 1 },
			};

			UniquePointer<SDL_GPUTexture> depth_texture_{
				SDL_CreateGPUTexture(gpu_device_.get(), &depth_texture_create_info),
				std::bind(SDL_ReleaseGPUTexture, gpu_device_.get(), std::placeholders::_1)
			};
	};
}

#endif
#include <print>
#include <span>

#include "application.hpp"

namespace fro
{
	Application::Application()
	{
	}

	Application::~Application()
	{
	}

	void Application::tick()
	{
		if (not SDL_WaitForGPUSwapchain(gpu_device_.get(), window_.get()))
			error();

		SDL_GPUCommandBuffer* const command_buffer{ SDL_AcquireGPUCommandBuffer(gpu_device_.get()) };
		if (not command_buffer)
			error();

		SDL_GPUTexture* swap_chain_texture;
		if (not SDL_AcquireGPUSwapchainTexture(command_buffer, window_.get(), &swap_chain_texture, nullptr, nullptr))
			error();

		if (not swap_chain_texture)
		{
			if (not SDL_CancelGPUCommandBuffer(command_buffer))
				error();

			return;
		}

		SDL_GPUColorTargetInfo const color_target_info{
			.texture{ swap_chain_texture },
			.clear_color{ 0.1f, 0.1f, 0.1f, 1.0f },
			.load_op{ SDL_GPU_LOADOP_CLEAR },
			.store_op{ SDL_GPU_STOREOP_STORE },
		};

		SDL_GPUBufferCreateInfo const index_buffer_create_info{
			.usage{ SDL_GPU_BUFFERUSAGE_INDEX },
			.size{ static_cast<Uint32>(sizeof(Index) * indices_.size()) },
		};

		UniquePointer<SDL_GPUBuffer> const index_buffer{
			SDL_CreateGPUBuffer(gpu_device_.get(), &index_buffer_create_info),
			std::bind(SDL_ReleaseGPUBuffer, gpu_device_.get(), std::placeholders::_1)
		};

		SDL_GPUBufferBinding const index_buffer_binding{
			.buffer{ index_buffer.get() }
		};

		SDL_GPUTransferBufferCreateInfo const index_transfer_buffer_create_info{
			.usage{ SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD },
			.size{ static_cast<Uint32>(sizeof(Index) * indices_.size()) },
		};

		UniquePointer<SDL_GPUTransferBuffer> const index_transfer_buffer{
			SDL_CreateGPUTransferBuffer(gpu_device_.get(), &index_transfer_buffer_create_info),
			std::bind(SDL_ReleaseGPUTransferBuffer, gpu_device_.get(), std::placeholders::_1)
		};

		std::ranges::copy(indices_,
			static_cast<Index*>(SDL_MapGPUTransferBuffer(gpu_device_.get(), index_transfer_buffer.get(), true)));

		SDL_UnmapGPUTransferBuffer(gpu_device_.get(), index_transfer_buffer.get());

		SDL_GPUTransferBufferLocation const index_buffer_location{
			.transfer_buffer{ index_transfer_buffer.get() }
		};

		SDL_GPUBufferRegion const index_buffer_region{
			.buffer{ index_buffer.get() },
			.size{ index_transfer_buffer_create_info.size }
		};

		SDL_GPUBufferCreateInfo const vertex_buffer_create_info{
			.usage{ SDL_GPU_BUFFERUSAGE_VERTEX },
			.size{ static_cast<Uint32>(sizeof(Vertex) * vertices_.size()) },
		};

		UniquePointer<SDL_GPUBuffer> const vertex_buffer{
			SDL_CreateGPUBuffer(gpu_device_.get(), &vertex_buffer_create_info),
			std::bind(SDL_ReleaseGPUBuffer, gpu_device_.get(), std::placeholders::_1)
		};

		SDL_GPUBufferBinding const vertex_buffer_binding{
			.buffer{ vertex_buffer.get() }
		};

		SDL_GPUTransferBufferCreateInfo const vertex_transfer_buffer_create_info{
			.usage{ SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD },
			.size{ vertex_buffer_create_info.size },
		};

		UniquePointer<SDL_GPUTransferBuffer> const vertex_transfer_buffer{
			SDL_CreateGPUTransferBuffer(gpu_device_.get(), &vertex_transfer_buffer_create_info),
			std::bind(SDL_ReleaseGPUTransferBuffer, gpu_device_.get(), std::placeholders::_1)
		};

		std::ranges::copy(vertices_,
			static_cast<Vertex*>(SDL_MapGPUTransferBuffer(gpu_device_.get(), vertex_transfer_buffer.get(), true)));

		SDL_UnmapGPUTransferBuffer(gpu_device_.get(), vertex_transfer_buffer.get());

		SDL_GPUTransferBufferLocation const vertex_buffer_location{
			.transfer_buffer{ vertex_transfer_buffer.get() }
		};

		SDL_GPUBufferRegion const vertex_buffer_region{
			.buffer{ vertex_buffer.get() },
			.size{ vertex_buffer_create_info.size }
		};

		SDL_GPUCopyPass* const copy_pass{ SDL_BeginGPUCopyPass(command_buffer) };
		SDL_UploadToGPUBuffer(copy_pass, &index_buffer_location, &index_buffer_region, false);
		SDL_UploadToGPUBuffer(copy_pass, &vertex_buffer_location, &vertex_buffer_region, false);
		SDL_EndGPUCopyPass(copy_pass);

		SDL_GPURenderPass& render_pass{ *SDL_BeginGPURenderPass(command_buffer, &color_target_info, 1, nullptr) };
		SDL_BindGPUGraphicsPipeline(&render_pass, pipeline_.get());
		SDL_BindGPUIndexBuffer(&render_pass, &index_buffer_binding, SDL_GPU_INDEXELEMENTSIZE_32BIT);
		SDL_BindGPUVertexBuffers(&render_pass, 0, &vertex_buffer_binding, 1);
		SDL_DrawGPUIndexedPrimitives(&render_pass, static_cast<Uint32>(indices_.size()), 1, 0, 0, 0);
		SDL_EndGPURenderPass(&render_pass);

		if (not SDL_SubmitGPUCommandBuffer(command_buffer))
			error();
	}

	bool Application::process_event(SDL_Event& event)
	{
		switch (event.type)
		{
			case SDL_EVENT_QUIT:
				return false;

			default:
				return true;
		}
	}

	void Application::error()
	{
		std::println("{}", SDL_GetError());
		std::abort();
	}
}
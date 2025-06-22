#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <print>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_shadercross/SDL_shadercross.h>

#include "shader/shader.hpp"
#include "utility/unique_pointer.hpp"

[[noreturn]] static void error()
{
   std::print("{}", SDL_GetError());
   std::abort();
}

int main(int, char*[]) try
{
   SDL_InitSubSystem(SDL_INIT_VIDEO);

   {
      fro::UniquePointer<SDL_Window> const window{
         SDL_CreateWindow("SDL_GPU", 1280, 720, SDL_WINDOW_RESIZABLE),
         SDL_DestroyWindow
      };
      if (not window)
         error();

      fro::UniquePointer<SDL_GPUDevice> const gpu_device{
         SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr),
         SDL_DestroyGPUDevice
      };
      if (not gpu_device)
         error();

      if (not SDL_ClaimWindowForGPUDevice(gpu_device.get(), window.get()))
         error();

      if (not SDL_SetGPUSwapchainParameters(gpu_device.get(), window.get(),
         SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_IMMEDIATE))
         error();

      fro::Shader const vertex_shader{ *gpu_device, "resources/shaders/vertex.vert" };
      fro::Shader const fragment_shader{ *gpu_device, "resources/shaders/fragment.frag" };

      SDL_GPUColorTargetDescription const color_target_description{
         .format{ SDL_GetGPUSwapchainTextureFormat(gpu_device.get(), window.get()) },
      };

      SDL_GPUGraphicsPipelineCreateInfo const pipeline_create_info{
         .vertex_shader{ &vertex_shader.native_shader() },
         .fragment_shader{ &fragment_shader.native_shader() },
         .primitive_type{ SDL_GPU_PRIMITIVETYPE_TRIANGLELIST },
         .rasterizer_state{
            .fill_mode{ SDL_GPU_FILLMODE_FILL },
            .cull_mode{ SDL_GPU_CULLMODE_BACK },
            .front_face{ SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE }
         },
         .target_info{
            .color_target_descriptions{ &color_target_description },
            .num_color_targets{ 1 }
         }
      };

      fro::UniquePointer<SDL_GPUGraphicsPipeline> const pipeline{
         SDL_CreateGPUGraphicsPipeline(gpu_device.get(), &pipeline_create_info),
         std::bind(SDL_ReleaseGPUGraphicsPipeline, gpu_device.get(), std::placeholders::_1)
      };
      if (not pipeline)
         error();

      bool is_running{ true };
      while (is_running)
      {
         SDL_Event event;
         while (SDL_PollEvent(&event))
            switch (event.type)
            {
               case SDL_EVENT_QUIT:
                  is_running = false;
                  break;

               default:
                  break;
            }

         if (not SDL_WaitForGPUSwapchain(gpu_device.get(), window.get()))
            error();

         SDL_GPUCommandBuffer* const command_buffer{ SDL_AcquireGPUCommandBuffer(gpu_device.get()) };
         if (not command_buffer)
            error();

         SDL_GPUTexture* swap_chain_texture;
         if (not SDL_AcquireGPUSwapchainTexture(command_buffer, window.get(), &swap_chain_texture, nullptr, nullptr))
            error();

         if (not swap_chain_texture)
         {
            if (not SDL_CancelGPUCommandBuffer(command_buffer))
               error();

            continue;
         }

         SDL_GPUColorTargetInfo const color_target_info{
            .texture{ swap_chain_texture },
            .clear_color{ 0.1f, 0.1f, 0.1f, 1.0f },
            .load_op{ SDL_GPU_LOADOP_CLEAR },
            .store_op{ SDL_GPU_STOREOP_STORE },
         };

         SDL_GPURenderPass& render_pass{ *SDL_BeginGPURenderPass(command_buffer, &color_target_info, 1, nullptr) };
         SDL_BindGPUGraphicsPipeline(&render_pass, pipeline.get());
         SDL_DrawGPUPrimitives(&render_pass, 3, 1, 0, 0);
         SDL_EndGPURenderPass(&render_pass);

         if (not SDL_SubmitGPUCommandBuffer(command_buffer))
            error();
      }
   }

   SDL_QuitSubSystem(SDL_INIT_VIDEO);

   return 0;
}
catch (std::exception const& exception)
{
   std::print("{}", exception.what());
   return 0;
}
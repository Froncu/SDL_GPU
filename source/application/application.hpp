#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "camera/camera.hpp"
#include "constants.hpp"
#include "pch.hpp"
#include "scene/scene.hpp"
#include "scene/vertex.hpp"
#include "shader/shader.hpp"
#include "utility/unique_pointer.hpp"

namespace fro
{
   struct Transforms
   {
      glm::mat4 camera;
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

         UniquePointer<SDL_GPUDevice> const gpu_device_{
            SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, DEBUG, nullptr),
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

         SDL_GPUGraphicsPipelineCreateInfo const pipeline_create_info_{
            .vertex_shader{ &vertex_shader_.native_shader() },
            .fragment_shader{ &fragment_shader_.native_shader() },
            .vertex_input_state{
               .vertex_buffer_descriptions{ Vertex::BUFFER_DESCRIPTIONS.data() },
               .num_vertex_buffers{ static_cast<Uint32>(Vertex::BUFFER_DESCRIPTIONS.size()) },
               .vertex_attributes{ Vertex::ATTRIBUTES.data() },
               .num_vertex_attributes{ static_cast<Uint32>(Vertex::ATTRIBUTES.size()) }
            },
            .primitive_type{ SDL_GPU_PRIMITIVETYPE_TRIANGLELIST },
            .rasterizer_state{
               .fill_mode{ SDL_GPU_FILLMODE_FILL },
               .cull_mode{ SDL_GPU_CULLMODE_BACK },
               .front_face{ SDL_GPU_FRONTFACE_CLOCKWISE },
               .enable_depth_clip{ true }
            },
            .depth_stencil_state{
               .compare_op{ SDL_GPU_COMPAREOP_LESS },
               .enable_depth_test{ true },
               .enable_depth_write{ true }
            },
            .target_info{
               .color_target_descriptions{ &color_target_description_ },
               .num_color_targets{ 1 },
               .depth_stencil_format{ SDL_GPU_TEXTUREFORMAT_D32_FLOAT },
               .has_depth_stencil_target{ true }
            }
         };

         UniquePointer<SDL_GPUGraphicsPipeline> const pipeline_{
            SDL_CreateGPUGraphicsPipeline(gpu_device_.get(), &pipeline_create_info_),
            std::bind(SDL_ReleaseGPUGraphicsPipeline, gpu_device_.get(), std::placeholders::_1)
         };

         Scene scene_{ *gpu_device_, "resources/scenes/flight_helmet.gltf" };
         SDL_GPUBufferBinding index_buffer_binding{ scene_.index_buffer() };
         SDL_GPUBufferBinding vertex_buffer_binding{ scene_.vertex_buffer() };

         Camera camera_{};

         SDL_GPUTextureCreateInfo depth_texture_create_info{
            .type{ SDL_GPU_TEXTURETYPE_2D },
            .format{ SDL_GPU_TEXTUREFORMAT_D32_FLOAT },
            .usage{ SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET },
            .width{ 1 },
            .height{ 1 },
            .layer_count_or_depth{ 1 },
            .num_levels{ 1 },
         };

         UniquePointer<SDL_GPUTexture> depth_texture_{
            SDL_CreateGPUTexture(gpu_device_.get(), &depth_texture_create_info),
            std::bind(SDL_ReleaseGPUTexture, gpu_device_.get(), std::placeholders::_1)
         };

         glm::vec3 movement_{};
         float movement_speed_{ 0.005f };
   };
}

#endif
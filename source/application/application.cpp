#include "application.hpp"
#include "scene/index.hpp"

namespace fro
{
   Application::Application(std::span<char const* const> const)
   {
      SDL_GPUSamplerCreateInfo const sampler_create_info{};
      sampler_ = {
         SDL_CreateGPUSampler(gpu_device_.get(), &sampler_create_info),
         std::bind(SDL_ReleaseGPUSampler, gpu_device_.get(), std::placeholders::_1),
      };

      UniquePointer<SDL_Surface> leather{
         IMG_Load("resources/leather.png"),
         SDL_DestroySurface
      };

      leather.reset(SDL_ConvertSurface(leather.get(), SDL_PIXELFORMAT_RGBA32));

      UniquePointer<SDL_Surface> plastic{
         IMG_Load("resources/plastic.png"),
         SDL_DestroySurface
      };

      plastic.reset(SDL_ConvertSurface(plastic.get(), SDL_PIXELFORMAT_RGBA32));

      SDL_GPUTextureCreateInfo const texture_create_info{
         .type{ SDL_GPU_TEXTURETYPE_2D_ARRAY },
         .format{ SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM_SRGB },
         .usage{ SDL_GPU_TEXTUREUSAGE_SAMPLER },
         .width{ static_cast<Uint32>(leather->w) },
         .height{ static_cast<Uint32>(leather->h) },
         .layer_count_or_depth{ 10 },
         .num_levels{ 1 }
      };

      base_color_texture_ = {
         SDL_CreateGPUTexture(gpu_device_.get(), &texture_create_info),
         std::bind(SDL_ReleaseGPUTexture, gpu_device_.get(), std::placeholders::_1),
      };

      auto const image_size{ static_cast<Uint32>(leather->pitch * leather->h) };
      SDL_GPUTransferBufferCreateInfo const transfer_buffer_create_info{
         .usage{ SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD },
         .size{ image_size * 10 }
      };

      UniquePointer<SDL_GPUTransferBuffer> const transfer_buffer{
         SDL_CreateGPUTransferBuffer(gpu_device_.get(), &transfer_buffer_create_info),
         std::bind(SDL_ReleaseGPUTransferBuffer, gpu_device_.get(), std::placeholders::_1),
      };

      auto* const transfer_data{
         std::bit_cast<std::uint8_t*>(SDL_MapGPUTransferBuffer(gpu_device_.get(), transfer_buffer.get(), false))
      };

      std::memcpy(transfer_data, leather->pixels, image_size);
      std::memcpy(transfer_data + image_size, plastic->pixels, image_size);
      std::memcpy(transfer_data + image_size * 3, leather->pixels, image_size);
      SDL_UnmapGPUTransferBuffer(gpu_device_.get(), transfer_buffer.get());

      SDL_GPUTextureTransferInfo const leather_transfer_info{
         .transfer_buffer{ transfer_buffer.get() }
      };

      SDL_GPUTextureRegion const leather_region{
         .texture{ base_color_texture_.get() },
         .layer{ 0 },
         .w{ static_cast<Uint32>(leather->w) },
         .h{ static_cast<Uint32>(leather->h) },
         .d{ 1 }
      };

      SDL_GPUTextureTransferInfo const plastic_transfer_info{
         .transfer_buffer{ transfer_buffer.get() },
         .offset{ image_size }
      };

      SDL_GPUTextureRegion const plastic_region{
         .texture{ base_color_texture_.get() },
         .layer{ 1 },
         .w{ static_cast<Uint32>(leather->w) },
         .h{ static_cast<Uint32>(leather->h) },
         .d{ 1 }
      };

      SDL_GPUTextureTransferInfo const leather_transfer_info2{
         .transfer_buffer{ transfer_buffer.get() },
         .offset{ image_size * 3 }
      };

      SDL_GPUTextureRegion const leather_region2{
         .texture{ base_color_texture_.get() },
         .layer{ 3 },
         .w{ static_cast<Uint32>(leather->w) },
         .h{ static_cast<Uint32>(leather->h) },
         .d{ 1 }
      };

      SDL_GPUCommandBuffer* const command_buffer{ SDL_AcquireGPUCommandBuffer(gpu_device_.get()) };
      if (not command_buffer)
         std::abort();

      SDL_GPUCopyPass& copy_pass{ *SDL_BeginGPUCopyPass(command_buffer) };
      SDL_UploadToGPUTexture(&copy_pass, &leather_transfer_info, &leather_region, false);
      SDL_UploadToGPUTexture(&copy_pass, &plastic_transfer_info, &plastic_region, false);
      SDL_UploadToGPUTexture(&copy_pass, &leather_transfer_info2, &leather_region2, false);
      SDL_EndGPUCopyPass(&copy_pass);
      SDL_SubmitGPUCommandBuffer(command_buffer);

      int width;
      int height;
      SDL_GetWindowSize(window_.get(), &width, &height);

      SDL_Event event{
         .window{
            .type{ SDL_EVENT_WINDOW_RESIZED },
            .timestamp{ SDL_GetTicksNS() },
            .windowID{ SDL_GetWindowID(window_.get()) },
            .data1{ width },
            .data2{ height }
         }
      };

      SDL_PushEvent(&event);
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

      SDL_GPUDepthStencilTargetInfo const depth_target_info{
         .texture{ depth_texture_.get() },
         .clear_depth{ 1.0f },
         .load_op{ SDL_GPU_LOADOP_CLEAR },
         .store_op{ SDL_GPU_STOREOP_DONT_CARE },
      };

      SDL_GPURenderPass& render_pass{ *SDL_BeginGPURenderPass(command_buffer, &color_target_info, 1, &depth_target_info) };
      SDL_BindGPUGraphicsPipeline(&render_pass, pipeline_.get());
      SDL_BindGPUIndexBuffer(&render_pass, &index_buffer_binding, INDEX_SIZE);
      SDL_BindGPUVertexBuffers(&render_pass, 0, &vertex_buffer_binding, 1);

      camera_.move(movement_);

      Transforms transforms{
         .camera{ camera_.matrix() },
         .model{ glm::identity<glm::mat4>() },
      };
      SDL_PushGPUVertexUniformData(command_buffer, 0, &transforms, sizeof(transforms));

      SDL_GPUTextureSamplerBinding const texture_sampler_binding{
         .texture{ base_color_texture_.get() },
         .sampler{ sampler_.get() }
      };
      SDL_BindGPUFragmentSamplers(&render_pass, 0, &texture_sampler_binding, 1);
      std::uint32_t const material_index{ 0 };
      SDL_PushGPUFragmentUniformData(command_buffer, 0, &material_index, sizeof(material_index));
      for (SubMesh const& sub_mesh : scene_.sub_meshes())
         SDL_DrawGPUIndexedPrimitives(&render_pass, sub_mesh.index_count, 1, sub_mesh.index_offset, sub_mesh.vertex_offset, 0);

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

         case SDL_EVENT_MOUSE_MOTION:
            switch (event.motion.state)
            {
               case SDL_BUTTON_LMASK:
                  camera_.move({ event.motion.xrel * movement_speed_, event.motion.yrel * movement_speed_, 0.0f });
                  break;

               case SDL_BUTTON_RMASK:
                  camera_.move({ 0.0f, 0.0f, -event.motion.yrel * movement_speed_ });
                  break;
            }
            return true;

         case SDL_EVENT_WINDOW_RESIZED:
            depth_texture_create_info.width = event.window.data1;
            depth_texture_create_info.height = event.window.data2;
            depth_texture_.reset(SDL_CreateGPUTexture(gpu_device_.get(), &depth_texture_create_info));
            camera_.change_aspect_ratio(static_cast<float>(event.window.data1) / event.window.data2);
            return true;

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
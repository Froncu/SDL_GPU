#ifndef SHADER_HPP
#define SHADER_HPP

#include <filesystem>
#include <vector>

#include <SDL3_shadercross/SDL_shadercross.h>

#include "utility/unique_pointer.hpp"

struct SDL_GPUShader;
struct SDL_GPUDevice;

namespace fro
{
   class Shader final
   {
      public:
         explicit Shader(SDL_GPUDevice& gpu_device, std::filesystem::path const& path, std::string_view entry_point = "main",
            std::string_view include_directory = "", std::vector<SDL_ShaderCross_HLSL_Define> defines = {},
            bool enable_debug = false);

         [[nodiscard]] SDL_GPUShader& native_shader() const;

      private:
         UniquePointer<SDL_GPUShader> shader_;
   };
}

#endif
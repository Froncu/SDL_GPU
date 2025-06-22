#include <fstream>
#include <format>

#include <SDL3/SDL.h>
#include <SDL3_shadercross/SDL_shadercross.h>

#include "shader.hpp"

namespace fro
{
   Shader::Shader(SDL_GPUDevice& gpu_device, std::filesystem::path const& path, std::string_view const entry_point,
      std::string_view const include_directory, std::vector<SDL_ShaderCross_HLSL_Define> defines, bool const enable_debug)
      : shader_{
         [&gpu_device, &path, &entry_point, &include_directory, &defines, &enable_debug]
         {
            if (not std::filesystem::exists(path))
               throw std::runtime_error{ std::format("file \"{}\" does not exist!", path.string()) };

            if (not std::filesystem::is_regular_file(path))
               throw std::runtime_error{ std::format("file \"{}\" is not a regular file!", path.string()) }; 

            std::string source_code{};
            if (std::ifstream file{ path, std::ifstream::in }; file.is_open())
               source_code.assign(std::istreambuf_iterator(file), std::istreambuf_iterator<char>());
            else
               throw std::runtime_error{ std::format("failed to open \"{}\"!", path.string()) };

            SDL_ShaderCross_ShaderStage shader_stage;
            if (std::string const extension{ path.extension().string() }; extension == ".vert")
               shader_stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
            else if (extension == ".frag")
               shader_stage = SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
               // else if (extension == ".comp")
               //    shader_type = SDL_SHADERCROSS_SHADERSTAGE_COMPUTE;
            else
               throw std::runtime_error{ std::format("file extension \"{}\" is not a valid shader extension!", extension) };

            if (entry_point.empty())
               throw std::runtime_error{
                  std::format("the entry point for the \"{}\" shader cannot be empty!", path.string())
               };

            // TODO: provide parameters for this
            SDL_PropertiesID constexpr properties{};

            std::string const filename{ path.filename().string() };
            SDL_ShaderCross_HLSL_Info const compile_info{
               .source{ source_code.c_str() },
               .entrypoint{ entry_point.data() },
               .include_dir{ include_directory.data() },
               .defines{ defines.data() }, // TODO: has to be always nullptr terminated
               .shader_stage{ shader_stage },
               .enable_debug{ enable_debug },
               .name{ filename.c_str() },
               .props{ properties }
            };

            std::size_t byte_code_size;
            auto const byte_code{
               static_cast<Uint8 const* const>(SDL_ShaderCross_CompileSPIRVFromHLSL(&compile_info, &byte_code_size))
            };

            UniquePointer<SDL_ShaderCross_GraphicsShaderMetadata> const shader_data{
               SDL_ShaderCross_ReflectGraphicsSPIRV(byte_code, byte_code_size, properties),
               SDL_free
            };
            if (not shader_data)
               throw std::runtime_error{ std::format("failed to reflect \"{}\" data ({})!", path.string(), SDL_GetError()) };

            SDL_GPUShaderCreateInfo const create_info{
               .code_size{ byte_code_size },
               .code{ byte_code },
               .entrypoint{ "main" },
               .format{ SDL_GPU_SHADERFORMAT_SPIRV }, // TODO: support multiple formats
               .stage{ static_cast<SDL_GPUShaderStage>(shader_stage) },
               .num_samplers{ shader_data->num_samplers },
               .num_storage_textures{ shader_data->num_storage_textures },
               .num_storage_buffers{ shader_data->num_storage_buffers },
               .num_uniform_buffers{ shader_data->num_uniform_buffers },
               .props{ properties }
            };
            UniquePointer<SDL_GPUShader> shader{
               SDL_CreateGPUShader(&gpu_device, &create_info),
               std::bind(SDL_ReleaseGPUShader, &gpu_device, std::placeholders::_1)
            };
            if (not shader)
               throw std::runtime_error{
                  std::format("failed to create \"{}\" shader ({})!", path.string(), SDL_GetError())
               };

            return shader;
         }()
      }
   {
   }

   SDL_GPUShader& Shader::native_shader() const
   {
      return *shader_;
   }
}
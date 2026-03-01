#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "pch.hpp"

class Camera final
{
   static glm::vec3 constexpr TARGET{ 0.0f, 0.0f, 0.0f };
   static glm::vec3 constexpr UP{ 0.0f, 1.0f, 0.0f };

   public:
      Camera() = default;
      Camera(Camera const&) = default;
      Camera(Camera&&) = default;

      ~Camera() = default;

      Camera& operator=(Camera const&) = default;
      Camera& operator=(Camera&&) = default;

      void change_field_of_view(float field_of_view);
      void change_aspect_ratio(float aspect_ratio);
      void change_near_plane(float near_plane);
      void change_far_plane(float far_plane);

      void move(glm::vec3 const& direction);

      [[nodiscard]] glm::mat4 matrix() const;

   private:
      [[nodiscard]] glm::mat4 compute_matrix() const;

      float field_of_view_{ 60.0f };
      float aspect_ratio_{ 1.0f };
      float near_plane_{ 0.1f };
      float far_plane_{ 100.0f };
      glm::vec3 position_{ 0.0f, 0.0f, -1.0f };

      mutable glm::mat4 matrix_{ compute_matrix() };
      mutable bool dirty_{ false };
};

#endif
#include "camera.hpp"

void Camera::change_field_of_view(float field_of_view)
{
   field_of_view_ = field_of_view;
   dirty_ = true;
}

void Camera::change_aspect_ratio(float aspect_ratio)
{
   aspect_ratio_ = aspect_ratio;
   dirty_ = true;
}

void Camera::change_near_plane(float near_plane)
{
   near_plane_ = near_plane;
   dirty_ = true;
}

void Camera::change_far_plane(float far_plane)
{
   far_plane_ = far_plane;
   dirty_ = true;
}

void Camera::move(glm::vec3 const& direction)
{
   float const length{ glm::length(position_) };
   glm::vec3 const forward{ -position_ / length };
   glm::vec3 const right{ glm::normalize(glm::cross(forward, UP)) };
   glm::vec3 const up{ glm::cross(right, forward) };

   position_ +=
      right * length * direction.x +
      up * length * direction.y +
      forward * length * direction.z;

   dirty_ = true;
}

glm::mat4 Camera::matrix() const
{
   if (dirty_)
   {
      matrix_ = compute_matrix();
      dirty_ = false;
   }

   return matrix_;
}

glm::mat4 Camera::compute_matrix() const
{
   glm::mat4 const projection_{ glm::perspective(glm::radians(field_of_view_), aspect_ratio_, near_plane_, far_plane_) };
   glm::mat4 const view_{ glm::lookAt(position_, TARGET, UP) };
   return projection_ * view_;
}
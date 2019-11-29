#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "transformation.h"
#include "bspline.h"

namespace MH
{
    enum CameraMovement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN,
    };

    class Camera
    {
        public:
        Camera(float width, float height)
        :
        width(width),
        height(height)
        {
            // default poosition
            transformation = Transformation(glm::vec3(0.0f, 0.0f, 1.0f));
            set_ortho(true);
        }
        
        void set_ortho(bool value)
        {
            is_ortho = value;
            if(is_ortho)
            {
                transformation.set_position(glm::vec3(0.0f, 0.0f, 1.0f));
                
                camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
                projection = glm::ortho(
                    -width / (2.0f * zoom),
                    width / (2.0f * zoom),
                    -height / (2.0f * zoom),
                    height / (2.0f * zoom)
                    );
            }
            else
            {
                projection = glm::perspective(glm::radians(fov), width / height, 0.1f, 100.0f);
            }
        }

        void process_keyboard(CameraMovement direction, float deltaTime)
        {
            float velocity = 300.0f * deltaTime;
            if(is_ortho)
            {
                if (direction == UP)
                    transformation.move_up (1.0f * velocity / zoom);
                if (direction == DOWN)
                    transformation.move_up (-1.0f * velocity / zoom);
                if (direction == LEFT)
                    transformation.move_right(-1.0f * velocity / zoom);
                if (direction == RIGHT)
                    transformation.move_right(1.0f * velocity / zoom);
            }
            else
            {
                velocity = 3.0f * deltaTime;
                if (direction == UP)
                    transformation.translate(velocity * camera_front);
                if (direction == DOWN)
                    transformation.translate(-velocity * camera_front);
                if (direction == LEFT)
                    transformation.translate(-glm::normalize(glm::cross(camera_front, transformation.get_up())) * velocity);
                if (direction == RIGHT)
                    transformation.translate(glm::normalize(glm::cross(camera_front, transformation.get_up())) * velocity);
                if (direction == FORWARD)
                {
                    auto crossValue = glm::cross(
                                                 glm::cross(camera_front, transformation.get_up()
                                                            ), camera_front
                    );
                    transformation.translate(glm::normalize(crossValue) * velocity);
                }
                if (direction == BACKWARD)
                {
                    auto crossValue = glm::cross(
                                                 glm::cross(camera_front, transformation.get_up()
                                                            ), camera_front
                                                 );
                    transformation.translate(-glm::normalize(crossValue) * velocity);
                }
                
                if(!orbit)
                {
                    if(surface != nullptr)
                    {
                        orbitDistance = (transformation.get_position() - surface->center).length();
                    }
                    else
                    {
                        orbitDistance = transformation.get_position().length();
                    }
                }
            }
        }

        void process_mouse_scroll(float y_offset)
        {
            if(is_ortho)
            {
                zoom += y_offset * 7.0f;
                if(zoom >= 2000.0f)
                {
                    zoom = 2000.0f;
                }
                if(zoom <= 0.05f)
                {
                    zoom = 0.05f;
                }
                projection = glm::ortho(
                                        -width / (2.0f * zoom),
                                        width / (2.0f * zoom),
                                        -height / (2.0f * zoom),
                                        height / (2.0f * zoom)
                                        );
            }
            else
            {
                if(orbit)
                {
                    orbitDistance += y_offset * 0.1f;
                    if(surface != nullptr)
                    {
                        transformation.set_position(surface->center + (-camera_front * orbitDistance));
                    }
                    else
                    {
                        transformation.set_position(-camera_front * orbitDistance);
                    }
                }
                else
                {
                    if (fov >= 1.0f && fov <= 45.0f)
                        fov -= y_offset;
                    if (fov <= 1.0f)
                        fov = 1.0f;
                    if (fov >= 45.0f)
                        fov = 45.0f;
                    projection = glm::perspective(glm::radians(fov), width / height, 0.1f, 100.0f);
                }
            }
        }

        float zoom = 300.0f;
        glm::mat4 projection;
        Transformation transformation;
        float width;
        float height;
        bool is_ortho = true;
        float yaw   = -90.0f;    // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
        float pitch =  0.0f;
        float fov   =  45.0f;
        glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
        bool right_pressed = false;
        bool orbit = false;
        float orbitDistance = 2.0f;
        std::shared_ptr<BSplineSurface> surface = nullptr;
    };
} // namespace MH

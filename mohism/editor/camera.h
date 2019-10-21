#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "transformation.h"

namespace MH
{
    enum CameraMovement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
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

            projection = glm::ortho(
                -width / (2.0f * zoom), 
                width / (2.0f * zoom), 
                -height / (2.0f * zoom), 
                height / (2.0f * zoom)
                );
        }

        void process_keyboard(CameraMovement direction, float deltaTime)
        {
            float velocity = 300.0f * deltaTime;
            if (direction == UP)
                transformation.move_up (1.0f * velocity / zoom);
            if (direction == DOWN)
                transformation.move_up (-1.0f * velocity / zoom);
            if (direction == LEFT)
                transformation.move_right(-1.0f * velocity / zoom);
            if (direction == RIGHT)
                transformation.move_right(1.0f * velocity / zoom);
        }

        void process_mouse_scroll(float y_offset)
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

        float zoom = 300.0f;
        glm::mat4 projection;
        Transformation transformation;
        float width;
        float height;
    };
} // namespace MH

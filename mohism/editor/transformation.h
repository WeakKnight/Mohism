#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace MH
{
    class Transformation
    {
        public:
        
        Transformation(glm::vec3 position)
        :position(position),
        forward(glm::vec3(0.0f, 0.0f, -1.0f)),
        up(glm::vec3(0.0f, 1.0f, 0.0f)),
        right(glm::vec3(1.0f, 0.0f, 0.0f))
        {
            need_update = true;
        }

        Transformation()
        :position(glm::vec3(0.0f, 0.0f, 0.0f)),
        forward(glm::vec3(0.0f, 0.0f, -1.0f)),
        up(glm::vec3(0.0f, 1.0f, 0.0f)),
        right(glm::vec3(1.0f, 0.0f, 0.0f))
        {
            need_update = true;
        }

        void translate(glm::vec3 offset)
        {
            need_update = true;
            position += offset;
        }

        void set_position(glm::vec3 position)
        {
            need_update = true;
            this->position = position;
        }

        const glm::mat4& get_local_to_world_mat()
        {
            if(need_update)
            {
                local_to_world = glm::translate(glm::mat4(1.0f), position);
                need_update = false;
            }
            
            return local_to_world;
        }

        const glm::vec3& get_position() const
        {
            return position;
        }

        glm::mat4 get_view_matrix() const
        {
            return glm::lookAt(position, position + forward, up);
        }

        glm::vec3 get_forward() const
        {
            return forward;
        }

        glm::vec3 get_right() const
        {
            return right;
        }

        glm::vec3 get_up() const
        {
            return up;
        }

        void move_forward(float value)
        {
            translate(forward * value);
        }

        void move_right(float value)
        {
            translate(right * value);
        }

        void move_up(float value)
        {
            translate(up * value);
        }

        private:
        
        bool need_update = false;
        
        glm::vec3 forward;
        glm::vec3 up;
        glm::vec3 right;
        glm::vec3 position;
        glm::mat4 local_to_world;
        glm::mat4 world_to_local;
    };
} // namespace MH

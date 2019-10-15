#pragma once

#include "glm/vec3.hpp"
#include "transformation.h"
#include "glad/glad.h"

namespace MH
{
    class BSpline
    {
    public:
        
        BSpline()
        {
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
        }
        
        ~BSpline()
        {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
        }
        
        void remove_control_point(size_t index)
        {
            control_points.erase(control_points.begin() + index);
            need_updated = true;
        }
        void add_control_point(glm::vec3 point)
        {
            control_points.push_back(point);
            need_updated = true;
        }
        void add_control_points(std::vector<glm::vec3>& points)
        {
            control_points.insert(control_points.end(), points.begin(), points.end());
            need_updated = true;
        }
        void insert_control_point(size_t index, glm::vec3 point)
        {
            control_points.insert(control_points.begin() + index, point);
            need_updated = true;
        }
        
        void update_render_data()
        {
            if(need_updated)
            {
                need_updated = false;
                
//                float vertices[] = {
//                    // positions
//                    50.0f, -50.0f, 0.0f,
//                    -50.0f, -50.0f, 0.0f,
//                    0.0f,  50.0f, 0.0f
//                };
                
                vertices.clear();
                
                for(int i = 0; i < control_points.size(); i++)
                {
                    const auto& point = control_points[i];
                    vertices.push_back(point.x);
                    vertices.push_back(point.y);
                    vertices.push_back(point.z);
                }
                
                glBindVertexArray(VAO);
                
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_DYNAMIC_DRAW);
                
//                 glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
                
                // position attribute
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);
            }
        }
        
        void draw()
        {
            update_render_data();
            glBindVertexArray(VAO);
            glDrawArrays(GL_LINE_STRIP, 0, control_points.size());
        }
        
    private:
        
        bool need_updated = false;
        
        std::vector<float> vertices;
        std::vector<glm::vec3> line_segments;
        std::vector<glm::vec3> control_points;
        std::vector<float> knot_vector;
        
        unsigned int VAO;
        unsigned int VBO;
    };
} // namespace MH

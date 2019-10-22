#pragma once

#include "glm/vec3.hpp"
#include "transformation.h"
#include "glad/glad.h"
#include "core/log.h"
#include "shader.h"

#define OPEN_KNOT_MODIFIED_UNIFORM_VECTOR 1
#define FLOATING_UNIFORM_VECTOR 2

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
        
        std::vector<glm::vec3>& get_control_points()
        {
            return control_points;
        }
        
        glm::vec3 get_control_point(int index)
        {
            if(index < 0)
            {
                index = 0;
            }
            
            if (index >= control_points.size())
            {
                index = control_points.size() - 1;
            }
            
            return control_points[index];
        }
        
        std::vector<glm::mat4>& get_control_point_matrixes()
        {
            return control_point_matrixes;
        }
        
        void remove_control_point(size_t index)
        {
            control_points.erase(control_points.begin() + index);
            control_point_matrixes.erase(control_point_matrixes.begin() + index);
            need_updated = true;
        }
        
        void add_control_point(glm::vec3 point)
        {
            control_points.push_back(point);
            control_point_matrixes.push_back(calculate_control_point_matrix(point));
            
            need_updated = true;
        }
        void add_control_points(std::vector<glm::vec3>& points)
        {
            control_points.insert(control_points.end(), points.begin(), points.end());
            
            std::vector<glm::mat4> matrixes;
            for(int i = 0; i < points.size(); i ++)
            {
                auto point = points[i];
                matrixes.push_back(calculate_control_point_matrix(point));
            }
            control_point_matrixes.insert(control_point_matrixes.end(), matrixes.begin(), matrixes.end());
            
            need_updated = true;
        }
        
        void insert_control_point(size_t index, glm::vec3 point)
        {
//            if(index < 0)
//            {
//                index = 0;
//            }
//            
//            if(index >= control_points.size())
//            {
//                index = control_points.size() - 1;
//            }
            
            control_points.insert(control_points.begin() + index, point);
            control_point_matrixes.insert(control_point_matrixes.begin() + index, calculate_control_point_matrix(point));
            
            need_updated = true;
        }
        
        void remove_knot_vector(size_t index)
        {
            knot_vector.erase(knot_vector.begin() + index);
            need_updated = true;
        }
        void add_knot_vector(float value)
        {
            knot_vector.push_back(value);
            need_updated = true;
        }
        void add_knot_vector(std::vector<float>& values)
        {
            knot_vector.insert(knot_vector.end(), values.begin(), values.end());
            need_updated = true;
        }
        void insert_knot_vector(size_t index, float value)
        {
            knot_vector.insert(knot_vector.begin() + index, value);
            need_updated = true;
        }
        
        void generate_modified_open_knot_uniform_vector()
        {
            knot_vector.clear();
            for(int i = 0; i < used_knot_num; i++)
            {
                if(i <= k())
                {
                    knot_vector.push_back(0.0f);
                }
                else if (i >= (used_knot_num - k() - 1))
                {
                    if(i == (used_knot_num - 1))
                    {
                        knot_vector.push_back(knot_vector[used_knot_num - k() - 2] + 1.5f);
                    }
                    else
                    {
                        knot_vector.push_back(knot_vector[used_knot_num - k() - 2] + 1.0f);
                    }
                }
                else
                {
                    knot_vector.push_back(knot_vector[i - 1] + 1.0f);
                }
            }
        }
        
        void generate_float_uniform_knot_vector()
        {
            knot_vector.clear();
            for(int i = 0; i < used_knot_num; i++)
            {
                knot_vector.push_back(i);
            }
        }
        
        void process_knot_vector_by_degree_and_control_points(int type = OPEN_KNOT_MODIFIED_UNIFORM_VECTOR)
        {
            knot_vector_type = type;
            
            used_knot_num = control_points.size() + k() + 1;
            // N = count of control points + k, N = count of knot vector + 1
            if(knot_vector.size() == used_knot_num)
            {
                // do nothing
                LOG_INFO("good");
            }
            // extra
            else if(knot_vector.size() > used_knot_num)
            {
                LOG_INFO("Extra knot vector, ok");
            }
            // not enough, regenerate knot vector
            else
            {
                LOG_INFO("need more knots");
                if(type == OPEN_KNOT_MODIFIED_UNIFORM_VECTOR)
                {
                    generate_modified_open_knot_uniform_vector();
                }
                else if(type == FLOATING_UNIFORM_VECTOR)
                {
                    generate_float_uniform_knot_vector();
                }
                else
                {
                    assert(false);
                }
            }
            
            for(int i = 0; i < used_knot_num - 1; i++)
            {
                if(t(i) < t(i+1))
                {
                    if(i > j_max)
                    {
                        j_max = i;
                    }
                }
            }
            
            domain.clear();
            domain.push_back(t(k()));
            domain.push_back(t(N() - k()));
        }
        
        void update_render_data()
        {
            if(need_updated)
            {
                need_updated = false;
                
                process_knot_vector_by_degree_and_control_points(knot_vector_type);
                
                sample_line_segments();
                
                vertices.clear();
                
                for(int i = 0; i < control_points.size(); i++)
                {
                    const auto& point = control_points[i];
                    vertices.push_back(point.x);
                    vertices.push_back(point.y);
                    vertices.push_back(point.z);
                }
                
                for(int i = 0; i < line_segments.size(); i++)
                {
                    const auto& point = line_segments[i];
                    vertices.push_back(point.x);
                    vertices.push_back(point.y);
                    vertices.push_back(point.z);
                }
                
                glBindVertexArray(VAO);
                
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_DYNAMIC_DRAW);
                
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(0);
            }
        }
        
        void draw(Shader* shader)
        {
            update_render_data();
            glBindVertexArray(VAO);
            // polygon
            shader->setVec4("customColor", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
            glDrawArrays(GL_LINE_STRIP, 0, control_points.size());
            
            shader->setVec4("customColor", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
            glDrawArrays(GL_LINE_STRIP, control_points.size(), line_segments.size());
        }
        
        void set_degree(int value)
        {
            degree = value;
        }
        
        int get_degree()
        {
            return degree;
        }
        
        void mark_need_update()
        {
            need_updated = true;
        }
        
    private:
        
        glm::mat4 calculate_control_point_matrix(glm::vec3 point)
        {
            auto result = glm::translate(glm::mat4(1.0f), point);
            return result;
        }
        
        void sample_line_segments(int sample_count = 200)
        {
            line_segments.clear();
            
            float domain_length = domain[1] - domain[0];
            
            float delta = domain_length / (float)(sample_count + 2);
            
            line_segments.push_back(evaluate(domain[0]));
            for(int i = 1; i <= sample_count; i ++)
            {
                line_segments.push_back(evaluate(domain[0] + (i) * delta));
            }
            line_segments.push_back(evaluate(domain[1]));
        }
        
        glm::vec3 evaluate(float _t)
        {
            glm::vec3 result(0.0f, 0.0f, 0.0f);
            
            for(int i = 0; i < control_points.size(); i++)
            {
                float blending_value = blending_func(i, k(), _t);
                result += (blending_value * control_points[i]);
            }
            
            return result;
        }
        
        float blending_func(int i, int _k, float _t)
        {
            if(_k == 0)
            {
                if(_t == t(j_max + 1))
                {
                    return 1;
                }
                
                if(_t >= t(i) && _t < t(i + 1))
                {
                    return 1;
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                assert(_k > 0);
                if(t(i) < t(i + 1 + _k))
                {
                    float left_factor = (_t - t(i)) / (t(i + _k) - t(i));
                    float left_blending = blending_func(i, _k - 1, _t);
                    float left_result = left_factor * left_blending;
                    
                    if(left_blending == 0.0f)
                    {
                        left_result = 0.0f;
                    }
                    
                    float right_factor = (t(i + 1 + _k) - _t)/(t(i + 1 + _k) - t(i + 1));
                    float right_blending = blending_func(i + 1, _k - 1, _t);
                    float right_result = right_factor * right_blending;
                    
                    if(right_blending == 0.0f)
                    {
                        right_result = 0.0f;
                    }
                    
                    return left_result + right_result;
                }
                else
                {
                    return 0;
                }
            }
        }
        
        inline int N()
        {
            return knot_vector.size() - 1;
        }
        
        inline int k()
        {
            return degree;
        }
        
        int j_max = 0;
        
        
        inline float t(int i)
        {
            return knot_vector[i];
        }
        
        int knot_vector_type = OPEN_KNOT_MODIFIED_UNIFORM_VECTOR;
        
        bool need_save_knot_vector = false;
        bool need_updated = false;
        
        std::vector<glm::vec3> control_points;
        std::vector<glm::mat4> control_point_matrixes;
        
        int degree;
        std::vector<float> knot_vector;
        int used_knot_num;
        
        std::vector<float> domain;
        
        std::vector<float> vertices;
        std::vector<glm::vec3> line_segments;
        
        unsigned int VAO;
        unsigned int VBO;
    };
} // namespace MH

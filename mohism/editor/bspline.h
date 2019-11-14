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
        
        std::vector<float>& get_knot_vector()
        {
            return knot_vector;
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
        
        void calculate_jmax()
        {
            for(int i = 0; i < knot_vector.size() - 1; i++)
            {
                if(t(i) < t(i+1))
                {
                    if(i > j_max)
                    {
                        j_max = i;
                    }
                }
            }
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
            if(show_polygon)
            {
                shader->setVec4("customColor", glm::vec4(0.3f, 0.0f, 0.52f, 1.0f));
                glDrawArrays(GL_LINE_STRIP, 0, control_points.size());
            }
            // the curve
            if(show_curve)
            {
                if(is_special_color)
                {
                    shader->setVec4("customColor", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
                }
                else
                {
                    shader->setVec4("customColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                }
                glDrawArrays(GL_LINE_STRIP, control_points.size(), line_segments.size());
            }
            glBindVertexArray(0);
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
        
        void set_dimension(int value)
        {
            dimension = value;
        }
        
        int get_dimension()
        {
            return dimension;
            
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
        
        bool show_polygon= true;
        bool show_curve = true;
        bool show_control_point = true;
        bool is_special_color = false;
        
    private:
        
        glm::mat4 calculate_control_point_matrix(glm::vec3 point)
        {
            auto result = glm::translate(glm::mat4(1.0f), point);
            return result;
        }
        
        void sample_line_segments(int sample_count = 300)
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
        
        int dimension;
        
        unsigned int VAO;
        unsigned int VBO;
    };
    
    class BSplineSurface
    {
    public:
        BSplineSurface()
        {
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
        }
        
        ~BSplineSurface()
        {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
        }
        
        // m
        std::vector<float> knot_u;
        // n
        std::vector<float> knot_v;
        // m x n
        std::vector<glm::vec3> control_points;
        
        int degree_u;
        int degree_v;
        
        int knot_length_u;
        int knot_length_v;
        
        void compute_derived_date()
        {
            compute_domain();
            compute_grid();
            compute_model_spline();
            compute_segments();
        }
        
        void draw(Shader* shader)
        {
            // draw_grid(shader);
            
            glBindVertexArray(VAO);
            shader->setVec4("customColor", glm::vec4(0.3f, 0.4f, 0.52f, 1.0f));
            glDrawArrays(GL_LINES, 0, segments.size());
            glBindVertexArray(0);
        }
        
        glm::vec3 evaluate(float u, float v)
        {
            int n = knot_length_v - degree_v - 1 - 1;
            int m = knot_length_u - degree_u - 1 - 1;
            
            glm::vec3 result = glm::vec3(0.0f, 0.0f, 0.0f);
            
            for(int i = 0; i <= m; i++)
            {
                for(int j = 0; j <= n; j++)
                {
                    
                    result +=
                    P(i, j) * model_u->blending_func(i, model_u->get_degree(), u)
                    * model_v->blending_func(j, model_v->get_degree(), v);
                }
            }
            
            return result;
        }
        
    private:
        
        glm::vec3 P(int i, int j)
        {
            int n = knot_length_v - degree_v - 1 - 1;
            int width = n + 1;
            
            return control_points[width * i + j];
        }
        
        void draw_grid(Shader* shader)
        {
            for(int i = 0; i < grid_row.size(); i++)
            {
                grid_row[i]->draw(shader);
            }
            
            for(int i = 0; i < grid_column.size(); i++)
            {
                grid_column[i]->draw(shader);
            }
        }
        
        void compute_domain()
        {
            domain_u = glm::vec2(knot_u[degree_u], knot_u[knot_length_u - 1 - degree_u]);
            domain_v = glm::vec2(knot_v[degree_v], knot_v[knot_length_v - 1 - degree_v]);
        }
        
        // make each row a bspline, each column a bspline
        void compute_grid()
        {
            int n = knot_length_v - degree_v - 1 - 1;
            int m = knot_length_u - degree_u - 1 - 1;
            
            // row
            for(int i = 0; i <= m; i++)
            {
                auto row = get_row(i);
                // make row a spline
                auto rowSpline = std::make_shared<BSpline>();
                rowSpline->set_degree(degree_v);
                rowSpline->add_knot_vector(knot_v);
                rowSpline->add_control_points(row);
                rowSpline->mark_need_update();
                
                grid_row.push_back(rowSpline);
            }
            
            // column
            for(int i = 0; i <= n; i++)
            {
                auto column = get_column(i);
                // make row a spline
                auto columnSpline = std::make_shared<BSpline>();
                columnSpline->set_degree(degree_u);
                columnSpline->add_knot_vector(knot_u);
                columnSpline->add_control_points(column);
                columnSpline->mark_need_update();
                
                grid_column.push_back(columnSpline);
            }
        }
        
        void compute_model_spline()
        {
            model_u = std::make_shared<BSpline>();
            model_u->set_degree(degree_u);
            model_u->add_knot_vector(knot_u);
            model_u->calculate_jmax();
            
            model_v = std::make_shared<BSpline>();
            model_v->set_degree(degree_v);
            model_v->add_knot_vector(knot_v);
            model_v->calculate_jmax();
        }
        
        void compute_segments(int sub_u = 50, int sub_v = 50)
        {
            float length_u = domain_u.y - domain_u.x;
            float step_u = length_u / (float)sub_u;
            
            float length_v = domain_v.y - domain_v.x;
            float step_v = length_v / (float)sub_v;
            
            for(int step_index_u = 0; step_index_u <= sub_u; step_index_u++)
            {
                float u = domain_u.x + step_u * step_index_u;
                for(int step_index_v = 0; step_index_v < sub_v; step_index_v++)
                {
                    float v = domain_v.x + step_v * step_index_v;
                    float next_v = domain_v.x + step_v * (step_index_v + 1);
                    
                    glm::vec3 a = evaluate(u, v);
                    glm::vec3 b = evaluate(u, next_v);
                    
                    segments.push_back(a);
                    segments.push_back(b);
                }
            }
            
            for(int step_index_v = 0; step_index_v <= sub_v; step_index_v++)
            {
                float v = domain_v.x + step_v * step_index_v;
                for(int step_index_u = 0; step_index_u < sub_u; step_index_u++)
                {
                    float u = domain_u.x + step_u * step_index_u;
                    float next_u = domain_u.x + step_u * (step_index_u + 1);
                    
                    glm::vec3 a = evaluate(u, v);
                    glm::vec3 b = evaluate(next_u, v);
                    
                    segments.push_back(a);
                    segments.push_back(b);
                }
            }
            
            for(int i = 0; i < segments.size(); i++)
            {
                auto point = segments[i];
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
        
        std::vector<glm::vec3> get_row(int row)
        {
            std::vector<glm::vec3> result;
            
            int n = knot_length_v - degree_v - 1 - 1;
            
            int width = n + 1;
            
            // 0 to m, each row has n + 1 elements
            for(int i = 0; i < width; i++)
            {
                result.push_back(control_points[width * row + i]);
            }
            
            return result;
        }
        
        std::vector<glm::vec3> get_column(int column)
        {
            std::vector<glm::vec3> result;
            
            int n = knot_length_v - degree_v - 1 - 1;
            int m = knot_length_u - degree_u - 1 - 1;
            
            int width = n + 1;
            int height = m + 1;
            
            // 0 to m, each row has n + 1 elements
            for(int i = 0; i < height; i++)
            {
                result.push_back(control_points[width * i + column]);
            }
            
            return result;
        }
        
        // min max
        glm::vec2 domain_u;
        glm::vec2 domain_v;
        
        std::vector<std::shared_ptr<BSpline>> grid_row;
        std::vector<std::shared_ptr<BSpline>> grid_column;
        
        std::vector<glm::vec3> segments;
        std::vector<float> vertices;
        
        unsigned int VAO;
        unsigned int VBO;
        
        std::shared_ptr<BSpline> model_u;
        std::shared_ptr<BSpline> model_v;
//        std::vector<std::shared_ptr<BSpline>> segment_v;
    };
} // namespace MH

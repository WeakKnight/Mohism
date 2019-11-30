#pragma once

#include "glm/vec3.hpp"
#include "transformation.h"
#include "glad/glad.h"
#include "core/log.h"
#include "shader.h"
#include <map>
#include <tuple>

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
        
        float get_nodal_value(int i)
        {
            float result = 0.0f;
            for(int index = 1; index <= k(); index++)
            {
                result += knot_vector[i + index];
            }
            float bottom = k();
            result = result / bottom;
            return result;
        }
        
        std::map<std::tuple<int, int, float>, float> blending_cache;
        
    public:
        float blending_func(int i, int _k, float _t)
        {
            auto key = std::make_tuple(i, _k, _t);
            auto it = blending_cache.find(key);
            if(it != blending_cache.end())
            {
                //element found;
                return it->second;
            }
            
            if(_k == 0)
            {
                if(_t == t(j_max + 1))
                {
                    blending_cache[key] = 1;
                    return 1;
                }
                
                if(_t >= t(i) && _t < t(i + 1))
                {
                    blending_cache[key] = 1;
                    return 1;
                }
                else
                {
                    blending_cache[key] = 0;
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
                    
                    blending_cache[key] = left_result + right_result;
                    return left_result + right_result;
                }
                else
                {
                    blending_cache[key] = 0;
                    return 0;
                }
            }
        }
        
        bool show_polygon= false;
        bool show_curve = true;
        bool show_control_point = false;
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
            glGenVertexArrays(1, &NODAL_VAO);
            glGenBuffers(1, &NODAL_VBO);
            glGenVertexArrays(1, &KNOT_VAO);
            glGenBuffers(1, &KNOT_VBO);
        }
        
        ~BSplineSurface()
        {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            glDeleteVertexArrays(1, &NODAL_VAO);
            glDeleteBuffers(1, &NODAL_VBO);
            glDeleteVertexArrays(1, &KNOT_VAO);
            glDeleteBuffers(1, &KNOT_VBO);
        }
        
        // m
        std::vector<float> knot_u;
        // n
        std::vector<float> knot_v;
        // m x n
        std::vector<glm::vec4> control_points;
        std::vector<glm::vec3> grid_points;
        
        std::vector<std::shared_ptr<BSpline>> nodal_curves;
        
        float sourceMatrix[50][50] = {};
        float destMatrix[50][50] = {};
        
        int degree_u;
        int degree_v;
        
        int knot_length_u;
        int knot_length_v;
        
        float get_nodal_blending_u(int i, int nodal_index)
        {
            float ustarVal = u_star(nodal_index);
            if(ustarVal >= domain_u.y)
            {
                ustarVal = domain_u.y - 0.0001f;
            }
            return model_u->blending_func(i, degree_u, ustarVal);
        }
        
        float get_nodal_blending_v(int i, int nodal_index)
        {
            float vstarVal = v_star(nodal_index);
            if(vstarVal >= domain_v.y)
            {
                vstarVal = domain_v.y - 0.0001f;
            }
            return model_v->blending_func(i, degree_v, vstarVal);
        }
        
        void compute_domain()
        {
            left_u = degree_u;
            right_u = knot_length_u - 1 - degree_u;
            left_v = degree_v;
            right_v = knot_length_v - 1 - degree_v;
            
            domain_u = glm::vec2(knot_u[degree_u], knot_u[knot_length_u - 1 - degree_u]);
            domain_v = glm::vec2(knot_v[degree_v], knot_v[knot_length_v - 1 - degree_v]);
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
        
        void compute_derived_data_for_nodal()
        {
            compute_segments();
            compute_knot_segments();
            compute_nodal_segments();
            compute_center();
        }
        
        void compute_derived_date()
        {
            compute_domain();
            compute_model_spline();
            compute_segments();
            compute_knot_segments();
            compute_nodal_segments();
            compute_center();
        }
        
        void draw(Shader* shader)
        {
            if(ForNodal)
            {
                if(nodal_curvedisplay)
                {
                    for(int i = 0; i < nodal_curves.size(); i++)
                    {
                        auto curve = nodal_curves[i];
                        curve->draw(shader);
                    }
                }
                if(general_display)
                {
                    glBindVertexArray(VAO);
                    shader->setVec4("customColor", glm::vec4(0.3f, 0.4f, 0.52f, 1.0f));
                    glDrawArrays(GL_LINES, 0, segments.size());
                    glBindVertexArray(0);
                }
                if(nodal_display)
                {
                    glBindVertexArray(NODAL_VAO);
                    shader->setVec4("customColor", glm::vec4(0.0f, 0.1f, 0.1f, 1.0f));
                    glDrawArrays(GL_LINES, 0, nodal_segments.size());
                    glBindVertexArray(0);
                }
                if(knot_display)
                {
                    glBindVertexArray(KNOT_VAO);
                    shader->setVec4("customColor", glm::vec4(0.3f, 0.7f, 0.52f, 1.0f));
                    glDrawArrays(GL_LINES, 0, knot_segments.size());
                    glBindVertexArray(0);
                }
            }
            else
            {
                if(general_display)
                {
                    glBindVertexArray(VAO);
                    shader->setVec4("customColor", glm::vec4(0.3f, 0.4f, 0.52f, 1.0f));
                    glDrawArrays(GL_LINES, 0, segments.size());
                    glBindVertexArray(0);
                }
                if(nodal_display)
                {
                    glBindVertexArray(NODAL_VAO);
                    shader->setVec4("customColor", glm::vec4(0.0f, 0.1f, 0.1f, 1.0f));
                    glDrawArrays(GL_LINES, 0, nodal_segments.size());
                    glBindVertexArray(0);
                }
                if(knot_display)
                {
                    glBindVertexArray(KNOT_VAO);
                    shader->setVec4("customColor", glm::vec4(0.3f, 0.7f, 0.52f, 1.0f));
                    glDrawArrays(GL_LINES, 0, knot_segments.size());
                    glBindVertexArray(0);
                }
            }
        }
        
        std::map<std::pair<float, float>, glm::vec3> evaluateCache;
        
        glm::vec3 evaluate(float u, float v)
        {
            assert(domain_u.x <= u && u <= domain_u.y);
            assert(domain_v.x <= v && v <= domain_v.y);
            
            if(u == domain_u.y)
            {
                u -= 0.0001f;
            }
            
            if(v == domain_v.y)
            {
                v -= 0.0001f;
            }
            
            auto key = std::make_pair(u, v);
            auto it = evaluateCache.find(key);
            if(it != evaluateCache.end())
            {
                return it->second;
            }
            
            int n = knot_length_v - degree_v - 1 - 1;
            int m = knot_length_u - degree_u - 1 - 1;
            
            glm::vec3 result = glm::vec3(0.0f, 0.0f, 0.0f);
            
            for(int i = 0; i <= m; i++)
            {
                for(int j = 0; j <= n; j++)
                {
                    glm::vec4 control_point = P(i, j);
                    glm::vec3 control_point_3d = glm::vec3(control_point.x, control_point.y, control_point.z);
                    result += (blending(i, j, u, v) * control_point_3d);
                }
            }
            
            evaluateCache[key] = result;
            // return (result / bottom_factor);
            return result;
        }
        
        glm::vec3 center;
        glm::mat4 transform = glm::mat4(1.0f);
        
        bool general_display = true;
        bool nodal_display = false;
        bool knot_display = false;
        bool nodal_curvedisplay = true;
        
        bool ForNodal = false;
        
    private:
        
        float blending(int i, int j, float u, float v)
        {
            int n = knot_length_v - degree_v - 1 - 1;
            int m = knot_length_u - degree_u - 1 - 1;
            float bottom = 0.0f;
            for(int ii = 0; ii <= m; ii++)
            {
                for(int jj = 0; jj <= n; jj++)
                {
                    
                    glm::vec4 control_point_ii_jj = P(ii, jj);
                    float scaler_ii_jj = control_point_ii_jj.w;
                    bottom += (scaler_ii_jj * model_u->blending_func(ii, model_u->get_degree(), u)
                    * model_v->blending_func(jj, model_v->get_degree(), v));
                }
            }
            
            glm::vec4 control_point = P(i, j);
            float scaler_i_j = control_point.w;
            float top = scaler_i_j * model_u->blending_func(i, model_u->get_degree(), u)
            * model_v->blending_func(j, model_v->get_degree(), v);
            
            return top / bottom;
        }
        
        glm::vec4 P(int i, int j)
        {
            int n = knot_length_v - degree_v - 1 - 1;
            int width = n + 1;
            
            return control_points[width * i + j];
        }
        
        void compute_knot_segments()
        {
            for(int i = left_u; i <= right_u; i++)
            {
                float u = knot_u[i];
                for(int j = left_v; j < right_v; j++)
                {
                    float v = knot_v[j];
                    float next_v = knot_v[j + 1];
                    glm::vec3 a = evaluate(u, v);
                    glm::vec3 b = evaluate(u, next_v);
                    knot_segments.push_back(a);
                    knot_segments.push_back(b);
                }
            }
            
            for(int j = left_v; j <= right_v; j++)
            {
                float v = knot_v[j];
                for(int i = left_u; i < right_u; i++)
                {
                    float u = knot_u[i];
                    float next_u = knot_u[i + 1];
                    glm::vec3 a = evaluate(u, v);
                    glm::vec3 b = evaluate(next_u, v);
                    knot_segments.push_back(a);
                    knot_segments.push_back(b);
                }
            }
            
            for(int i = 0; i < knot_segments.size(); i++)
            {
                auto point = knot_segments[i];
                knot_vertices.push_back(point.x);
                knot_vertices.push_back(point.y);
                knot_vertices.push_back(point.z);
            }
            
            glBindVertexArray(KNOT_VAO);
            
            glBindBuffer(GL_ARRAY_BUFFER, KNOT_VBO);
            glBufferData(GL_ARRAY_BUFFER, knot_vertices.size() * sizeof(float), &knot_vertices[0], GL_DYNAMIC_DRAW);
            
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
        }
        
        void compute_nodal_segments()
        {
            int n = knot_length_v - degree_v - 1 - 1;
            int m = knot_length_u - degree_u - 1 - 1;
            
            for(int i = 0; i <= m; i++)
            {
                float u = u_star(i);
                for(int j = 0; j < n; j++)
                {
                    float v = v_star(j);
                    float next_v = v_star(j + 1);
                    glm::vec3 a = evaluate(u, v);
                    glm::vec3 b = evaluate(u, next_v);
                    nodal_segments.push_back(a);
                    nodal_segments.push_back(b);
                }
            }
            
            for(int j = 0; j <= n; j++)
            {
                float v = v_star(j);
                for(int i = 0; i < m; i++)
                {
                    float u = u_star(i);
                    float next_u = u_star(i + 1);
                    glm::vec3 a = evaluate(u, v);
                    glm::vec3 b = evaluate(next_u, v);
                    nodal_segments.push_back(a);
                    nodal_segments.push_back(b);
                }
            }
            
            for(int i = 0; i < nodal_segments.size(); i++)
            {
                auto point = nodal_segments[i];
                nodal_vertices.push_back(point.x);
                nodal_vertices.push_back(point.y);
                nodal_vertices.push_back(point.z);
            }
            
            glBindVertexArray(NODAL_VAO);
            
            glBindBuffer(GL_ARRAY_BUFFER, NODAL_VBO);
            glBufferData(GL_ARRAY_BUFFER, nodal_vertices.size() * sizeof(float), &nodal_vertices[0], GL_DYNAMIC_DRAW);
            
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
        }
        
        void compute_segments(int sub_u = 20, int sub_v = 20)
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
        
        void compute_center()
        {
            int n = knot_length_v - degree_v - 1 - 1;
            int m = knot_length_u - degree_u - 1 - 1;
            
            glm::vec3 result = glm::vec3(0.0f, 0.0f, 0.0f);
            
            for(int i = 0; i <= m; i++)
            {
                for(int j = 0; j <= n; j++)
                {
                    auto control_point = P(i, j);
                    result += glm::vec3(control_point.x, control_point.y, control_point.z);
                }
            }
            
            result = result / (float)(m*n);
            center = result;
        }
        
        float evaluate_u(int i)
        {
            return model_u->get_knot_vector()[i];
        }
        
        float evaluate_v(int j)
        {
            return model_v->get_knot_vector()[j];
        }
        
        float u_star(int i)
        {
            return model_u->get_nodal_value(i);
        }
        
        float v_star(int j)
        {
            return model_v->get_nodal_value(j);
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
        int left_u;
        int right_u;
        int left_v;
        int right_v;
        
        std::vector<glm::vec3> segments;
        std::vector<glm::vec3> nodal_segments;
        std::vector<glm::vec3> knot_segments;
        
        std::vector<float> vertices;
        std::vector<float> nodal_vertices;
        std::vector<float> knot_vertices;
        
        unsigned int VAO;
        unsigned int VBO;
        
        unsigned int NODAL_VAO;
        unsigned int NODAL_VBO;
        
        unsigned int KNOT_VAO;
        unsigned int KNOT_VBO;
        
        std::shared_ptr<BSpline> model_u;
        std::shared_ptr<BSpline> model_v;
//        std::vector<std::shared_ptr<BSpline>> segment_v;
    };
} // namespace MH

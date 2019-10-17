#include "main_layout.h"
#include "shader.h"
#include "camera.h"
#include "GLFW/glfw3.h"
#include "bspline.h"
#include "core/log.h"
#include "serializer.h"
#include "ImGuizmo.h"
#include "imgui.h"

namespace MH
{
//    float vertices[] = {
//        // positions         // colors
//        50.0f, -50.0f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
//        -50.0f, -50.0f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
//        0.0f,  50.0f, 0.0f,  0.0f, 0.0f, 1.0f    // top
//    };
    
//    unsigned int VBO, VAO;
    Camera* camera = nullptr;
    BSpline* bspline = nullptr;
    
    // timing
    float deltaTime = 0.0f;	// time between current frame and last frame
    float lastFrame = 0.0f;
    
    void process_input(GLFWwindow *window)
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera->process_keyboard(UP, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera->process_keyboard(DOWN, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera->process_keyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera->process_keyboard(RIGHT, deltaTime);
    }

    void MainLayout::init()
    {
        group = std::make_shared<CurveGroup>();
        camera = new Camera(window->get_width(), window->get_height());
        
        defaultShader = new Shader("default.vert", "default.frag");
        
        glfwSetScrollCallback((GLFWwindow*)window->get_native_window(), [](GLFWwindow* window, double dx, double dy)
        {
            camera->process_mouse_scroll(dy);
        });
        
        
        auto test = deserialize("2d/test.dat");
        for(int i = 0; i < test.size(); i++)
        {
            group->add_child(test[i]);
        }
        
        glLineWidth(2.0f);
        glEnable(GL_LINE_SMOOTH);
    }
    
    void MainLayout::on_menu_bar()
    {
        
    }
    
    void MainLayout::on_imgui()
    {
        glm::mat4 view = camera->transformation.get_view_matrix();
        glm::mat4 projection = camera->projection;
        auto viewInverse = glm::inverse(view);
        auto projectionInverse = glm::inverse(projection);
        
        auto screen_to_world = [&view, &projection, &viewInverse, &projectionInverse, this](glm::vec2 screen_pos)
        {
            auto screenX = (screen_pos.x - window->get_width() * 0.5f) / (window->get_width() * 0.5f);
            auto screenY = -1.0f * (screen_pos.y - window->get_height() * 0.5f) / (window->get_height() * 0.5f);
            return viewInverse * (projectionInverse *
                                  glm::vec4(
                                            screenX,
                                            screenY,
                                            0.0f, 1.0f));
        };
        
        auto screen_to_world_relative = [&view, &projection, &viewInverse, &projectionInverse, this](glm::vec2 screen_pos)
        {
            auto screenX = (screen_pos.x) / (window->get_width() * 0.5f);
            auto screenY = -1.0f * (screen_pos.y) / (window->get_height() * 0.5f);
            
            return viewInverse * (projectionInverse *
                                  glm::vec4(
                                            screenX,
                                            screenY,
                                            0.0f, 1.0f));
        };
        
        auto world_to_screen = [&view, &projection, this](glm::vec3 world_pos)
        {
            auto screen_space = projection * view * glm::vec4(world_pos, 1.0f);
            return glm::vec2(
                             screen_space.x * (window->get_width() * 0.5f) + window->get_width() * 0.5f,
                             -1.0f * screen_space.y * (window->get_height() * 0.5f) + window->get_height() * 0.5f
            );
        };
        
        ImGuizmo::SetOrthographic(true);
        ImGuizmo::BeginFrame();
        
        ImGuiIO& io = ImGui::GetIO();
        ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        
        auto overlay_drawList = ImGui::GetOverlayDrawList();
        
        float circleRadius = 10.0f;
        float buttonBorder = 8.0f;
        static int selectedIndex = -1;
        static int selectedPointIndex = -1;
        static bool isDragging = false;
        static bool transparent_open = true;
        
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::SetNextWindowPos(glm::vec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(io.DisplaySize);
        
        if (ImGui::Begin("Example: Simple overlay", &transparent_open,  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
        {
            for(int curve_index = 0; curve_index < group->get_child_count(); curve_index++)
            {
                auto child = group->get_child(curve_index);
                auto& control_points = child->get_control_points();
                
                for(size_t point_index = 0; point_index < control_points.size(); point_index++)
                {
                    auto point = control_points[point_index];
                    
//                    ImGuizmo::Manipulate(&view[0][0], &projection[0][0], ImGuizmo::TRANSLATE, ImGuizmo::WORLD, &matrix[0][0]);
                    
                    auto screen_pos = world_to_screen(point);
                    overlay_drawList->AddCircle(screen_pos, circleRadius, IM_COL32(255, 255, 255, 255));
                    
                    ImGui::SetCursorScreenPos(screen_pos
                                              - glm::vec2(0.5f * (circleRadius + buttonBorder), 0.5f * (circleRadius + buttonBorder)));
                    
                    std::string hashId = Format("%d_%d", curve_index, point_index);
                    
                    if(ImGui::InvisibleButton(hashId.c_str(), ImVec2((circleRadius + buttonBorder), (circleRadius + buttonBorder))))
                    {
                        selectedIndex = curve_index;
                        selectedPointIndex = point_index;
                    }
                    
                    if (
                        (ImGui::IsItemActive()
                         ||ImGui::IsItemHovered())
                        && !isDragging
                        )
                    {
                        ImGui::SetTooltip("#%d P%zu (%4.3f, %4.3f)", curve_index, point_index, point.x, point.y);
                    }
                    
                    
                    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0))
                    {
                        if(!isDragging)
                        {
                            isDragging = true;
                        }
                    }
                }
            }
            
            if(isDragging && ImGui::IsMouseReleased(0))
            {
                isDragging = false;
            }
            
            if(selectedIndex != -1 && selectedPointIndex != -1)
            {
                auto child = group->get_child(selectedIndex);
                auto& control_points = child->get_control_points();
                auto& control_point_matrixes = child->get_control_point_matrixes();
                
                auto& matrix = control_point_matrixes[selectedPointIndex];
                ImGuizmo::Manipulate(&view[0][0], &projection[0][0], ImGuizmo::TRANSLATE, ImGuizmo::WORLD, &matrix[0][0]);
                
                auto old_value = control_points[selectedPointIndex];
                control_points[selectedPointIndex] = matrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
                
                auto delta_length = glm::length(old_value - control_points[selectedPointIndex]);
                
                if(delta_length >= 0.000001f)
                {
                    child->mark_need_update();
                }
            }
        }
        ImGui::End();
        
        on_menu_bar();
        
//        if(ImGui::IsMouseDragging(2) ||
//           (ImGui::IsMouseDragging(0) && ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Space)))
//           )
//        {
//            auto delta = (io.MouseDelta);
//            camera->transformation.translate(-screen_to_world_relative(delta));
//        }
    }
    
    void MainLayout::update()
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        process_input((GLFWwindow *)window->get_native_window());

        glm::mat4 view = camera->transformation.get_view_matrix();
        glm::mat4 projection = camera->projection;
        
        // rendering
        {
            glm::mat4 model = glm::mat4(1.0f);
            // render the triangle
            defaultShader->use();

            defaultShader->setVec4("customColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            defaultShader->setMat4("projection", projection);
            defaultShader->setMat4("view", view);
            defaultShader->setMat4("model", model);// in this editor, model always is identity

    //        glBindVertexArray(VAO);
    //        glDrawArrays(GL_TRIANGLES, 0, 3);
            for(int i = 0; i < group->get_child_count(); i++)
            {
                auto child = group->get_child(i);
                child->draw(defaultShader);
            }
//            bspline->draw();
        }
    }
}
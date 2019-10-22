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
        
        glLineWidth(2.0f);
        glEnable(GL_LINE_SMOOTH);
    }
    
    void MainLayout::on_menu_bar()
    {
        static int action = -1;
        
        ImGui::BeginMainMenuBar();
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open"))
            {
                action = 1;
            }
            else if (ImGui::MenuItem("Save"))
            {
               
            }
            else if (ImGui::MenuItem("Save As..."))
            {
                action = 2;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
        
        static char pathBuf[256];
        
        if(action == 1)
        {
            ImGui::OpenPopup("Select File");
        }
        
        if (ImGui::BeginPopupModal("Select File"))
        {
            ImGui::InputText("File Path", pathBuf, 256);
            
            static int READING_TYPE_OPTION = OPEN_KNOT_MODIFIED_UNIFORM_VECTOR;
            ImGui::Text("What To Do When Not Having Predefined Knot Point?");
            ImGui::RadioButton("Read As Open Modified", &READING_TYPE_OPTION, OPEN_KNOT_MODIFIED_UNIFORM_VECTOR); ImGui::SameLine();
            ImGui::RadioButton("Read As Floating", &READING_TYPE_OPTION, FLOATING_UNIFORM_VECTOR); ImGui::SameLine();
            
            ImGui::NewLine();
            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                group->clear();
                auto curves = deserialize(pathBuf);
                for(int i = 0; i < curves.size(); i++)
                {
                    auto curve = curves[i];
                    curve->process_knot_vector_by_degree_and_control_points(READING_TYPE_OPTION);
                    group->add_child(curves[i]);
                }
                
                ImGui::CloseCurrentPopup();
                action = -1;
            }
            
            ImGui::SameLine();
            
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
                action = -1;
            }
            ImGui::EndPopup();
        }
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
        
//        auto screen_to_world_relative = [&view, &projection, &viewInverse, &projectionInverse, this](glm::vec2 screen_pos)
//        {
//            auto screenX = (screen_pos.x) / (window->get_width() * 0.5f);
//            auto screenY = -1.0f * (screen_pos.y) / (window->get_height() * 0.5f);
//            
//            return viewInverse * (projectionInverse *
//                                  glm::vec4(
//                                            screenX,
//                                            screenY,
//                                            0.0f, 1.0f));
//        };
        
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
        ImGui::SetNextWindowPos(glm::vec2(0.0f, 18.0f));
        ImGui::SetNextWindowSize(io.DisplaySize);
        
        if (ImGui::Begin("Example: Simple overlay", &transparent_open,  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings
                         |
                         ImGuiWindowFlags_NoFocusOnAppearing |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoNav
                         ))
        {
            for(int curve_index = 0; curve_index < group->get_child_count(); curve_index++)
            {
                auto child = group->get_child(curve_index);
                auto& control_points = child->get_control_points();
                
                for(size_t point_index = 0; point_index < control_points.size(); point_index++)
                {
                    auto point = control_points[point_index];
                    
                    auto screen_pos = world_to_screen(point);
                    overlay_drawList->AddCircle(screen_pos, circleRadius, IM_COL32(255, 255, 255, 255));
                    
                    ImGui::SetCursorScreenPos(screen_pos
                                              - glm::vec2(0.5f * (circleRadius + buttonBorder), 0.5f * (circleRadius + buttonBorder)));
                    
                    std::string hashId = Format("%d_%d", curve_index, point_index);
                    
                    if(ImGui::InvisibleButton(hashId.c_str(), ImVec2((circleRadius + buttonBorder), (circleRadius + buttonBorder))))
                    {
                        if(ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_X)))
                        {
                            child->remove_control_point(point_index);
                        }
                        else
                        {
                            selectedIndex = curve_index;
                            selectedPointIndex = point_index;
                        }
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
        
        auto pos = ImGui::GetIO().MousePos;
        if(ImGui::IsMouseClicked(0) && ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Z)))
        {
            glm::vec3 worldPos = screen_to_world(pos);
            worldPos = glm::vec3(worldPos.x, worldPos.y, 0.0f);
            
            float minimum_distance = INFINITY;
            int minimum_curve_index = -1;
            int minimum_edge_index = -1;
            // -1 = left, 0 = inside, 1 = right
            int minimum_side = 0;
            
            for(int curve_index = 0; curve_index < group->get_child_count(); curve_index++)
            {
                auto curve = group->get_child(curve_index);
                auto& control_points = curve->get_control_points();
                for(int point_index = 0; point_index < control_points.size() - 1; point_index++)
                {
                    auto point_a = control_points[point_index];
                    auto point_b = control_points[point_index + 1];
                    auto point_c = worldPos;
                    
                    // calculate c to segment ab
                    auto ac = point_c - point_a;
                    auto ab = point_b - point_a;
                    
                    auto bc = point_c - point_b;
                    auto ba = -ab;
                    
                    float distance = 0.0f;
                    int side = 0;
                    
                    if(glm::dot(ac, ab) < 0.0f)
                    {
                        distance = glm::length(ac);
                        side = -1;
                    }
                    else if(glm::dot(bc, ba) < 0.0f)
                    {
                        distance = glm::length(bc);
                        side = 1;
                    }
                    else
                    {
                        auto perp = glm::dot(glm::normalize(ba), bc);
                        distance = sqrt(glm::dot(bc, bc) - perp * perp);
                        side = 0;
                    }
                    
                    if(minimum_distance > distance)
                    {
                        minimum_distance = distance;
                        minimum_curve_index = curve_index;
                        minimum_edge_index = point_index;
                        minimum_side = side;
                    }
                }
            }

            if(minimum_curve_index != -1)
            {
                auto target_curve = group->get_child(minimum_curve_index);
                
                if(minimum_side == 0)
                {
                    target_curve->insert_control_point(minimum_edge_index + 1, worldPos);
                }
                else if(minimum_side == 1)
                {
                    target_curve->insert_control_point(minimum_edge_index + 2, worldPos);
                }
                else if(minimum_side == -1)
                {
                    target_curve->insert_control_point(minimum_edge_index + 0, worldPos);
                }
            }
        }
        
        on_menu_bar();
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

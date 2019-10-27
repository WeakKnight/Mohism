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
    bool firstMouse = true;
    float lastX =  800.0f / 2.0;
    float lastY =  600.0 / 2.0;
    
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
    
    void mouse_dragging(double xoffset, double yoffset)
    {
//        xoffset = -xoffset;
        yoffset = - yoffset;
        
        if(camera == nullptr || camera->is_ortho)
        {
            return;
        }
        
        float sensitivity = 0.1f; // change this value to your liking
        xoffset *= sensitivity;
        yoffset *= sensitivity;
       
        camera->yaw += xoffset;
        camera->pitch += yoffset;
        
        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (camera->pitch > 89.0f)
            camera->pitch = 89.0f;
        if (camera->pitch < -89.0f)
            camera->pitch = -89.0f;
        
        glm::vec3 front;
        front.x = cos(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));
        front.y = sin(glm::radians(camera->pitch));
        front.z = sin(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));
        camera->camera_front = glm::normalize(front);
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
    
    void MainLayout::on_inspector()
    {
        static ImGuiWindowFlags toolboxFlag =  ImGuiWindowFlags_None;
        static bool* toolboxOpen = nullptr;
        
        static int camera_option = 1;
        
        ImGui::Begin("Inspector", toolboxOpen, toolboxFlag);
        {
            if(ImGui::RadioButton("Ortho", &camera_option, 1))
            {
                camera->set_ortho(true);
            }
            ImGui::SameLine();
            if(ImGui::RadioButton("Perspective", &camera_option, 0))
            {
                camera->set_ortho(false);
            }
            
            ImGui::BeginChild("left pane", ImVec2(170, 0), true);
            
            static char label[128];
            
            for(int curve_index = 0; curve_index < group->get_child_count(); curve_index++)
            {
                auto curve = group->get_child(curve_index);
                sprintf(label, "Degree %d Spline #%d", curve->get_degree(), curve_index);
                if (ImGui::Selectable(label, selectedIndex == curve_index))
                {
                    selectedIndex = curve_index;
                }
            }
            ImGui::EndChild();
            ImGui::SameLine();
            ImGui::BeginChild("mid pane", ImVec2(210, 0), true);
            if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
            {
                if (ImGui::BeginTabItem("Control Point"))
                {
                    if(group->get_child_count() > 0 && selectedIndex != -1)
                    {
                        auto curve = group->get_child(selectedIndex);
                        auto& control_points = curve->get_control_points();
                        
                        for(size_t i = 0; i < control_points.size(); i++)
                        {
                            if(ImGui::InputFloat3(Format("#%d", (int)i).c_str(), &(control_points[i][0])))
                            {
                                curve->mark_need_update();
                            }
                            ImGui::SameLine();
                            ImGui::PushID(i);
                            if(ImGui::Button("X"))
                            {
                                curve->remove_control_point(i);
                            }
                            ImGui::PopID();
                        }
                    }
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Knot Vector"))
                {
                    if(group->get_child_count() > 0 && selectedIndex != -1)
                    {
                        auto curve = group->get_child(selectedIndex);
                        auto& knot_vector = curve->get_knot_vector();
                        
                        if(ImGui::Button("OpenModified"))
                        {
                            curve->generate_modified_open_knot_uniform_vector();
                            curve->mark_need_update();
                        }
                        
                        if(ImGui::Button("Floating"))
                        {
                            curve->generate_float_uniform_knot_vector();
                            curve->mark_need_update();
                        }
                        
                        for(size_t i = 0; i < knot_vector.size(); i++)
                        {
                            if(ImGui::InputFloat(Format("#%d", (int)i).c_str(), &(knot_vector[i])))
                            {
                                curve->mark_need_update();
                            }
                            ImGui::SameLine();
                            ImGui::PushID(i);
                            if(ImGui::Button("X"))
                            {
                                curve->remove_knot_vector(i);
                            }
                            ImGui::PopID();
                        }
                    }
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Misc"))
                {
                    static bool all_show_curve = true;
                    static bool all_show_control_point = true;
                    static bool all_show_polygon = true;
                    
                    if(ImGui::Button("All Curve Display"))
                    {
                        all_show_curve = !all_show_curve;
                        for(int index = 0; index < group->get_child_count(); index++)
                        {
                            auto curve = group->get_child(index);
                            curve->show_curve = all_show_curve;
                        }
                    }
                    if(ImGui::Button("All Point Display"))
                    {
                        all_show_control_point = !all_show_control_point;
                        for(int index = 0; index < group->get_child_count(); index++)
                        {
                            auto curve = group->get_child(index);
                            curve->show_control_point = all_show_control_point;
                        }
                    }
                    if(ImGui::Button("All Polygon Display"))
                    {
                        all_show_polygon = !all_show_polygon;
                        for(int index = 0; index < group->get_child_count(); index++)
                        {
                            auto curve = group->get_child(index);
                            curve->show_polygon = all_show_polygon;
                        }
                    }
                    
                    if(group->get_child_count() > 0 && selectedIndex != -1)
                    {
                        auto curve = group->get_child(selectedIndex);
                        
                        if(ImGui::Button("Delete"))
                        {
                            group->remove_bspline(selectedIndex);
                            selectedIndex = -1;
                        }
                        
                        ImGui::Checkbox("Curve Display", &curve->show_curve);
                        ImGui::Checkbox("Point Display", &curve->show_control_point);
                        ImGui::Checkbox("Polygon Display", &curve->show_polygon);
                        
                        ImGui::Checkbox("Special Color", &curve->is_special_color);
                        
                        if(ImGui::Button("degree raise"))
                        {
                            curve->set_degree(curve->get_degree() + 1);
                            curve->mark_need_update();
                        }
                        if(ImGui::Button("degree decline"))
                        {
                            curve->set_degree(curve->get_degree() - 1);
                            curve->mark_need_update();
                        }
                        
                        static bool is_2d = true;
                        if(ImGui::Checkbox("2D", &is_2d))
                        {
                            curve->set_dimension(is_2d?2:3);
                        }
                        
                        if(ImGui::Button("Duplicate"))
                        {
                            auto newCurve = group->add_empty_bspline();
                            newCurve->set_degree(curve->get_degree());
                            newCurve->set_dimension(curve->get_dimension());
                            newCurve->add_knot_vector(curve->get_knot_vector());
                            newCurve->add_control_points(curve->get_control_points());
                            newCurve->is_special_color = curve->is_special_color;
                        }
                        
                        static float whole_curve_move = 0.0f;
                        ImGui::InputFloat("Move Delta", &whole_curve_move);
                        if(ImGui::Button("Right"))
                        {
                            auto& controlPoints = curve->get_control_points();
                            auto& control_point_matrixes = curve->get_control_point_matrixes();
                            
                            for(int i = 0; i < controlPoints.size(); i++)
                            {
                                controlPoints[i] += (glm::vec3(1.0f, 0.0f, 0.0f) * whole_curve_move);
                                control_point_matrixes[i] = glm::translate(glm::mat4(1.0f), controlPoints[i]);
                            }
                            curve->mark_need_update();
                        }
                        if(ImGui::Button("Top"))
                        {
                            auto& controlPoints = curve->get_control_points();
                            auto& control_point_matrixes = curve->get_control_point_matrixes();
                            
                            for(int i = 0; i < controlPoints.size(); i++)
                            {
                                controlPoints[i] += (glm::vec3(0.0f, 1.0f, 0.0f) * whole_curve_move);
                                control_point_matrixes[i] = glm::translate(glm::mat4(1.0f), controlPoints[i]);
                            }
                            curve->mark_need_update();
                        }
                    }
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }
            ImGui::EndChild();
        }
        ImGui::End();
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
                if(current_path.size() != 0)
                {
                    auto content = serialize(*group);
                    WriteFile(current_path, content);
                }
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
                current_path = pathBuf;
                
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
        
        if(action == 2)
        {
            ImGui::OpenPopup("Save As");
        }
        
        if (ImGui::BeginPopupModal("Save As"))
        {
            ImGui::InputText("File Path", pathBuf, 256);
            
            ImGui::NewLine();
            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                current_path = pathBuf;
               
                auto content = serialize(*group);
                WriteFile(current_path, content);
               
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
        glm::mat4 view = camera->transformation.look_at(camera->camera_front);
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
            auto clipSpacePos = projection * (view * glm::vec4(world_pos, 1.0f));
            auto ndcSpacePos = glm::vec3(clipSpacePos.x, clipSpacePos.y, clipSpacePos.z) / clipSpacePos.w;
            glm::vec2 windowSpacePos = ((glm::vec2(ndcSpacePos.x, -ndcSpacePos.y) + 1.0f) / 2.0f) * glm::vec2(window->get_width(), window->get_height());
            return windowSpacePos;
        };
        
        ImGuizmo::SetOrthographic(camera->is_ortho);
        ImGuizmo::BeginFrame();
        
        ImGuiIO& io = ImGui::GetIO();
        ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        
        auto overlay_drawList = ImGui::GetBackgroundDrawList();
        
        float circleRadius = 10.0f;
        float buttonBorder = 8.0f;
        
        static bool isDragging = false;
        static bool transparent_open = true;
        
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::SetNextWindowPos(glm::vec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(io.DisplaySize);
        
        if (ImGui::Begin("Example: Simple overlay", &transparent_open,  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings
                         |
                         ImGuiWindowFlags_NoFocusOnAppearing |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoNav
                         | ImGuiWindowFlags_NoBringToFrontOnFocus
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
                    if(child->show_control_point)
                    {
                        overlay_drawList->AddCircle(screen_pos, circleRadius, IM_COL32(255, 255, 255, 255));
                    }
                    
                    ImGui::SetCursorScreenPos(screen_pos
                                              - glm::vec2(0.5f * (circleRadius + buttonBorder), 0.5f * (circleRadius + buttonBorder)));
                    
                    std::string hashId = Format("%d_%d", curve_index, point_index);
                    
                    bool button_pressed = false;
                    
                    if(ImGui::InvisibleButton(hashId.c_str(), ImVec2((circleRadius + buttonBorder), (circleRadius + buttonBorder))))
                    {
                        button_pressed = true;
                        
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
                    
                    if(ImGui::IsMouseClicked(1) && !button_pressed)
                    {
                        selectedIndex = -1;
                        selectedPointIndex = -1;
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
        
        if(ImGui::IsMouseDragging(1))
        {
            auto delta = (io.MouseDelta);
            mouse_dragging(delta.x, delta.y);
        }
        
        
        auto pos = ImGui::GetIO().MousePos;
        if(ImGui::IsMouseClicked(0) && ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_C)))
        {
            glm::vec3 worldPos = screen_to_world(pos);
            worldPos = glm::vec3(worldPos.x, worldPos.y, 0.0f);
            auto curve = std::make_shared<BSpline>();
            curve->set_degree(2);
            curve->add_control_point(worldPos);
            group->add_child(curve);
        }
        
        if(ImGui::IsMouseClicked(0) && ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Z)))
        {
            glm::vec3 worldPos = screen_to_world(pos);
            worldPos = glm::vec3(worldPos.x, worldPos.y, 0.0f);
            
            float minimum_distance = INFINITY;
            int minimum_curve_index = -1;
            int minimum_edge_index = -1;
            // -1 = left, 0 = inside, 1 = right
            int minimum_side = 0;
            
            //for(int curve_index = 0; curve_index < group->get_child_count(); curve_index++)
            if(selectedIndex != -1)
            {
                auto curve_index = selectedIndex;
                auto curve = group->get_child(curve_index);
                auto& control_points = curve->get_control_points();
                
                if(control_points.size() == 1)
                {
                    float distance = glm::length(control_points[0] - worldPos);
                    if(distance < minimum_distance)
                    {
                        minimum_distance = distance;
                        minimum_curve_index = curve_index;
                        minimum_edge_index = 0;
                        minimum_side = 0;
                    }
                }
                
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
        
        on_inspector();
        on_menu_bar();
    }
    
    void MainLayout::update()
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        process_input((GLFWwindow *)window->get_native_window());

        glm::mat4 view = camera->transformation.look_at(camera->camera_front);
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

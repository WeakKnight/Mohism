#include "main_layout.h"
#include "shader.h"
#include "camera.h"
#include "GLFW/glfw3.h"
#include "bspline.h"
#include "core/log.h"
#include "serializer.h"

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
        
        
        auto test = deserialize("2d/curve2a.dat");
        for(int i = 0; i < test.size(); i++)
        {
            group->add_child(test[i]);
        }
    }
    
    void MainLayout::update()
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        process_input((GLFWwindow *)window->get_native_window());

        glm::mat4 view = camera->transformation.get_view_matrix();
        glm::mat4 projection = camera->projection;
        
        auto screen_to_world = [&view, &projection, this](glm::vec2 screen_pos)
        {
            auto viewInverse = glm::inverse(view);
            auto projectionInverse = glm::inverse(projection);
            
            auto screenX = (screen_pos.x - window->get_width() * 0.5f) / (window->get_width() * 0.5f);
            auto screenY = -1.0f * (screen_pos.y - window->get_height() * 0.5f) / (window->get_height() * 0.5f);
            return viewInverse * (projectionInverse *
                           glm::vec4(
                                     screenX,
                                     screenY,
                                     0.0f, 1.0f));
        };
        
        // logic
        {
            auto pos = ImGui::GetIO().MousePos;
            
//            LOG_INFO("x:{0} y:{1}", pos.x, pos.y);
            
            if(ImGui::IsMouseClicked(0))
            {
                auto worldPos = screen_to_world(pos);
                bspline->add_control_point(glm::vec3(worldPos.x, worldPos.y, 0.0f));
            }
        }
        
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
                child->draw();
            }
//            bspline->draw();
        }
    }
}

#include "mac_window.h"
#include "events/application_event.h"
#include "events/key_event.h"
#include "events/mouse_event.h"
#include "core/log.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "imgui.h"

#define IMGUI_IMPL_OPENGL_LOADER_GLAD

#include "examples/imgui_impl_glfw.cpp"
#include "examples/imgui_impl_opengl3.cpp"

#include "editor/main_layout.h"

namespace MH
{
    static bool s_glfw_initialized = false;
    static void glfw_error_callback(int error, const char* description)
    {
        LOG_ERROR("{0} {1}", error, description);
    }

    Window* Window::create(const WindowProps& props)
    {
        return new MacWindow(props);
    }

    MacWindow::MacWindow(const WindowProps& props)
    {
        init(props);
    }

    MacWindow::~MacWindow()
    {
        shutdown();
    }

    MainLayout main_layout;
    
    void MacWindow::init(const WindowProps& props)
    {
        m_data.title = props.title;
        m_data.width = props.width;
        m_data.height = props.height;

        if(!s_glfw_initialized)
        {
            int result = glfwInit();
            assert(result);

            glfwSetErrorCallback(glfw_error_callback);
            s_glfw_initialized = true;
        }

        const char* glsl_version = "#version 150";

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        m_window = glfwCreateWindow((int)props.width, (int)props.height, m_data.title.c_str(), nullptr, nullptr);
        glfwMakeContextCurrent(m_window);

        bool err = gladLoadGL() == 0;
        
        if (err)
        {
            spdlog::debug("Failed to initialize OpenGL loader!");
            return;
        }
//        int glad_init_result = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
//        assert(glad_init_result);

        glfwSetWindowUserPointer(m_window, &m_data);
        
        set_vsync(true);

        glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            
            data.width = width;
            data.height = height;

            WindowResizeEvent event(width, height);
            data.event_callback(event);
        });

        glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            WindowCloseEvent event;
            data.event_callback(event);
        });

        glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action ,int mods)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch(action)
            {
                case GLFW_PRESS:
                {
                    KeyPressedEvent event(key, 0);
                    data.event_callback(event);
                }
                break;
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent event(key);
                    data.event_callback(event);
                }
                break;
                case GLFW_REPEAT:
                {
                    KeyPressedEvent event(key, 1);
                    data.event_callback(event);
                }
                break;
                default:
                break;
            }
        });

        glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            switch(action)
            {
                case GLFW_PRESS:
                {
                    MouseButtonPressedEvent event(button);
                    data.event_callback(event);
                }
                break;
                case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent event(button);
                    data.event_callback(event);
                }
                break;
                default:
                break;
            }
        });

        glfwSetScrollCallback(m_window, [](GLFWwindow* window, double offset_x, double offset_y)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            MouseScrolledEvent event(offset_x, offset_y);
            data.event_callback(event);
        });

        glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double x, double y)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            MouseMovedEvent event(x, y);
            data.event_callback(event);
        });
        
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        
        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();
        
        // Setup Platform/Renderer bindings
        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);
        
        main_layout.set_window(this);
        main_layout.init();
    }

    void MacWindow::shutdown()
    {
        glfwDestroyWindow(m_window);
    }

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    
    void MacWindow::on_update()
    {
        glfwPollEvents();
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // Rendering
        ImGui::Render();
        
        int display_w, display_h;
        glfwGetFramebufferSize(m_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        
        main_layout.update();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(m_window);
    }

    void MacWindow::set_vsync(bool enabled)
    {
        if(enabled)
        {
            glfwSwapInterval(1);
        }
        else
        {
            glfwSwapInterval(0);
        }

        m_data.vsync = enabled;
    }

    bool MacWindow::is_vsync() const
    {
        return m_data.vsync;
    }
} // namespace MH

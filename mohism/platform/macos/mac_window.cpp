#include "mac_window.h"
#include "events/application_event.h"
#include "events/key_event.h"
#include "events/mouse_event.h"
#include "core/log.h"

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

        m_window = glfwCreateWindow((int)props.width, (int)props.height, m_data.title.c_str(), nullptr, nullptr);
        glfwMakeContextCurrent(m_window);
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
                KeyPressedEvent event(key, 0);
                data.event_callback(event);
                break;
                case GLFW_RELEASE:
                KeyReleasedEvent event(key);
                data.event_callback(event);
                break;
                case GLFW_REPEAT:
                KeyPressedEvent event(key, 1);
                data.event_callback(event);
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
                MouseButtonPressedEvent event(button);
                data.event_callback(event);
                break;
                case GLFW_RELEASE:
                MouseButtonPressedEvent event(button);
                data.event_callback(event);
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
    }

    void MacWindow::shutdown()
    {
        glfwDestroyWindow(m_window);
    }

    void MacWindow::on_update()
    {
        glfwPollEvents();
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

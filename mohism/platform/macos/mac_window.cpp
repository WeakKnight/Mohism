#include "mac_window.h"

namespace MH
{
    static bool s_glfw_initialized = false;

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

            s_glfw_initialized = true;
        }

        m_window = glfwCreateWindow((int)props.width, (int)props.height, m_data.title.c_str(), nullptr, nullptr);
        glfwMakeContextCurrent(m_window);
        glfwSetWindowUserPointer(m_window, &m_data);
        
        set_vsync(true);
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

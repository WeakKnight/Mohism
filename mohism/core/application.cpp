#include "application.h"
namespace MH
{
Application::Application()
{
    m_window = std::unique_ptr<Window>(Window::create());
}

Application::~Application()
{
}
void Application::run()
{
    while(is_running)
    {
        m_window->on_update();
    }
}
} // namespace mh

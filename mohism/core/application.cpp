#include "log.h"
#include "application.h"

namespace MH
{
Application::Application()
{
    m_window = std::unique_ptr<Window>(Window::create());
    m_window->set_event_callback(std::bind(&Application::on_event, this, std::placeholders::_1));
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

void Application::on_event(Event& e)
{
    CORE_LOG_INFO("{0}", e.to_string());

    EventDispatcher dispatcher(e);
    dispatcher.dispatch<WindowCloseEvent>(std::bind(&Application::on_window_close, this, std::placeholders::_1));
}

bool Application::on_window_close(WindowCloseEvent& e)
{
    is_running = false;
    return true;
}

} // namespace mh

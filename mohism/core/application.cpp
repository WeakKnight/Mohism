#include "application.h"
#include "log.h"
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
    CORE_LOG_INFO("{0}", e);
}

} // namespace mh

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
        for (Layer* layer : m_layer_stack)
        {
		    layer->on_update();
        }

        m_window->on_update();
    }
}

void Application::on_event(Event& e)
{
    // CORE_LOG_INFO("{0}", e.to_string());

    EventDispatcher dispatcher(e);
    dispatcher.dispatch<WindowCloseEvent>(std::bind(&Application::on_window_close, this, std::placeholders::_1));

//    for(auto it = m_layer_stack.end() - 1; it != m_layer_stack.begin(); it--)
//    {
//        (*it)->on_event(e);
//        if(e.handled)
//        {
//            break;
//        }
//    }
}

bool Application::on_window_close(WindowCloseEvent& e)
{
    is_running = false;
    return true;
}

void Application::push_layer(Layer* layer)
{
    m_layer_stack.push_layer(layer);
}

void Application::push_overlay(Layer* layer)
{
    m_layer_stack.push_overlay(layer);
}

} // namespace mh

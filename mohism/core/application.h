#pragma once

#include "core.h"
#include "window.h"
#include "events/application_event.h"
#include "layer_stack.h"

namespace MH
{
class MH_API Application
{
public:
    Application();
    virtual ~Application();
    void run();

    void on_event(Event& e);

    void push_layer(Layer* layer);
    void push_overlay(Layer* layer);

private:
    bool on_window_close(WindowCloseEvent& e);
    std::unique_ptr<Window> m_window;
    bool is_running = true;
    LayerStack m_layer_stack;
};

Application* create_application();

} // namespace mh

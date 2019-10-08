#pragma once

#include "core.h"
#include "window.h"
#include "events/application_event.h"

namespace MH
{
class MH_API Application
{
public:
    Application();
    virtual ~Application();
    void run();

    void on_event(Event& e);

private:
    bool on_window_close(WindowCloseEvent& e);
    std::unique_ptr<Window> m_window;
    bool is_running = true;
};

Application* create_application();

} // namespace mh

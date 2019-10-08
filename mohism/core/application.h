#pragma once

#include "core.h"
#include "window.h"

namespace MH
{
class MH_API Application
{
public:
    Application();
    virtual ~Application();
    void run();

private:
    std::unique_ptr<Window> m_window;
    bool is_running = true;
};

Application* create_application();

} // namespace mh

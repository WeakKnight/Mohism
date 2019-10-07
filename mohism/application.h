#pragma once

#include "core.h"

namespace MH
{
class MH_API Application
{
public:
    Application();
    virtual ~Application();
    void run();

private:
};

Application* create_application();

} // namespace mh

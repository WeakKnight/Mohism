#pragma once

#include "imgui.h"
#include "glad/glad.h"
#include "core/window.h"

namespace MH
{
    class Shader;
    
    class MainLayout
    {
    public:
        void init();
        void update();
        void set_window(Window* window)
        {
            this->window = window;
        }
    private:
        Shader* defaultShader;
        Window* window;
    };
} // namespace MH

#pragma once

#include "imgui.h"
#include "glad/glad.h"
#include "core/window.h"

namespace MH
{
    class Shader;
    class CurveGroup;
    
    class MainLayout
    {
    public:
        void init();
        void update();
        void on_imgui();
        void set_window(Window* window)
        {
            this->window = window;
        }
    private:
        void on_menu_bar();
        void on_inspector();
        
        int selectedIndex = -1;
        int selectedPointIndex = -1;
        
        int surfaceSelectedIndex = -1;
        
        std::string current_path = "";
        
        Shader* defaultShader;
        Window* window;
        std::shared_ptr<CurveGroup> group;
    };
} // namespace MH

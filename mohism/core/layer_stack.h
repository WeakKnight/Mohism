#pragma once

#include "core/layer.h"
#include "core/core.h"

namespace MH
{
    class MH_API LayerStack
    {
        public:
            LayerStack();
            ~LayerStack();

            void push_layer(Layer* layer);
            void push_overlay(Layer* overlay);
            void pop_layer(Layer* layer);
            void pop_overlay(Layer* overlay);

            std::vector<Layer*>::iterator begin() {return m_layers.begin();}
            std::vector<Layer*>::iterator end() {return m_layers.end();}
        private:
            std::vector<Layer*> m_layers;
            unsigned int m_layer_insert_index;
    };
} // namespace MH

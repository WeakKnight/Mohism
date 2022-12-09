#pragma once

#include "core/core.h"
#include "events/event.h"

namespace MH
{
    class MH_API Layer
    {
        public:
            Layer(const std::string& name = "Layer");
            virtual ~Layer();

            virtual void on_attach() {}
            virtual void on_detach() {}
            virtual void on_update() {}
            virtual void on_event(Event& event) {}

            inline const std::string& get_name() const 
            {
                return m_name;
            }

        protected:
            std::string m_name;
    };
} // namespace MH

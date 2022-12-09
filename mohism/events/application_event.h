#pragma ONCE

#include "event.h"

namespace MH
{
    class MH_API WindowResizeEvent:public Event
    {
        public:
            WindowResizeEvent(unsigned int width, unsigned int height)
            :m_width(width), m_height(height)
            {
            }

            inline unsigned int get_width() const { return m_width; }
            inline unsigned int get_height() const { return m_height; }

            std::string to_string() const override
            {
                std::stringstream ss;
                ss << "WindowResizeEvent: " << m_width << ", " << m_height;
                return ss.str();
            }

            EVENT_CLASS_TYPE(WindowResize)
            EVENT_CLASS_CATEGORY(EventCategoryApplication)

        private:
            unsigned int m_width, m_height;
    };

    class MH_API WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() {}

		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

    class MH_API TickEvent : public Event
	{
	public:
		TickEvent() {}

		EVENT_CLASS_TYPE(Tick)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

    class MH_API UpdateEvent : public Event
	{
	public:
		UpdateEvent() {}

		EVENT_CLASS_TYPE(Update)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};

    class MH_API RenderEvent : public Event
	{
	public:
		RenderEvent() {}

		EVENT_CLASS_TYPE(Render)
		EVENT_CLASS_CATEGORY(EventCategoryApplication)
	};
} // namespace MH

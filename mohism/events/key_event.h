#pragma once

#include "event.h"

namespace MH
{
    class MH_API KeyEvent : public Event
	{
	public:
		inline int get_key_code() const { return m_keycode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
		KeyEvent(int keycode)
			: m_keycode(keycode) {}

		int m_keycode;
	};

    class MH_API KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(int keycode, int repeat_count)
			: KeyEvent(keycode), m_repeatCount(repeat_count) {}

		inline int get_repeat_count() const { return m_repeatCount; }

		std::string to_string() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_keycode << " (" << m_repeatCount << " repeats)";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		int m_repeatCount;
	};

    class MH_API KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(int keycode)
			: KeyEvent(keycode) {}

		std::string to_string() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_keycode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};

    class MH_API KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(int keycode)
			: KeyEvent(keycode) {}

		std::string to_string() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << m_keycode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)
	};
} // namespace MH

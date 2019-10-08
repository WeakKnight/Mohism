#include "pch.h"
#include "core/core.h"
#include "core/window.h"

#include "GLFW/glfw3.h"

namespace MH
{
    	class MacWindow : public Window
	{
	public:
		MacWindow(const WindowProps& props);
		virtual ~MacWindow();

		void on_update() override;

		inline unsigned int get_width() const override { return m_data.width; }
		inline unsigned int get_height() const override { return m_data.height; }

		// Window attributes
		inline void set_event_callback(const EventCallbackFn& callback) override { m_data.event_callback = callback; }
		void set_vsync(bool enabled) override;
		bool is_vsync() const override;

		inline virtual void* get_native_window() const { return m_window; }
	private:
		virtual void init(const WindowProps& props);
		virtual void shutdown();

	private:
		GLFWwindow* m_window;

		struct WindowData
		{
			std::string title;
			unsigned int width, height;
			bool vsync;

			EventCallbackFn event_callback;
		};

		WindowData m_data;
	};

} // namespace MH

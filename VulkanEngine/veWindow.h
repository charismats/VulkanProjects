#pragma once
#include "include.h"

namespace CharismaVulkan {
	class VeWindow {
	public:
		VeWindow(const int w, const int h, const char* title);
		~VeWindow();
		GLFWwindow* getWindow() const;
	private:
		GLFWwindow* m_window;
	};
}
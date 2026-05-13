#include "veWindow.h"

namespace CharismaVulkan {
	VeWindow::VeWindow(const int w, const int h, const char* title) {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_window = glfwCreateWindow(w, h, title, nullptr, nullptr);
	}

	VeWindow::~VeWindow() {
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}

	GLFWwindow* VeWindow::getWindow() const {
		return m_window;
	}


}
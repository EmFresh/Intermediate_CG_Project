#include "WindowCreator.h"

WindowInfo* m_info = new WindowInfo;
void(*WindowCreator::m_onWindowResizeCallback)(GLFWwindow*, int, int);
GLFWwindow* m_window;
GLFWmonitor* m_monitor;
bool m_full;

WindowCreator::WindowCreator()
{
	m_monitor = glfwGetPrimaryMonitor();
}

WindowCreator::WindowCreator(std::string name, Coord2D<int> size, Coord2D<int> position, int monitor, bool fullScreeen, bool visable)
{
	createWindow(name, size, position, monitor, fullScreeen, visable);
}


WindowCreator::~WindowCreator()
{}

int WindowCreator::createWindow(std::string name, Coord2D<int> size, Coord2D<int> position, int monitor, bool fullScreeen, bool visable)
{
	if(m_window)
	{
		glfwDestroyWindow(m_window);
		m_window = nullptr;
	}

	int monCount;
	GLFWmonitor** mons = glfwGetMonitors(&monCount);


	if(monitor < monCount)
		m_monitor = mons[m_info->monitor = monitor];
	else
		m_monitor = glfwGetPrimaryMonitor();

	//int tmp;
	const GLFWvidmode* mode = glfwGetVideoMode(m_monitor);

	size.width = ((int)size.width < mode->width ? size.width : mode->width);
	size.height = ((int)size.height < mode->height ? size.height : mode->height);
	m_info->size = size;
	m_info->position = position;


	m_window = glfwCreateWindow(mode->width, mode->height, (m_info->title = name).c_str(), nullptr, nullptr);
	
	glfwSetWindowSize(m_window, m_info->size.width, m_info->size.height);

	glfwMakeContextCurrent(m_window); //gives opengl the window it renders to

	setFullScreen(fullScreeen);
	setVisable(visable);

	
	glfwSetFramebufferSizeCallback(m_window, onWindowResize);

	return m_window != nullptr ? WINDOW_CREATED : WINDOW_FAILED;
}

void WindowCreator::setVisable(bool viz)
{
	if(viz)
		glfwShowWindow(m_window);
	else
		glfwHideWindow(m_window);
}

void WindowCreator::setFullScreen(bool full)
{
	if(m_full == full)
		return;//Return if the window is fullscreen


	static int w, h;
	static double x, y;
	if(full)
	{

		glfwGetWindowSize(m_window, &w, &h);
		m_info->size = Coord2D<int>{w, h};
		glfwGetWindowPos(m_window, &w, &h);
		m_info->position = Coord2D<int>{w, h};


		glfwGetCursorPos(m_window, &x, &y);

		// get reolution of monitor
		int tmp;
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetMonitors(&tmp)[m_info->monitor]);

		// switch to full screen
		glfwGetFramebufferSize(m_window, &w, &h);
		glViewport(0, 0, w, h);
		glfwSetWindowMonitor(m_window, m_monitor, 0, 0, mode->width, mode->height, 0);

		glfwSetCursorPos(m_window, x, y);
	}
	else
	{
		glfwGetCursorPos(m_window, &x, &y);
		glfwGetFramebufferSize(m_window, &w, &h);
		glViewport(0, 0, w, h);
		glfwSetWindowMonitor(m_window, nullptr, int(m_info->position.x), int(m_info->position.y), int(m_info->size.width), int(m_info->size.height), 0);
		glfwSetCursorPos(m_window, x, y);
		//	m_monitor = nullptr;
	}

	m_full = full;
}

GLFWwindow* WindowCreator::getWindow()
{
	return m_window;
}

std::string& WindowCreator::getTitle()
{
	return m_info->title;
}

Coord2D<int>& WindowCreator::getScreenSize()
{
	return m_info->size;
}

int WindowCreator::getScreenWidth()
{
	return getScreenSize().width;
}

int WindowCreator::getScreenHeight()
{
	return getScreenSize().height;
}

void WindowCreator::onWindowResize(GLFWwindow* glfw, int w, int h)
{
	//createWindow(m_info->title, {w,h}, m_info->position, m_info->monitor, m_full, true);
	m_info->size = {w,h};
	if(m_onWindowResizeCallback)
		m_onWindowResizeCallback(glfw,w,h);
	
}

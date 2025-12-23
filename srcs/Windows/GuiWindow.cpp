#include "GuiWindow.hpp"
#include "ModeManager.hpp"
#include "ModeEditor.hpp"
#include "StatisticsWindow.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

GuiWindow::GuiWindow(void)
	: _window(NULL), _glslVersion("#version 130")
{
}

GuiWindow::~GuiWindow(void)
{
	cleanup();
}

bool	GuiWindow::init(void)
{
	if (!initGLFW())
		return (false);
	if (!createWindow())
		return (false);
	initImGui();
	return (true);
}

bool	GuiWindow::initGLFW(void)
{
	if (!glfwInit())
		return (false);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

	return (true);
}

bool	GuiWindow::createWindow(void)
{
	_window = glfwCreateWindow(1480, 820, "JeanClaude Math", NULL, NULL);
	if (!_window)
	{
		glfwTerminate();
		return (false);
	}

	glfwMakeContextCurrent(_window);
	glfwSwapInterval(1);
	return (true);
}

void	GuiWindow::initImGui(void)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO	&io = ImGui::GetIO();
	(void)io;
	ImGui::StyleColorsDark();

	ImGuiStyle&	style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

	ImGui_ImplGlfw_InitForOpenGL(_window, true);
	ImGui_ImplOpenGL3_Init(_glslVersion);
}

void	GuiWindow::run(void)
{
	ModeManager			modeManager;
	ModeEditor			editor;
	StatisticsWindow	statsWindow;

	modeManager.addDefaultMode();

	while (!glfwWindowShouldClose(_window))
	{
		beginFrame();
		editor.render(modeManager, _window);
		statsWindow.render(modeManager);
		endFrame();
	}
}

void	GuiWindow::beginFrame(void)
{
	glfwPollEvents();

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void	GuiWindow::endFrame(void)
{
	ImGui::Render();
	int	displayW, displayH;
	glfwGetFramebufferSize(_window, &displayW, &displayH);
	glViewport(0, 0, displayW, displayH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.6f);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(_window);
}

void	GuiWindow::cleanup(void)
{
	if (_window)
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		glfwDestroyWindow(_window);
		glfwTerminate();
		_window = NULL;
	}
}

GLFWwindow*	GuiWindow::getWindow(void)
{
	return (_window);
}

bool	GuiWindow::shouldClose(void) const
{
	return (_window && glfwWindowShouldClose(_window));
}

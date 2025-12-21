#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "GuiWindow.hpp"

int	main(void)
{
	GuiWindow	window;

	if (!window.init())
		return (1);
	window.run();
	return (0);
}

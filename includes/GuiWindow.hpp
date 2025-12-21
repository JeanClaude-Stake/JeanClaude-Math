#ifndef GUIWINDOW_HPP
# define GUIWINDOW_HPP

# include <GLFW/glfw3.h>

class GuiWindow
{
	public:
		GuiWindow(void);
		~GuiWindow(void);

		bool				init(void);
		void				run(void);
		GLFWwindow*			getWindow(void);
		bool				shouldClose(void) const;

	private:
		GLFWwindow*			_window;
		const char*			_glslVersion;

		bool				initGLFW(void);
		bool				createWindow(void);
		void				initImGui(void);
		void				cleanup(void);
		void				beginFrame(void);
		void				endFrame(void);
};

#endif

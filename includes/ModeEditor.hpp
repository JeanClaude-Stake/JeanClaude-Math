#ifndef MODEEDITOR_HPP
# define MODEEDITOR_HPP

# include "ModeManager.hpp"
# include <GLFW/glfw3.h>
# include <string>

class ModeEditor
{
	public:
		ModeEditor(void);
		~ModeEditor(void);

		void				render(ModeManager &modeManager, GLFWwindow *window);

		int					getNumSimulations(void) const;
		void				setNumSimulations(int value);
		const char*			getOutputDir(void) const;
		void				setOutputDir(const char *dir);

		void				setStatusMessage(const std::string &msg);
		const std::string&	getStatusMessage(void) const;

		bool				hasExported(void) const;
		void				setExported(bool exported);

	private:
		int					_numSimulations;
		char				_outputDir[256];
		bool				_exported;
		bool				_isSimulating;
		std::string			_statusMsg;

		void				renderHeader(void);
		void				renderModePanel(ModeEntry &mode, int index);
		void				renderMultipliersTable(ModeEntry &mode);
		void				renderSettings(void);
		void				renderModesList(ModeManager &modeManager);
		void				renderActions(ModeManager &modeManager);
		void				renderExportPreview(const ModeManager &modeManager);
		void				renderStatus(void);
};

#endif

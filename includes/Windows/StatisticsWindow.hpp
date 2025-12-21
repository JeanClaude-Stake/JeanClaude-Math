#ifndef STATISTICSWINDOW_HPP
# define STATISTICSWINDOW_HPP

# include "ModeManager.hpp"

class StatisticsWindow
{
	public:
		StatisticsWindow(void);
		~StatisticsWindow(void);

		void	render(ModeManager &modeManager);

	private:
		int		_selectedModeIndex;

		void	renderModeSelector(ModeManager &modeManager);
		void	renderStatsTable(const ModeEntry &mode);
		void	renderNoDataWarning(void);
};

#endif

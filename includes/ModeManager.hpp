#ifndef MODEMANAGER_HPP
# define MODEMANAGER_HPP

# include <vector>
# include <string>
# include "Distribution.hpp"

struct MultiplierEntry
{
	float		multiplier;
	int			weight;
};

struct ModeEntry
{
	char						name[64];
	float						cost;
	std::vector<MultiplierEntry>	multipliers;
	bool						simulated;
	double						rtp;
	size_t						simCount;
};

class ModeManager
{
	public:
		ModeManager(void);
		~ModeManager(void);

		void						addDefaultMode(void);
		void						removeLastMode(void);
		void						runAllSimulations(int numSimulations);
		bool						exportFiles(const char *outputDir);

		std::vector<ModeEntry>&		getModes(void);
		const std::vector<ModeEntry>&	getModes(void) const;
		size_t						getModeCount(void) const;
		const Distribution&			getDistribution(void) const;

	private:
		std::vector<ModeEntry>		_modes;
		Distribution				_dist;
};

#endif

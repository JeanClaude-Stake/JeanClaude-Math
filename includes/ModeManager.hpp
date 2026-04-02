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

struct StatisticsCache
{
	bool	calculated;
	double	meanPayout;
	double	variance;
	double	stdDeviation;
	double	volatility;
	double	hitFrequency;
	double	minPayout;
	double	maxPayout;
};

struct FreeSpinsConfig
{
	bool	enabled;
	int		triggerWeight;		// Weight for triggering free spins (in the distribution)
	int		count;				// Number of free spins awarded
	float	multiplierBoost;	// Multiplier applied to winnings during free spins (e.g. 2.0 = x2)
	bool	canRetrigger;		// Can free spins retrigger more free spins?
};

struct ModeEntry
{
	char						name[64];
	float						cost;
	std::vector<MultiplierEntry>	multipliers;
	FreeSpinsConfig				freeSpins;
	bool						simulated;
	double						rtp;
	size_t						simCount;
	StatisticsCache				stats;
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

		bool						saveConfig(const char *path) const;
		bool						loadConfig(const char *path);

	private:
		std::vector<ModeEntry>		_modes;
		Distribution				_dist;
};

#endif

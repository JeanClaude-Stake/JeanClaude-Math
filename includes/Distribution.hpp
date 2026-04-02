#ifndef DISTRIBUTION_HPP
# define DISTRIBUTION_HPP

# include <vector>
# include <string>
# include <cstdint>
# include <random>
# include <map>
# include <zstd.h>

struct MultiplierConfig
{
	double		multiplier;
	uint64_t	weight;
};

// Game event matching Stake Engine format
// Types: "reveal", "winInfo", "setWin", "setTotalWin", "finalWin"
struct GameEvent
{
	int			index;
	std::string	type;		// "reveal", "winInfo", "setWin", "setTotalWin", "finalWin"
	std::string	gameType;	// "basegame" or "freegame" (used in reveal events)
	int			amount;		// Win amount (for setWin, setTotalWin, finalWin)
	int			totalWin;	// For winInfo
	int			winLevel;	// For setWin (win tier for UI animation)

	GameEvent(void);
	GameEvent(int idx, const std::string &t, int amt);
	GameEvent(int idx, const std::string &t, const std::string &gType);
};

// A single simulation/round result
struct Simulation
{
	uint64_t				id;
	uint64_t				weight;
	uint64_t				payoutMultiplier;	// In hundredths: 150 = 1.5x
	double					baseGameWins;		// Base game RTP portion
	double					freeGameWins;		// Free game RTP portion
	std::vector<GameEvent>	events;
};

struct FreeSpinsMode
{
	bool		enabled;
	uint64_t	triggerWeight;		// Added to totalWeight distribution
	int			count;				// Number of free spins
	double		multiplierBoost;	// Multiplier for free spin winnings
	bool		canRetrigger;
};

struct GameMode
{
	std::string						name;
	double							cost;
	std::vector<MultiplierConfig>	multipliers;
	std::vector<Simulation>			simulations;
	uint64_t						totalWeight;
	FreeSpinsMode					freeSpins;
};

class Distribution
{
	public:
		Distribution(void);
		~Distribution(void);

		void		addMode(const std::string &name, double cost);
		void		addMultiplier(const std::string &mode,
						double multiplier, uint64_t weight);
		void		setFreeSpins(const std::string &mode,
						uint64_t triggerWeight, int count,
						double multiplierBoost, bool canRetrigger);
		void		runSimulations(const std::string &mode,
						size_t count, uint64_t seed);

		size_t		modeCount(void) const;
		size_t		simulationCount(const std::string &mode) const;
		double		getRTP(const std::string &mode) const;

		double		getMeanPayout(const std::string &mode) const;
		double		getVariance(const std::string &mode) const;
		double		getStandardDeviation(const std::string &mode) const;
		double		getVolatility(const std::string &mode) const;
		double		getHitFrequency(const std::string &mode) const;
		double		getMinPayout(const std::string &mode) const;
		double		getMaxPayout(const std::string &mode) const;

		bool		exportAll(const std::string &outputDir) const;

	private:
		std::map<std::string, GameMode>	_modes;

		uint64_t	pickMultiplier(const GameMode &mode,
						std::mt19937_64 &rng) const;
		bool		exportCSV(const std::string &path,
						const GameMode &mode) const;
		bool		exportJSONLCompressed(const std::string &path,
						const GameMode &mode) const;
		bool		exportIndex(const std::string &path) const;
		std::string	formatGameEvent(const GameEvent &event) const;
		std::string	formatSimulation(const Simulation &sim) const;
};

#endif

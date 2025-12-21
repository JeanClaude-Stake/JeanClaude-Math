#ifndef DISTRIBUTION_HPP
# define DISTRIBUTION_HPP

# include <vector>
# include <string>
# include <cstdint>
# include <random>
# include <map>
# include <zstd.h>
# include "EventManager.hpp"

struct MultiplierConfig
{
	double		multiplier;
	uint64_t	weight;
};

struct SimulationEvent
{
	std::string	eventId;
	double		rtpBoost;
};

struct Simulation
{
	uint64_t					id;
	uint64_t					weight;
	uint64_t					payoutMultiplier;
	std::vector<SimulationEvent>	events;
};

struct GameMode
{
	std::string						name;
	double							cost;
	std::vector<MultiplierConfig>	multipliers;
	std::vector<Simulation>			simulations;
	uint64_t						totalWeight;
};

class Distribution
{
	public:
		Distribution(void);
		~Distribution(void);

		void		addMode(const std::string &name, double cost);
		void		addMultiplier(const std::string &mode,
						double multiplier, uint64_t weight);
		void		runSimulations(const std::string &mode,
						size_t count, uint64_t seed);
		void		runSimulationsWithEvents(const std::string &mode,
						size_t count, uint64_t seed,
						const EventManager &eventMgr);

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

		void		setEventManager(const EventManager &eventMgr);
		const EventManager&	getEventManager(void) const;
		EventManager&		getEventManager(void);

		bool		exportAll(const std::string &outputDir) const;

	private:
		std::map<std::string, GameMode>	_modes;
		EventManager					_eventManager;

		uint64_t	pickMultiplier(const GameMode &mode,
						std::mt19937_64 &rng) const;
		bool		exportCSV(const std::string &path,
						const GameMode &mode) const;
		bool		exportJSONLCompressed(const std::string &path,
						const GameMode &mode) const;
		bool		exportIndex(const std::string &path) const;
		bool		exportEventsJSON(const std::string &path) const;
		std::string	formatSimulationEvents(
						const std::vector<SimulationEvent> &events) const;
};

#endif

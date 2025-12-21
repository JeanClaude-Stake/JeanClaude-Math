#include "ModeManager.hpp"
#include <cstdio>
#include <sys/stat.h>

ModeManager::ModeManager(void)
{
}

ModeManager::~ModeManager(void)
{
}

void	ModeManager::addDefaultMode(void)
{
	ModeEntry	mode;

	snprintf(mode.name, sizeof(mode.name), "mode_%zu", _modes.size());
	mode.cost = 1.0f;
	mode.simulated = false;
	mode.rtp = 0.0;
	mode.simCount = 0;
	mode.stats.calculated = false;
	mode.multipliers.push_back({0.0f, 350});
	mode.multipliers.push_back({1.0f, 200});
	mode.multipliers.push_back({2.0f, 100});
	_modes.push_back(mode);
}

void	ModeManager::removeLastMode(void)
{
	if (_modes.size() > 0)
		_modes.pop_back();
}

void	ModeManager::runAllSimulations(int numSimulations)
{
	_dist = Distribution();
	for (size_t i = 0; i < _modes.size(); i++)
	{
		ModeEntry	&mode = _modes[i];

		_dist.addMode(mode.name, mode.cost);
		for (size_t j = 0; j < mode.multipliers.size(); j++)
		{
			_dist.addMultiplier(mode.name,
				mode.multipliers[j].multiplier,
				mode.multipliers[j].weight);
		}
		_dist.runSimulations(mode.name, numSimulations, 42 + i);
		mode.rtp = _dist.getRTP(mode.name);
		mode.simCount = _dist.simulationCount(mode.name);
		mode.simulated = true;
		mode.stats.calculated = true;
		mode.stats.meanPayout = _dist.getMeanPayout(mode.name);
		mode.stats.variance = _dist.getVariance(mode.name);
		mode.stats.stdDeviation = _dist.getStandardDeviation(mode.name);
		mode.stats.volatility = _dist.getVolatility(mode.name);
		mode.stats.hitFrequency = _dist.getHitFrequency(mode.name);
		mode.stats.minPayout = _dist.getMinPayout(mode.name);
		mode.stats.maxPayout = _dist.getMaxPayout(mode.name);
	}
}

bool	ModeManager::exportFiles(const char *outputDir)
{
	if (_dist.modeCount() == 0)
		return (false);
	mkdir(outputDir, 0755);
	return (_dist.exportAll(outputDir));
}

std::vector<ModeEntry>&	ModeManager::getModes(void)
{
	return (_modes);
}

const std::vector<ModeEntry>&	ModeManager::getModes(void) const
{
	return (_modes);
}

size_t	ModeManager::getModeCount(void) const
{
	return (_modes.size());
}

const Distribution&	ModeManager::getDistribution(void) const
{
	return (_dist);
}

#include "Distribution.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <cmath>
#include <sstream>

GameEvent::GameEvent(void)
	: index(0), type("reveal"), multiplier(0.0), amount(0)
{
}

GameEvent::GameEvent(int idx, const std::string &t, double mult, int amt)
	: index(idx), type(t), multiplier(mult), amount(amt)
{
}

Distribution::Distribution(void)
{
}

Distribution::~Distribution(void)
{
}

void	Distribution::addMode(const std::string &name, double cost)
{
	GameMode	mode;

	mode.name = name;
	mode.cost = cost;
	mode.totalWeight = 0;
	_modes[name] = mode;
}

void	Distribution::addMultiplier(const std::string &mode,
		double multiplier, uint64_t weight)
{
	MultiplierConfig	config;

	if (_modes.find(mode) == _modes.end())
		return ;
	config.multiplier = multiplier;
	config.weight = weight;
	_modes[mode].multipliers.push_back(config);
	_modes[mode].totalWeight += weight;
}

uint64_t	Distribution::pickMultiplier(const GameMode &mode,
		std::mt19937_64 &rng) const
{
	std::uniform_int_distribution<uint64_t>	dist(0, mode.totalWeight - 1);
	uint64_t								roll;
	uint64_t								cumulative;

	roll = dist(rng);
	cumulative = 0;
	for (size_t i = 0; i < mode.multipliers.size(); i++)
	{
		cumulative += mode.multipliers[i].weight;
		if (roll < cumulative)
			return (static_cast<uint64_t>(mode.multipliers[i].multiplier * 100));
	}
	return (0);
}

void	Distribution::runSimulations(const std::string &mode,
		size_t count, uint64_t seed)
{
	std::mt19937_64	rng(seed);
	Simulation		sim;
	double			mult;

	if (_modes.find(mode) == _modes.end())
		return ;
	_modes[mode].simulations.clear();
	_modes[mode].simulations.reserve(count);
	for (size_t i = 0; i < count; i++)
	{
		sim.id = i + 1;
		sim.weight = 1;
		sim.payoutMultiplier = pickMultiplier(_modes[mode], rng);
		sim.events.clear();
		mult = sim.payoutMultiplier / 100.0;
		sim.events.push_back(GameEvent(0, "reveal", mult,
			static_cast<int>(sim.payoutMultiplier)));
		sim.events.push_back(GameEvent(1, "finalWin", mult,
			static_cast<int>(sim.payoutMultiplier)));
		_modes[mode].simulations.push_back(sim);
	}
}

size_t	Distribution::modeCount(void) const
{
	return (_modes.size());
}

size_t	Distribution::simulationCount(const std::string &mode) const
{
	std::map<std::string, GameMode>::const_iterator	it;

	it = _modes.find(mode);
	if (it == _modes.end())
		return (0);
	return (it->second.simulations.size());
}

double	Distribution::getRTP(const std::string &mode) const
{
	std::map<std::string, GameMode>::const_iterator	it;
	double											totalPayout;
	uint64_t										totalWeight;

	it = _modes.find(mode);
	if (it == _modes.end() || it->second.simulations.empty())
		return (0.0);
	totalPayout = 0.0;
	totalWeight = 0;
	for (size_t i = 0; i < it->second.simulations.size(); i++)
	{
		totalPayout += it->second.simulations[i].weight
			* (it->second.simulations[i].payoutMultiplier / 100.0);
		totalWeight += it->second.simulations[i].weight;
	}
	return (totalPayout / totalWeight);
}

double	Distribution::getMeanPayout(const std::string &mode) const
{
	std::map<std::string, GameMode>::const_iterator	it;
	double											sum;
	size_t											count;

	it = _modes.find(mode);
	if (it == _modes.end() || it->second.simulations.empty())
		return (0.0);
	sum = 0.0;
	count = it->second.simulations.size();
	for (size_t i = 0; i < count; i++)
		sum += it->second.simulations[i].payoutMultiplier / 100.0;
	return (sum / count);
}

double	Distribution::getVariance(const std::string &mode) const
{
	std::map<std::string, GameMode>::const_iterator	it;
	double											mean;
	double											sumSquaredDiff;
	double											payout;
	size_t											count;

	it = _modes.find(mode);
	if (it == _modes.end() || it->second.simulations.empty())
		return (0.0);
	mean = getMeanPayout(mode);
	sumSquaredDiff = 0.0;
	count = it->second.simulations.size();
	for (size_t i = 0; i < count; i++)
	{
		payout = it->second.simulations[i].payoutMultiplier / 100.0;
		sumSquaredDiff += (payout - mean) * (payout - mean);
	}
	return (sumSquaredDiff / count);
}

double	Distribution::getStandardDeviation(const std::string &mode) const
{
	return (std::sqrt(getVariance(mode)));
}

double	Distribution::getVolatility(const std::string &mode) const
{
	double	mean;
	double	stdDev;

	mean = getMeanPayout(mode);
	if (mean < 0.0001)
		return (0.0);
	stdDev = getStandardDeviation(mode);
	return (stdDev / mean);
}

double	Distribution::getHitFrequency(const std::string &mode) const
{
	std::map<std::string, GameMode>::const_iterator	it;
	size_t											winCount;
	size_t											count;

	it = _modes.find(mode);
	if (it == _modes.end() || it->second.simulations.empty())
		return (0.0);
	winCount = 0;
	count = it->second.simulations.size();
	for (size_t i = 0; i < count; i++)
	{
		if (it->second.simulations[i].payoutMultiplier > 0)
			winCount++;
	}
	return ((double)winCount / count * 100.0);
}

double	Distribution::getMinPayout(const std::string &mode) const
{
	std::map<std::string, GameMode>::const_iterator	it;
	double											minVal;
	double											payout;

	it = _modes.find(mode);
	if (it == _modes.end() || it->second.simulations.empty())
		return (0.0);
	minVal = it->second.simulations[0].payoutMultiplier / 100.0;
	for (size_t i = 1; i < it->second.simulations.size(); i++)
	{
		payout = it->second.simulations[i].payoutMultiplier / 100.0;
		if (payout < minVal)
			minVal = payout;
	}
	return (minVal);
}

double	Distribution::getMaxPayout(const std::string &mode) const
{
	std::map<std::string, GameMode>::const_iterator	it;
	double											maxVal;
	double											payout;

	it = _modes.find(mode);
	if (it == _modes.end() || it->second.simulations.empty())
		return (0.0);
	maxVal = it->second.simulations[0].payoutMultiplier / 100.0;
	for (size_t i = 1; i < it->second.simulations.size(); i++)
	{
		payout = it->second.simulations[i].payoutMultiplier / 100.0;
		if (payout > maxVal)
			maxVal = payout;
	}
	return (maxVal);
}

std::string	Distribution::formatGameEvent(const GameEvent &event) const
{
	std::ostringstream	json;

	json << "{\"index\":" << event.index;
	json << ",\"type\":\"" << event.type << "\"";
	json << ",\"multiplier\":" << std::fixed << std::setprecision(1)
		 << event.multiplier;
	json << ",\"amount\":" << event.amount << "}";
	return (json.str());
}

std::string	Distribution::formatSimulation(const Simulation &sim) const
{
	std::ostringstream	json;

	json << "{\"id\":" << sim.id;
	json << ",\"payoutMultiplier\":" << sim.payoutMultiplier;
	json << ",\"events\":[";
	for (size_t i = 0; i < sim.events.size(); i++)
	{
		if (i > 0)
			json << ",";
		json << formatGameEvent(sim.events[i]);
	}
	json << "]}";
	return (json.str());
}

bool	Distribution::exportCSV(const std::string &path,
		const GameMode &mode) const
{
	std::ofstream	file(path);

	if (!file.is_open())
	{
		std::cerr << "Error: cannot open " << path << std::endl;
		return (false);
	}
	for (size_t i = 0; i < mode.simulations.size(); i++)
	{
		file << mode.simulations[i].id << ","
			 << mode.simulations[i].weight << ","
			 << mode.simulations[i].payoutMultiplier << "\n";
	}
	file.close();
	return (true);
}

bool	Distribution::exportJSONLCompressed(const std::string &path,
		const GameMode &mode) const
{
	std::string	jsonData;

	for (size_t i = 0; i < mode.simulations.size(); i++)
		jsonData += formatSimulation(mode.simulations[i]) + "\n";

	size_t				compressBound = ZSTD_compressBound(jsonData.size());
	std::vector<char>	compressedData(compressBound);
	size_t				compressedSize;

	compressedSize = ZSTD_compress(compressedData.data(), compressBound,
		jsonData.data(), jsonData.size(), 3);
	if (ZSTD_isError(compressedSize))
	{
		std::cerr << "Error: ZSTD compression failed" << std::endl;
		return (false);
	}

	std::ofstream	file(path, std::ios::binary);

	if (!file.is_open())
	{
		std::cerr << "Error: cannot open " << path << std::endl;
		return (false);
	}
	file.write(compressedData.data(), compressedSize);
	file.close();
	return (true);
}

bool	Distribution::exportIndex(const std::string &path) const
{
	std::ofstream								file(path);
	std::map<std::string, GameMode>::const_iterator	it;
	bool										first;

	if (!file.is_open())
	{
		std::cerr << "Error: cannot open " << path << std::endl;
		return (false);
	}
	file << "{\n";
	file << "  \"modes\": [\n";
	first = true;
	for (it = _modes.begin(); it != _modes.end(); ++it)
	{
		if (!first)
			file << ",\n";
		file << "    {\n";
		file << "      \"name\": \"" << it->second.name << "\",\n";
		file << "      \"cost\": " << std::fixed << std::setprecision(1)
			 << it->second.cost << ",\n";
		file << "      \"events\": \"books_" << it->second.name
			 << ".jsonl.zst\",\n";
		file << "      \"weights\": \"lookUpTable_" << it->second.name
			 << "_0.csv\"\n";
		file << "    }";
		first = false;
	}
	file << "\n  ]\n";
	file << "}\n";
	file.close();
	return (true);
}

bool	Distribution::exportAll(const std::string &outputDir) const
{
	std::map<std::string, GameMode>::const_iterator	it;
	std::string										csvPath;
	std::string										jsonlPath;

	for (it = _modes.begin(); it != _modes.end(); ++it)
	{
		csvPath = outputDir + "/lookUpTable_" + it->second.name + "_0.csv";
		jsonlPath = outputDir + "/books_" + it->second.name + ".jsonl.zst";
		if (!exportCSV(csvPath, it->second))
			return (false);
		if (!exportJSONLCompressed(jsonlPath, it->second))
			return (false);
		std::cout << "  Mode '" << it->second.name << "':" << std::endl;
		std::cout << "    CSV: " << csvPath << std::endl;
		std::cout << "    JSONL: " << jsonlPath << std::endl;
	}
	if (!exportIndex(outputDir + "/index.json"))
		return (false);
	std::cout << "  Index: " << outputDir << "/index.json" << std::endl;
	return (true);
}

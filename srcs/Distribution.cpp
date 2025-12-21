#include "Distribution.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>

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

	if (_modes.find(mode) == _modes.end())
		return ;
	_modes[mode].simulations.clear();
	_modes[mode].simulations.reserve(count);
	for (size_t i = 0; i < count; i++)
	{
		sim.id = i;
		sim.weight = 1;
		sim.payoutMultiplier = pickMultiplier(_modes[mode], rng);
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
	std::string	line;

	for (size_t i = 0; i < mode.simulations.size(); i++)
	{
		line = "{\"id\":" + std::to_string(mode.simulations[i].id)
			 + ",\"events\":[]"
			 + ",\"payoutMultiplier\":"
			 + std::to_string(mode.simulations[i].payoutMultiplier)
			 + "}\n";
		jsonData += line;
	}

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
		file << "      \"events\": \"books_" << it->second.name << ".jsonl.zst\",\n";
		file << "      \"weights\": \"lookUpTable_" << it->second.name << ".csv\"\n";
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
		csvPath = outputDir + "/lookUpTable_" + it->second.name + ".csv";
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

#include "ModeManager.hpp"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#ifdef _WIN32
#  include <direct.h>   // _mkdir
#  define MAKE_DIR(p) _mkdir(p)
#else
#  include <sys/stat.h>
#  define MAKE_DIR(p) mkdir(p, 0755)
#endif

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
	mode.freeSpins.enabled = false;
	mode.freeSpins.triggerWeight = 50;
	mode.freeSpins.count = 10;
	mode.freeSpins.multiplierBoost = 2.0f;
	mode.freeSpins.canRetrigger = false;
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
		if (mode.freeSpins.enabled)
		{
			_dist.setFreeSpins(mode.name,
				mode.freeSpins.triggerWeight,
				mode.freeSpins.count,
				mode.freeSpins.multiplierBoost,
				mode.freeSpins.canRetrigger);
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
	MAKE_DIR(outputDir);
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

bool	ModeManager::saveConfig(const char *path) const
{
	std::ofstream	file(path);

	if (!file.is_open())
		return (false);
	file << "{\n";
	file << "  \"modes\": [\n";
	for (size_t i = 0; i < _modes.size(); i++)
	{
		const ModeEntry	&m = _modes[i];
		if (i > 0)
			file << ",\n";
		file << "    {\n";
		file << "      \"name\": \"" << m.name << "\",\n";
		file << "      \"cost\": " << m.cost << ",\n";
		file << "      \"freeSpins\": {\n";
		file << "        \"enabled\": " << (m.freeSpins.enabled ? "true" : "false") << ",\n";
		file << "        \"triggerWeight\": " << m.freeSpins.triggerWeight << ",\n";
		file << "        \"count\": " << m.freeSpins.count << ",\n";
		file << "        \"multiplierBoost\": " << m.freeSpins.multiplierBoost << ",\n";
		file << "        \"canRetrigger\": " << (m.freeSpins.canRetrigger ? "true" : "false") << "\n";
		file << "      },\n";
		file << "      \"multipliers\": [\n";
		for (size_t j = 0; j < m.multipliers.size(); j++)
		{
			if (j > 0)
				file << ",\n";
			file << "        { \"multiplier\": " << m.multipliers[j].multiplier
				 << ", \"weight\": " << m.multipliers[j].weight << " }";
		}
		file << "\n      ]\n";
		file << "    }";
	}
	file << "\n  ]\n";
	file << "}\n";
	file.close();
	return (true);
}

// Simple JSON value extractor helpers (no external lib)
static std::string	extractString(const std::string &json, const std::string &key)
{
	std::string	search = "\"" + key + "\": \"";
	size_t		pos = json.find(search);
	if (pos == std::string::npos)
	{
		search = "\"" + key + "\":\"";
		pos = json.find(search);
		if (pos == std::string::npos)
			return ("");
	}
	pos += search.size();
	size_t	end = json.find("\"", pos);
	if (end == std::string::npos)
		return ("");
	return (json.substr(pos, end - pos));
}

static double	extractNumber(const std::string &json, const std::string &key)
{
	std::string	search = "\"" + key + "\": ";
	size_t		pos = json.find(search);
	if (pos == std::string::npos)
	{
		search = "\"" + key + "\":";
		pos = json.find(search);
		if (pos == std::string::npos)
			return (0.0);
	}
	pos += search.size();
	return (std::stod(json.substr(pos)));
}

static bool	extractBool(const std::string &json, const std::string &key)
{
	std::string	search = "\"" + key + "\": ";
	size_t		pos = json.find(search);
	if (pos == std::string::npos)
	{
		search = "\"" + key + "\":";
		pos = json.find(search);
		if (pos == std::string::npos)
			return (false);
	}
	pos += search.size();
	return (json.substr(pos, 4) == "true");
}

bool	ModeManager::loadConfig(const char *path)
{
	std::ifstream	file(path);

	if (!file.is_open())
		return (false);

	std::stringstream	ss;
	ss << file.rdbuf();
	std::string	content = ss.str();
	file.close();

	_modes.clear();

	// Find each mode block between { } inside "modes" array
	std::string	modesKey = "\"modes\"";
	size_t	modesPos = content.find(modesKey);
	if (modesPos == std::string::npos)
		return (false);

	size_t	arrayStart = content.find("[", modesPos);
	if (arrayStart == std::string::npos)
		return (false);

	// Parse each mode object
	size_t	pos = arrayStart;
	while (true)
	{
		size_t	modeStart = content.find("{", pos + 1);
		if (modeStart == std::string::npos)
			break ;

		// Find matching closing brace (handle nested objects)
		int		depth = 0;
		size_t	modeEnd = modeStart;
		for (size_t k = modeStart; k < content.size(); k++)
		{
			if (content[k] == '{')
				depth++;
			else if (content[k] == '}')
			{
				depth--;
				if (depth == 0)
				{
					modeEnd = k;
					break ;
				}
			}
		}
		if (modeEnd == modeStart)
			break ;

		std::string	modeJson = content.substr(modeStart, modeEnd - modeStart + 1);

		ModeEntry	mode;
		std::string	name = extractString(modeJson, "name");
#ifdef _WIN32
		strncpy_s(mode.name, sizeof(mode.name), name.c_str(), sizeof(mode.name) - 1);
#else
		strncpy(mode.name, name.c_str(), sizeof(mode.name) - 1);
#endif
		mode.name[sizeof(mode.name) - 1] = '\0';
		mode.cost = static_cast<float>(extractNumber(modeJson, "cost"));
		mode.simulated = false;
		mode.rtp = 0.0;
		mode.simCount = 0;
		mode.stats.calculated = false;

		// Parse freeSpins
		size_t	fsPos = modeJson.find("\"freeSpins\"");
		if (fsPos != std::string::npos)
		{
			size_t	fsStart = modeJson.find("{", fsPos);
			size_t	fsEnd = modeJson.find("}", fsStart);
			if (fsStart != std::string::npos && fsEnd != std::string::npos)
			{
				std::string	fsJson = modeJson.substr(fsStart, fsEnd - fsStart + 1);
				mode.freeSpins.enabled = extractBool(fsJson, "enabled");
				mode.freeSpins.triggerWeight = static_cast<int>(extractNumber(fsJson, "triggerWeight"));
				mode.freeSpins.count = static_cast<int>(extractNumber(fsJson, "count"));
				mode.freeSpins.multiplierBoost = static_cast<float>(extractNumber(fsJson, "multiplierBoost"));
				mode.freeSpins.canRetrigger = extractBool(fsJson, "canRetrigger");
			}
		}
		else
		{
			mode.freeSpins.enabled = false;
			mode.freeSpins.triggerWeight = 50;
			mode.freeSpins.count = 10;
			mode.freeSpins.multiplierBoost = 2.0f;
			mode.freeSpins.canRetrigger = false;
		}

		// Parse multipliers array
		size_t	multPos = modeJson.find("\"multipliers\"");
		if (multPos != std::string::npos)
		{
			size_t	multArrayStart = modeJson.find("[", multPos);
			size_t	multArrayEnd = modeJson.find("]", multArrayStart);
			if (multArrayStart != std::string::npos && multArrayEnd != std::string::npos)
			{
				std::string	multArray = modeJson.substr(multArrayStart,
					multArrayEnd - multArrayStart + 1);
				size_t	mPos = 0;
				while (true)
				{
					size_t	objStart = multArray.find("{", mPos);
					if (objStart == std::string::npos)
						break ;
					size_t	objEnd = multArray.find("}", objStart);
					if (objEnd == std::string::npos)
						break ;
					std::string	obj = multArray.substr(objStart, objEnd - objStart + 1);

					MultiplierEntry	entry;
					entry.multiplier = static_cast<float>(extractNumber(obj, "multiplier"));
					entry.weight = static_cast<int>(extractNumber(obj, "weight"));
					mode.multipliers.push_back(entry);
					mPos = objEnd + 1;
				}
			}
		}

		_modes.push_back(mode);
		pos = modeEnd + 1;

		// Check if we've reached the end of the modes array
		size_t	nextBrace = content.find("{", pos);
		size_t	arrayEnd = content.find("]", pos);
		if (arrayEnd != std::string::npos
			&& (nextBrace == std::string::npos || nextBrace > arrayEnd))
			break ;
	}
	return (!_modes.empty());
}

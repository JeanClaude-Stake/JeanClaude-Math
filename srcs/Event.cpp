#include "Event.hpp"
#include <random>
#include <algorithm>
#include <sstream>

EventTrigger::EventTrigger(void)
	: type(TriggerType::RANDOM), everyNGames(1000), probability(0.05)
{
	startTime = "00:00";
	endTime = "23:59";
}

EventModifiers::EventModifiers(void)
	: rtpBoost(1.0), hitFrequencyBoost(1.0), enableFreeSpins(false),
	  freeSpinsCount(0), freeSpinsMultiplier(1.0),
	  enableProgressiveJackpot(false), jackpotSeedValue(0.0),
	  jackpotContribution(0.0)
{
}

Event::Event(void)
	: _active(false)
{
}

Event::Event(const std::string &id, const std::string &name)
	: _id(id), _name(name), _active(true)
{
}

Event::~Event(void)
{
}

std::string	Event::getId(void) const
{
	return (_id);
}

std::string	Event::getName(void) const
{
	return (_name);
}

std::string	Event::getDescription(void) const
{
	return (_description);
}

bool	Event::isActive(void) const
{
	return (_active);
}

const EventTrigger&	Event::getTrigger(void) const
{
	return (_trigger);
}

const EventModifiers&	Event::getModifiers(void) const
{
	return (_modifiers);
}

void	Event::setId(const std::string &id)
{
	_id = id;
}

void	Event::setName(const std::string &name)
{
	_name = name;
}

void	Event::setDescription(const std::string &description)
{
	_description = description;
}

void	Event::setActive(bool active)
{
	_active = active;
}

void	Event::setTrigger(const EventTrigger &trigger)
{
	_trigger = trigger;
}

void	Event::setModifiers(const EventModifiers &modifiers)
{
	_modifiers = modifiers;
}

int	Event::parseTime(const std::string &time) const
{
	size_t	colonPos;
	int		hours;
	int		minutes;

	colonPos = time.find(':');
	if (colonPos == std::string::npos)
		return (-1);
	hours = std::stoi(time.substr(0, colonPos));
	minutes = std::stoi(time.substr(colonPos + 1));
	return (hours * 60 + minutes);
}

bool	Event::checkTimeTrigger(const std::string &day,
		const std::string &time) const
{
	bool	dayMatch;
	int		currentMinutes;
	int		startMinutes;
	int		endMinutes;

	dayMatch = false;
	for (const auto &d : _trigger.daysOfWeek)
	{
		if (d == day)
		{
			dayMatch = true;
			break ;
		}
	}
	if (!dayMatch)
		return (false);
	currentMinutes = parseTime(time);
	startMinutes = parseTime(_trigger.startTime);
	endMinutes = parseTime(_trigger.endTime);
	if (currentMinutes < 0 || startMinutes < 0 || endMinutes < 0)
		return (false);
	return (currentMinutes >= startMinutes && currentMinutes <= endMinutes);
}

bool	Event::checkGameCountTrigger(int gameCount) const
{
	if (_trigger.everyNGames <= 0)
		return (false);
	return (gameCount > 0 && gameCount % _trigger.everyNGames == 0);
}

bool	Event::checkRandomTrigger(void) const
{
	static std::random_device				rd;
	static std::mt19937						gen(rd());
	std::uniform_real_distribution<double>	dist(0.0, 1.0);

	return (dist(gen) < _trigger.probability);
}

bool	Event::shouldTrigger(int gameCount, const std::string &day,
		const std::string &time) const
{
	if (!_active)
		return (false);
	if (_trigger.type == TriggerType::TIME)
		return (checkTimeTrigger(day, time));
	if (_trigger.type == TriggerType::GAME_COUNT)
		return (checkGameCountTrigger(gameCount));
	if (_trigger.type == TriggerType::RANDOM)
		return (checkRandomTrigger());
	return (false);
}

bool	Event::isValid(void) const
{
	if (_id.empty() || _name.empty())
		return (false);
	if (_trigger.type == TriggerType::TIME)
	{
		if (_trigger.daysOfWeek.empty())
			return (false);
		if (parseTime(_trigger.startTime) < 0)
			return (false);
		if (parseTime(_trigger.endTime) < 0)
			return (false);
	}
	if (_trigger.type == TriggerType::GAME_COUNT)
	{
		if (_trigger.everyNGames <= 0)
			return (false);
	}
	if (_trigger.type == TriggerType::RANDOM)
	{
		if (_trigger.probability < 0.0 || _trigger.probability > 1.0)
			return (false);
	}
	return (true);
}

static std::string	triggerTypeToString(TriggerType type)
{
	if (type == TriggerType::TIME)
		return ("time");
	if (type == TriggerType::GAME_COUNT)
		return ("game_count");
	return ("random");
}

static std::string	escapedString(const std::string &s)
{
	std::string	result;

	result = "\"";
	for (char c : s)
	{
		if (c == '"')
			result += "\\\"";
		else if (c == '\\')
			result += "\\\\";
		else
			result += c;
	}
	result += "\"";
	return (result);
}

std::string	Event::toJSON(void) const
{
	std::ostringstream	json;

	json << "{";
	json << "\"id\":" << escapedString(_id) << ",";
	json << "\"name\":" << escapedString(_name) << ",";
	json << "\"description\":" << escapedString(_description) << ",";
	json << "\"active\":" << (_active ? "true" : "false") << ",";
	json << "\"trigger\":{";
	json << "\"type\":\"" << triggerTypeToString(_trigger.type) << "\",";
	json << "\"daysOfWeek\":[";
	for (size_t i = 0; i < _trigger.daysOfWeek.size(); i++)
	{
		if (i > 0)
			json << ",";
		json << escapedString(_trigger.daysOfWeek[i]);
	}
	json << "],";
	json << "\"startTime\":\"" << _trigger.startTime << "\",";
	json << "\"endTime\":\"" << _trigger.endTime << "\",";
	json << "\"everyNGames\":" << _trigger.everyNGames << ",";
	json << "\"probability\":" << _trigger.probability;
	json << "},";
	json << "\"modifiers\":{";
	json << "\"rtpBoost\":" << _modifiers.rtpBoost << ",";
	json << "\"hitFrequencyBoost\":" << _modifiers.hitFrequencyBoost << ",";
	if (_modifiers.maxMultiplierOverride.has_value())
		json << "\"maxMultiplierOverride\":" << *_modifiers.maxMultiplierOverride << ",";
	else
		json << "\"maxMultiplierOverride\":null,";
	json << "\"enableFreeSpins\":" << (_modifiers.enableFreeSpins ? "true" : "false") << ",";
	json << "\"freeSpinsCount\":" << _modifiers.freeSpinsCount << ",";
	json << "\"freeSpinsMultiplier\":" << _modifiers.freeSpinsMultiplier << ",";
	json << "\"enableProgressiveJackpot\":" << (_modifiers.enableProgressiveJackpot ? "true" : "false") << ",";
	json << "\"jackpotSeedValue\":" << _modifiers.jackpotSeedValue << ",";
	json << "\"jackpotContribution\":" << _modifiers.jackpotContribution;
	json << "}}";
	return (json.str());
}

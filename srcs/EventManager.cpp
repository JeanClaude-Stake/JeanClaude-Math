#include "EventManager.hpp"
#include <sstream>
#include <algorithm>

EventManager::EventManager(void)
{
}

EventManager::~EventManager(void)
{
}

int	EventManager::findEventIndex(const std::string &eventId) const
{
	for (size_t i = 0; i < _events.size(); i++)
	{
		if (_events[i].getId() == eventId)
			return (static_cast<int>(i));
	}
	return (-1);
}

void	EventManager::addEvent(const Event &event)
{
	if (findEventIndex(event.getId()) >= 0)
		return ;
	_events.push_back(event);
}

void	EventManager::removeEvent(const std::string &eventId)
{
	int	index;

	index = findEventIndex(eventId);
	if (index >= 0)
		_events.erase(_events.begin() + index);
}

void	EventManager::updateEvent(const std::string &eventId,
		const Event &updatedEvent)
{
	int	index;

	index = findEventIndex(eventId);
	if (index >= 0)
		_events[index] = updatedEvent;
}

Event*	EventManager::getEvent(const std::string &eventId)
{
	int	index;

	index = findEventIndex(eventId);
	if (index >= 0)
		return (&_events[index]);
	return (nullptr);
}

const Event*	EventManager::getEvent(const std::string &eventId) const
{
	int	index;

	index = findEventIndex(eventId);
	if (index >= 0)
		return (&_events[index]);
	return (nullptr);
}

std::vector<Event>&	EventManager::getAllEvents(void)
{
	return (_events);
}

const std::vector<Event>&	EventManager::getAllEvents(void) const
{
	return (_events);
}

std::vector<Event>	EventManager::getActiveEvents(void) const
{
	std::vector<Event>	active;

	for (const auto &event : _events)
	{
		if (event.isActive())
			active.push_back(event);
	}
	return (active);
}

void	EventManager::activateEvent(const std::string &eventId)
{
	int	index;

	index = findEventIndex(eventId);
	if (index >= 0)
		_events[index].setActive(true);
}

void	EventManager::deactivateEvent(const std::string &eventId)
{
	int	index;

	index = findEventIndex(eventId);
	if (index >= 0)
		_events[index].setActive(false);
}

void	EventManager::toggleEvent(const std::string &eventId)
{
	int	index;

	index = findEventIndex(eventId);
	if (index >= 0)
		_events[index].setActive(!_events[index].isActive());
}

std::vector<Event>	EventManager::getTriggeredEvents(int gameCount,
		const std::string &day, const std::string &time) const
{
	std::vector<Event>	triggered;

	for (const auto &event : _events)
	{
		if (event.shouldTrigger(gameCount, day, time))
			triggered.push_back(event);
	}
	return (triggered);
}

std::string	EventManager::toJSON(void) const
{
	std::ostringstream	json;

	json << "[";
	for (size_t i = 0; i < _events.size(); i++)
	{
		if (i > 0)
			json << ",";
		json << _events[i].toJSON();
	}
	json << "]";
	return (json.str());
}

bool	EventManager::hasConflicts(void) const
{
	for (size_t i = 0; i < _events.size(); i++)
	{
		for (size_t j = i + 1; j < _events.size(); j++)
		{
			if (_events[i].getId() == _events[j].getId())
				return (true);
		}
	}
	return (false);
}

size_t	EventManager::getActiveEventsCount(void) const
{
	size_t	count;

	count = 0;
	for (const auto &event : _events)
	{
		if (event.isActive())
			count++;
	}
	return (count);
}

size_t	EventManager::getTotalEventsCount(void) const
{
	return (_events.size());
}

void	EventManager::clear(void)
{
	_events.clear();
}

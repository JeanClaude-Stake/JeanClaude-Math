#ifndef EVENTMANAGER_HPP
# define EVENTMANAGER_HPP

# include "Event.hpp"
# include <vector>
# include <memory>

class EventManager
{
	public:
		EventManager(void);
		~EventManager(void);

		void						addEvent(const Event &event);
		void						removeEvent(const std::string &eventId);
		void						updateEvent(const std::string &eventId,
									const Event &updatedEvent);

		Event*						getEvent(const std::string &eventId);
		const Event*				getEvent(const std::string &eventId) const;
		std::vector<Event>&			getAllEvents(void);
		const std::vector<Event>&	getAllEvents(void) const;
		std::vector<Event>			getActiveEvents(void) const;

		void						activateEvent(const std::string &eventId);
		void						deactivateEvent(const std::string &eventId);
		void						toggleEvent(const std::string &eventId);

		std::vector<Event>			getTriggeredEvents(int gameCount,
									const std::string &day,
									const std::string &time) const;

		std::string					toJSON(void) const;
		bool						hasConflicts(void) const;

		size_t						getActiveEventsCount(void) const;
		size_t						getTotalEventsCount(void) const;
		void						clear(void);

	private:
		std::vector<Event>			_events;

		int							findEventIndex(const std::string &eventId) const;
};

#endif

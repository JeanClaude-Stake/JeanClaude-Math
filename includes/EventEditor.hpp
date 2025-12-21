#ifndef EVENTEDITOR_HPP
# define EVENTEDITOR_HPP

# include "EventManager.hpp"
# include <GLFW/glfw3.h>
# include <string>

struct EventEditState
{
	char		id[64];
	char		name[64];
	char		description[256];
	bool		active;
	int			triggerType;
	bool		days[7];
	char		startTime[6];
	char		endTime[6];
	int			everyNGames;
	float		probability;
	float		rtpBoost;
	float		hitFrequencyBoost;
	bool		hasMaxMultiplier;
	int			maxMultiplierOverride;
	bool		enableFreeSpins;
	int			freeSpinsCount;
	float		freeSpinsMultiplier;
	bool		enableProgressiveJackpot;
	float		jackpotSeedValue;
	float		jackpotContribution;

	void		reset(void);
	void		loadFromEvent(const Event &event);
	Event		toEvent(void) const;
};

class EventEditor
{
	public:
		EventEditor(void);
		~EventEditor(void);

		void				render(EventManager &eventManager);
		bool				isOpen(void) const;
		void				open(void);
		void				close(void);

	private:
		bool				_open;
		int					_selectedEventIndex;
		EventEditState		_editState;
		bool				_showNewEventPopup;
		bool				_showDeleteConfirm;
		char				_newEventId[64];
		char				_newEventName[64];

		void				renderEventsList(EventManager &eventManager);
		void				renderEventDetails(EventManager &eventManager);
		void				renderTriggerConfig(void);
		void				renderModifiersConfig(void);
		void				renderPreviewPanel(const EventManager &eventManager);
		void				renderNewEventPopup(EventManager &eventManager);
		void				renderDeleteConfirmPopup(EventManager &eventManager);
		void				renderActions(EventManager &eventManager);

		double				estimateRTPWithEvents(const EventManager &eventManager) const;
		double				estimateActivationRate(void) const;
};

#endif

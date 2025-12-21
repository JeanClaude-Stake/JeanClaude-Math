#ifndef EVENT_HPP
# define EVENT_HPP

# include <string>
# include <vector>
# include <optional>
# include <cstdint>

enum class TriggerType
{
	TIME,
	GAME_COUNT,
	RANDOM
};

struct EventTrigger
{
	TriggerType					type;
	std::vector<std::string>	daysOfWeek;
	std::string					startTime;
	std::string					endTime;
	int							everyNGames;
	double						probability;

	EventTrigger(void);
};

struct EventModifiers
{
	double						rtpBoost;
	double						hitFrequencyBoost;
	std::optional<int>			maxMultiplierOverride;
	bool						enableFreeSpins;
	int							freeSpinsCount;
	double						freeSpinsMultiplier;
	bool						enableProgressiveJackpot;
	double						jackpotSeedValue;
	double						jackpotContribution;

	EventModifiers(void);
};

class Event
{
	public:
		Event(void);
		Event(const std::string &id, const std::string &name);
		~Event(void);

		std::string				getId(void) const;
		std::string				getName(void) const;
		std::string				getDescription(void) const;
		bool					isActive(void) const;
		const EventTrigger&		getTrigger(void) const;
		const EventModifiers&	getModifiers(void) const;

		void					setId(const std::string &id);
		void					setName(const std::string &name);
		void					setDescription(const std::string &description);
		void					setActive(bool active);
		void					setTrigger(const EventTrigger &trigger);
		void					setModifiers(const EventModifiers &modifiers);

		bool					shouldTrigger(int gameCount,
								const std::string &day,
								const std::string &time) const;
		std::string				toJSON(void) const;
		bool					isValid(void) const;

	private:
		std::string				_id;
		std::string				_name;
		std::string				_description;
		bool					_active;
		EventTrigger			_trigger;
		EventModifiers			_modifiers;

		bool					checkTimeTrigger(const std::string &day,
								const std::string &time) const;
		bool					checkGameCountTrigger(int gameCount) const;
		bool					checkRandomTrigger(void) const;
		int						parseTime(const std::string &time) const;
};

#endif

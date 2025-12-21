#include "EventEditor.hpp"
#include "imgui.h"
#include <cstring>
#include <cstdio>

static const char	*g_dayNames[7] = {
	"Monday", "Tuesday", "Wednesday", "Thursday",
	"Friday", "Saturday", "Sunday"
};

void	EventEditState::reset(void)
{
	memset(id, 0, sizeof(id));
	memset(name, 0, sizeof(name));
	memset(description, 0, sizeof(description));
	active = true;
	triggerType = 2;
	for (int i = 0; i < 7; i++)
		days[i] = false;
	strcpy(startTime, "00:00");
	strcpy(endTime, "23:59");
	everyNGames = 1000;
	probability = 0.05f;
	rtpBoost = 1.0f;
	hitFrequencyBoost = 1.0f;
	hasMaxMultiplier = false;
	maxMultiplierOverride = 100;
	enableFreeSpins = false;
	freeSpinsCount = 5;
	freeSpinsMultiplier = 2.0f;
	enableProgressiveJackpot = false;
	jackpotSeedValue = 1000.0f;
	jackpotContribution = 0.01f;
}

void	EventEditState::loadFromEvent(const Event &event)
{
	const EventTrigger		&t = event.getTrigger();
	const EventModifiers	&m = event.getModifiers();

	strncpy(id, event.getId().c_str(), sizeof(id) - 1);
	strncpy(name, event.getName().c_str(), sizeof(name) - 1);
	strncpy(description, event.getDescription().c_str(), sizeof(description) - 1);
	active = event.isActive();
	triggerType = static_cast<int>(t.type);
	for (int i = 0; i < 7; i++)
		days[i] = false;
	for (const auto &day : t.daysOfWeek)
	{
		for (int i = 0; i < 7; i++)
		{
			if (day == g_dayNames[i])
				days[i] = true;
		}
	}
	strncpy(startTime, t.startTime.c_str(), sizeof(startTime) - 1);
	strncpy(endTime, t.endTime.c_str(), sizeof(endTime) - 1);
	everyNGames = t.everyNGames;
	probability = static_cast<float>(t.probability);
	rtpBoost = static_cast<float>(m.rtpBoost);
	hitFrequencyBoost = static_cast<float>(m.hitFrequencyBoost);
	hasMaxMultiplier = m.maxMultiplierOverride.has_value();
	if (hasMaxMultiplier)
		maxMultiplierOverride = *m.maxMultiplierOverride;
	else
		maxMultiplierOverride = 100;
	enableFreeSpins = m.enableFreeSpins;
	freeSpinsCount = m.freeSpinsCount;
	freeSpinsMultiplier = static_cast<float>(m.freeSpinsMultiplier);
	enableProgressiveJackpot = m.enableProgressiveJackpot;
	jackpotSeedValue = static_cast<float>(m.jackpotSeedValue);
	jackpotContribution = static_cast<float>(m.jackpotContribution);
}

Event	EventEditState::toEvent(void) const
{
	Event			event(id, name);
	EventTrigger	trigger;
	EventModifiers	modifiers;

	event.setDescription(description);
	event.setActive(active);
	trigger.type = static_cast<TriggerType>(triggerType);
	for (int i = 0; i < 7; i++)
	{
		if (days[i])
			trigger.daysOfWeek.push_back(g_dayNames[i]);
	}
	trigger.startTime = startTime;
	trigger.endTime = endTime;
	trigger.everyNGames = everyNGames;
	trigger.probability = probability;
	event.setTrigger(trigger);
	modifiers.rtpBoost = rtpBoost;
	modifiers.hitFrequencyBoost = hitFrequencyBoost;
	if (hasMaxMultiplier)
		modifiers.maxMultiplierOverride = maxMultiplierOverride;
	modifiers.enableFreeSpins = enableFreeSpins;
	modifiers.freeSpinsCount = freeSpinsCount;
	modifiers.freeSpinsMultiplier = freeSpinsMultiplier;
	modifiers.enableProgressiveJackpot = enableProgressiveJackpot;
	modifiers.jackpotSeedValue = jackpotSeedValue;
	modifiers.jackpotContribution = jackpotContribution;
	event.setModifiers(modifiers);
	return (event);
}

EventEditor::EventEditor(void)
	: _open(false), _selectedEventIndex(-1), _showNewEventPopup(false),
	  _showDeleteConfirm(false)
{
	_editState.reset();
	memset(_newEventId, 0, sizeof(_newEventId));
	memset(_newEventName, 0, sizeof(_newEventName));
}

EventEditor::~EventEditor(void)
{
}

bool	EventEditor::isOpen(void) const
{
	return (_open);
}

void	EventEditor::open(void)
{
	_open = true;
}

void	EventEditor::close(void)
{
	_open = false;
}

void	EventEditor::render(EventManager &eventManager)
{
	if (!_open)
		return ;
	ImGui::SetNextWindowSize(ImVec2(700, 600), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Event Editor", &_open))
	{
		ImGui::Columns(2, "EventColumns", true);
		ImGui::SetColumnWidth(0, 200);
		renderEventsList(eventManager);
		ImGui::NextColumn();
		if (_selectedEventIndex >= 0)
			renderEventDetails(eventManager);
		else
			ImGui::TextDisabled("Select an event to edit");
		ImGui::Columns(1);
		ImGui::Separator();
		renderActions(eventManager);
	}
	ImGui::End();
	renderNewEventPopup(eventManager);
	renderDeleteConfirmPopup(eventManager);
}

void	EventEditor::renderEventsList(EventManager &eventManager)
{
	std::vector<Event>	&events = eventManager.getAllEvents();

	ImGui::Text("Events (%zu)", events.size());
	ImGui::Separator();
	for (size_t i = 0; i < events.size(); i++)
	{
		ImGui::PushID(static_cast<int>(i));
		bool	isSelected = (_selectedEventIndex == static_cast<int>(i));
		char	label[128];
		snprintf(label, sizeof(label), "%s %s###event",
			events[i].isActive() ? "[ON]" : "[OFF]",
			events[i].getName().c_str());
		if (ImGui::Selectable(label, isSelected))
		{
			_selectedEventIndex = static_cast<int>(i);
			_editState.loadFromEvent(events[i]);
		}
		ImGui::PopID();
	}
	ImGui::Spacing();
	if (ImGui::Button("+ New Event", ImVec2(-1, 0)))
	{
		_showNewEventPopup = true;
		memset(_newEventId, 0, sizeof(_newEventId));
		memset(_newEventName, 0, sizeof(_newEventName));
	}
}

void	EventEditor::renderEventDetails(EventManager &eventManager)
{
	(void)eventManager;
	ImGui::Text("Event Details");
	ImGui::Separator();
	ImGui::InputText("ID", _editState.id, sizeof(_editState.id));
	ImGui::InputText("Name", _editState.name, sizeof(_editState.name));
	ImGui::InputTextMultiline("Description", _editState.description,
		sizeof(_editState.description), ImVec2(-1, 50));
	ImGui::Checkbox("Active", &_editState.active);
	ImGui::Spacing();
	renderTriggerConfig();
	ImGui::Spacing();
	renderModifiersConfig();
	ImGui::Spacing();
	renderPreviewPanel(eventManager);
}

void	EventEditor::renderTriggerConfig(void)
{
	const char	*triggerTypes[] = {"Time", "Game Count", "Random"};

	if (ImGui::CollapsingHeader("Trigger", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Indent();
		ImGui::Combo("Type", &_editState.triggerType, triggerTypes, 3);
		if (_editState.triggerType == 0)
		{
			ImGui::Text("Days of Week:");
			ImGui::Indent();
			for (int i = 0; i < 7; i++)
			{
				if (i > 0 && i % 4 != 0)
					ImGui::SameLine();
				ImGui::Checkbox(g_dayNames[i], &_editState.days[i]);
			}
			ImGui::Unindent();
			ImGui::SetNextItemWidth(80);
			ImGui::InputText("Start", _editState.startTime,
				sizeof(_editState.startTime));
			ImGui::SameLine();
			ImGui::SetNextItemWidth(80);
			ImGui::InputText("End", _editState.endTime,
				sizeof(_editState.endTime));
		}
		else if (_editState.triggerType == 1)
		{
			ImGui::InputInt("Every N Games", &_editState.everyNGames, 100, 1000);
			if (_editState.everyNGames < 1)
				_editState.everyNGames = 1;
		}
		else
		{
			ImGui::SliderFloat("Probability", &_editState.probability,
				0.0f, 1.0f, "%.3f");
			ImGui::Text("(%.1f%% chance per round)", _editState.probability * 100.0f);
		}
		ImGui::Unindent();
	}
}

void	EventEditor::renderModifiersConfig(void)
{
	if (ImGui::CollapsingHeader("Modifiers", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Indent();
		ImGui::SliderFloat("RTP Boost", &_editState.rtpBoost, 0.9f, 1.2f, "%.3f");
		float	rtpPercent = (_editState.rtpBoost - 1.0f) * 100.0f;
		ImGui::SameLine();
		ImGui::Text("(%+.1f%%)", rtpPercent);
		ImGui::SliderFloat("Hit Freq Boost", &_editState.hitFrequencyBoost,
			0.5f, 2.0f, "%.2f");
		ImGui::Checkbox("Max Multiplier Override", &_editState.hasMaxMultiplier);
		if (_editState.hasMaxMultiplier)
		{
			ImGui::SameLine();
			ImGui::SetNextItemWidth(100);
			ImGui::InputInt("##maxmult", &_editState.maxMultiplierOverride);
		}
		ImGui::Separator();
		ImGui::Checkbox("Enable Free Spins", &_editState.enableFreeSpins);
		if (_editState.enableFreeSpins)
		{
			ImGui::Indent();
			ImGui::InputInt("Count", &_editState.freeSpinsCount);
			ImGui::SliderFloat("Multiplier", &_editState.freeSpinsMultiplier,
				1.0f, 10.0f, "%.1fx");
			ImGui::Unindent();
		}
		ImGui::Checkbox("Enable Progressive Jackpot",
			&_editState.enableProgressiveJackpot);
		if (_editState.enableProgressiveJackpot)
		{
			ImGui::Indent();
			ImGui::InputFloat("Seed Value", &_editState.jackpotSeedValue,
				100.0f, 1000.0f, "%.0f");
			ImGui::SliderFloat("Contribution", &_editState.jackpotContribution,
				0.0f, 0.1f, "%.3f");
			ImGui::Text("(%.1f%% of each bet)", _editState.jackpotContribution * 100.0f);
			ImGui::Unindent();
		}
		ImGui::Unindent();
	}
}

double	EventEditor::estimateActivationRate(void) const
{
	if (_editState.triggerType == 0)
	{
		int	activeDays = 0;
		for (int i = 0; i < 7; i++)
		{
			if (_editState.days[i])
				activeDays++;
		}
		return (activeDays / 7.0 * 0.5);
	}
	if (_editState.triggerType == 1)
		return (1.0 / _editState.everyNGames);
	return (_editState.probability);
}

double	EventEditor::estimateRTPWithEvents(const EventManager &eventManager) const
{
	double	baseRTP = 96.0;
	double	activationRate = estimateActivationRate();
	double	boost = _editState.rtpBoost;

	(void)eventManager;
	return (baseRTP * (1.0 - activationRate) + baseRTP * boost * activationRate);
}

void	EventEditor::renderPreviewPanel(const EventManager &eventManager)
{
	if (ImGui::CollapsingHeader("Preview Impact"))
	{
		ImGui::Indent();
		double	estimatedRTP = estimateRTPWithEvents(eventManager);
		double	activationRate = estimateActivationRate();
		ImGui::Text("Base RTP: 96.00%%");
		ImGui::Text("Estimated RTP with event: %.2f%%", estimatedRTP);
		ImGui::Text("Activation rate: %.2f%%", activationRate * 100.0);
		ImGui::Unindent();
	}
}

void	EventEditor::renderNewEventPopup(EventManager &eventManager)
{
	if (_showNewEventPopup)
		ImGui::OpenPopup("New Event");
	if (ImGui::BeginPopupModal("New Event", &_showNewEventPopup,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::InputText("Event ID", _newEventId, sizeof(_newEventId));
		ImGui::InputText("Event Name", _newEventName, sizeof(_newEventName));
		ImGui::Spacing();
		if (ImGui::Button("Create", ImVec2(120, 0)))
		{
			if (strlen(_newEventId) > 0 && strlen(_newEventName) > 0)
			{
				Event	newEvent(_newEventId, _newEventName);
				eventManager.addEvent(newEvent);
				_selectedEventIndex = static_cast<int>(
					eventManager.getTotalEventsCount() - 1);
				_editState.loadFromEvent(newEvent);
				_showNewEventPopup = false;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
			_showNewEventPopup = false;
		ImGui::EndPopup();
	}
}

void	EventEditor::renderDeleteConfirmPopup(EventManager &eventManager)
{
	if (_showDeleteConfirm)
		ImGui::OpenPopup("Delete Event?");
	if (ImGui::BeginPopupModal("Delete Event?", &_showDeleteConfirm,
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Are you sure you want to delete this event?");
		ImGui::Spacing();
		if (ImGui::Button("Delete", ImVec2(120, 0)))
		{
			std::vector<Event>	&events = eventManager.getAllEvents();
			if (_selectedEventIndex >= 0 &&
				_selectedEventIndex < static_cast<int>(events.size()))
			{
				eventManager.removeEvent(events[_selectedEventIndex].getId());
				_selectedEventIndex = -1;
				_editState.reset();
			}
			_showDeleteConfirm = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
			_showDeleteConfirm = false;
		ImGui::EndPopup();
	}
}

void	EventEditor::renderActions(EventManager &eventManager)
{
	std::vector<Event>	&events = eventManager.getAllEvents();

	if (_selectedEventIndex >= 0 &&
		_selectedEventIndex < static_cast<int>(events.size()))
	{
		if (ImGui::Button("Save Changes", ImVec2(120, 30)))
		{
			Event	updated = _editState.toEvent();
			eventManager.updateEvent(events[_selectedEventIndex].getId(),
				updated);
		}
		ImGui::SameLine();
		if (ImGui::Button("Delete", ImVec2(80, 30)))
			_showDeleteConfirm = true;
		ImGui::SameLine();
		if (ImGui::Button("Toggle Active", ImVec2(100, 30)))
		{
			eventManager.toggleEvent(events[_selectedEventIndex].getId());
			_editState.active = events[_selectedEventIndex].isActive();
		}
	}
	ImGui::SameLine(ImGui::GetWindowWidth() - 150);
	ImGui::Text("Events: %zu", eventManager.getTotalEventsCount());
}

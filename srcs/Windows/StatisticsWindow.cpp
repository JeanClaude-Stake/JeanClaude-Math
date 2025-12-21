#include "StatisticsWindow.hpp"
#include <imgui.h>

StatisticsWindow::StatisticsWindow(void)
	: _selectedModeIndex(0)
{
}

StatisticsWindow::~StatisticsWindow(void)
{
}

void	StatisticsWindow::render(ModeManager &modeManager)
{
	ImGui::SetNextWindowPos(ImVec2(420, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);

	ImGui::Begin("Statistics");

	if (modeManager.getModeCount() == 0)
	{
		ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
			"No mode avaible");
		ImGui::End();
		return ;
	}

	renderModeSelector(modeManager);
	ImGui::Separator();

	if (_selectedModeIndex >= 0
		&& _selectedModeIndex < (int)modeManager.getModeCount())
	{
		const ModeEntry	&mode = modeManager.getModes()[_selectedModeIndex];

		if (!mode.simulated || !mode.stats.calculated)
			renderNoDataWarning();
		else
			renderStatsTable(mode);
	}

	ImGui::End();
}

void	StatisticsWindow::renderModeSelector(ModeManager &modeManager)
{
	const std::vector<ModeEntry>	&modes = modeManager.getModes();

	if (_selectedModeIndex >= (int)modes.size())
		_selectedModeIndex = 0;

	ImGui::Text("Mode:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(300);
	if (ImGui::BeginCombo("##ModeSelect", modes[_selectedModeIndex].name))
	{
		for (size_t i = 0; i < modes.size(); i++)
		{
			bool	isSelected = (_selectedModeIndex == (int)i);

			if (ImGui::Selectable(modes[i].name, isSelected))
				_selectedModeIndex = i;
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
}

void	StatisticsWindow::renderStatsTable(const ModeEntry &mode)
{
	ImGui::Spacing();
	ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Basic metrics");
	ImGui::Separator();

	ImGui::Text("RTP:");
	ImGui::SameLine(200);
	ImGui::Text("%.2f%%", mode.rtp * 100.0);

	ImGui::Text("Average gain:");
	ImGui::SameLine(200);
	ImGui::Text("%.4f", mode.stats.meanPayout);

	ImGui::Text("Hit Frequency:");
	ImGui::SameLine(200);
	ImGui::Text("%.2f%%", mode.stats.hitFrequency);

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Distribution");
	ImGui::Separator();

	ImGui::Text("Variance:");
	ImGui::SameLine(200);
	ImGui::Text("%.4f", mode.stats.variance);

	ImGui::Text("Standard Deviation:");
	ImGui::SameLine(200);
	ImGui::Text("%.4f", mode.stats.stdDeviation);

	ImGui::Text("Volatility:");
	ImGui::SameLine(200);
	ImGui::Text("%.4f", mode.stats.volatility);

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Range of Gains");
	ImGui::Separator();

	ImGui::Text("Min Payout:");
	ImGui::SameLine(200);
	ImGui::Text("%.2fx", mode.stats.minPayout);

	ImGui::Text("Max Payout:");
	ImGui::SameLine(200);
	ImGui::Text("%.2fx", mode.stats.maxPayout);

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Sample");
	ImGui::Separator();

	ImGui::Text("Simulations:");
	ImGui::SameLine(200);
	ImGui::Text("%zu", mode.simCount);
}

void	StatisticsWindow::renderNoDataWarning(void)
{
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
		"No data avaible");
	ImGui::Spacing();
	ImGui::TextWrapped("Please run simulation to have data about this mode");
}

#include "StatisticsWindow.hpp"
#include <imgui.h>
#include <cmath>
#include <algorithm>

StatisticsWindow::StatisticsWindow(void)
	: _selectedModeIndex(0)
{
}

StatisticsWindow::~StatisticsWindow(void)
{
}

void	StatisticsWindow::render(ModeManager &modeManager)
{
	ImGui::SetNextWindowPos(ImVec2(470, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(450, 700), ImGuiCond_FirstUseEver);

	ImGui::Begin("Statistics & Distribution", NULL, ImGuiWindowFlags_NoCollapse);

	if (modeManager.getModeCount() == 0)
	{
		ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
			"No modes available");
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
		{
			renderStatsTable(mode);
			ImGui::Spacing();
			ImGui::Separator();
			renderDistributionChart(mode);
			ImGui::Separator();
			renderRTPBar(mode);
		}
	}

	ImGui::End();
}

void	StatisticsWindow::renderModeSelector(ModeManager &modeManager)
{
	const std::vector<ModeEntry>	&modes = modeManager.getModes();

	if (_selectedModeIndex >= (int)modes.size())
		_selectedModeIndex = 0;

	ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Select Mode");
	ImGui::Spacing();

	ImGui::Text("Mode:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(300);
	if (ImGui::BeginCombo("##ModeSelect", modes[_selectedModeIndex].name))
	{
		for (size_t i = 0; i < modes.size(); i++)
		{
			bool	isSelected = (_selectedModeIndex == (int)i);
			char	label[128];
			if (modes[i].simulated)
			{
				snprintf(label, sizeof(label), "%s (RTP: %.1f%%)",
					modes[i].name, modes[i].rtp * 100.0);
			}
			else
			{
				snprintf(label, sizeof(label), "%s (no data)",
					modes[i].name);
			}

			if (ImGui::Selectable(label, isSelected))
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
	ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Key Metrics");
	ImGui::Spacing();

	if (ImGui::BeginTable("StatsTable", 2, ImGuiTableFlags_Borders
		| ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn("Metric", ImGuiTableColumnFlags_WidthFixed, 180);
		ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("RTP");
		ImGui::TableSetColumnIndex(1);
		float	rtp = mode.rtp * 100.0f;
		ImVec4	rtpColor = rtp < 90 ? ImVec4(1, 0.3f, 0.3f, 1)
			: (rtp < 96 ? ImVec4(1, 0.8f, 0.2f, 1)
			: ImVec4(0.3f, 1, 0.3f, 1));
		ImGui::TextColored(rtpColor, "%.2f%%", rtp);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Hit Frequency");
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%.2f%%", mode.stats.hitFrequency);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Mean Payout");
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%.4fx", mode.stats.meanPayout);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Volatility");
		ImGui::TableSetColumnIndex(1);
		const char	*volLabel = mode.stats.volatility < 0.5 ? "Low"
			: (mode.stats.volatility < 1.5 ? "Medium" : "High");
		ImGui::Text("%.2f (%s)", mode.stats.volatility, volLabel);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Std Deviation");
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%.4f", mode.stats.stdDeviation);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Min / Max Payout");
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%.2fx / %.2fx", mode.stats.minPayout, mode.stats.maxPayout);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("Simulations");
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%zu", mode.simCount);

		ImGui::EndTable();
	}
}

void	StatisticsWindow::renderDistributionChart(const ModeEntry &mode)
{
	ImGui::Spacing();
	ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Multiplier Distribution");
	ImGui::Spacing();

	if (mode.multipliers.empty())
		return ;

	uint64_t	totalWeight = 0;
	float		maxProb = 0.0f;
	for (const auto &m : mode.multipliers)
		totalWeight += m.weight;

	std::vector<float>	probs;
	std::vector<float>	mults;
	for (const auto &m : mode.multipliers)
	{
		float	prob = totalWeight > 0
			? (m.weight * 100.0f / totalWeight) : 0.0f;
		probs.push_back(prob);
		mults.push_back(m.multiplier);
		if (prob > maxProb)
			maxProb = prob;
	}

	float	chartHeight = 120.0f;
	float	barWidth = std::min(40.0f,
		(ImGui::GetContentRegionAvail().x - 40) / mode.multipliers.size());

	ImVec2	pos = ImGui::GetCursorScreenPos();
	ImDrawList	*drawList = ImGui::GetWindowDrawList();

	for (size_t i = 0; i < probs.size(); i++)
	{
		float	barHeight = maxProb > 0
			? (probs[i] / maxProb) * chartHeight : 0.0f;
		float	x = pos.x + 20 + i * (barWidth + 5);
		float	y = pos.y + chartHeight;

		ImU32	color = ImGui::ColorConvertFloat4ToU32(
			ImVec4(0.3f, 0.6f, 0.9f, 0.8f));
		if (mults[i] == 0.0f)
			color = ImGui::ColorConvertFloat4ToU32(
				ImVec4(0.5f, 0.5f, 0.5f, 0.8f));
		else if (mults[i] >= 10.0f)
			color = ImGui::ColorConvertFloat4ToU32(
				ImVec4(0.9f, 0.7f, 0.2f, 0.8f));

		drawList->AddRectFilled(
			ImVec2(x, y - barHeight),
			ImVec2(x + barWidth - 2, y),
			color);

		char	label[32];
		snprintf(label, sizeof(label), "%.1fx", mults[i]);
		ImVec2	textSize = ImGui::CalcTextSize(label);
		drawList->AddText(
			ImVec2(x + (barWidth - textSize.x) / 2, y + 2),
			IM_COL32(200, 200, 200, 255), label);

		snprintf(label, sizeof(label), "%.1f%%", probs[i]);
		textSize = ImGui::CalcTextSize(label);
		drawList->AddText(
			ImVec2(x + (barWidth - textSize.x) / 2, y - barHeight - 15),
			IM_COL32(255, 255, 255, 255), label);
	}

	ImGui::Dummy(ImVec2(0, chartHeight + 25));
}

void	StatisticsWindow::renderRTPBar(const ModeEntry &mode)
{
	ImGui::Spacing();
	ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "RTP Indicator");
	ImGui::Spacing();

	float	rtp = mode.rtp * 100.0f;
	float	barWidth = ImGui::GetContentRegionAvail().x - 20;
	float	barHeight = 25.0f;

	ImVec2	pos = ImGui::GetCursorScreenPos();
	ImDrawList	*drawList = ImGui::GetWindowDrawList();

	drawList->AddRectFilled(
		pos,
		ImVec2(pos.x + barWidth, pos.y + barHeight),
		IM_COL32(50, 50, 50, 255));

	float	markers[] = {85, 90, 95, 100, 105};
	for (float mark : markers)
	{
		float	x = pos.x + ((mark - 80) / 30.0f) * barWidth;
		if (x > pos.x && x < pos.x + barWidth)
		{
			drawList->AddLine(
				ImVec2(x, pos.y),
				ImVec2(x, pos.y + barHeight),
				IM_COL32(100, 100, 100, 255));
			char	label[16];
			snprintf(label, sizeof(label), "%.0f%%", mark);
			drawList->AddText(ImVec2(x - 10, pos.y + barHeight + 2),
				IM_COL32(150, 150, 150, 255), label);
		}
	}

	float	rtpX = pos.x + ((rtp - 80) / 30.0f) * barWidth;
	rtpX = std::max(pos.x, std::min(pos.x + barWidth - 10, rtpX));

	ImU32	rtpColor = rtp < 90 ? IM_COL32(255, 80, 80, 255)
		: (rtp < 96 ? IM_COL32(255, 200, 50, 255)
		: IM_COL32(80, 255, 80, 255));

	drawList->AddTriangleFilled(
		ImVec2(rtpX, pos.y - 5),
		ImVec2(rtpX - 8, pos.y - 15),
		ImVec2(rtpX + 8, pos.y - 15),
		rtpColor);

	char	rtpLabel[32];
	snprintf(rtpLabel, sizeof(rtpLabel), "%.2f%%", rtp);
	ImVec2	textSize = ImGui::CalcTextSize(rtpLabel);
	drawList->AddText(ImVec2(rtpX - textSize.x / 2, pos.y - 32),
		rtpColor, rtpLabel);

	ImGui::Dummy(ImVec2(0, barHeight + 25));
}

void	StatisticsWindow::renderNoDataWarning(void)
{
	ImGui::Spacing();
	ImGui::Spacing();

	ImVec2	windowSize = ImGui::GetContentRegionAvail();
	ImVec2	textSize = ImGui::CalcTextSize("No simulation data");

	ImGui::SetCursorPosX((windowSize.x - textSize.x) / 2);
	ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "No simulation data");

	ImGui::Spacing();
	textSize = ImGui::CalcTextSize("Run simulations to see statistics");
	ImGui::SetCursorPosX((windowSize.x - textSize.x) / 2);
	ImGui::TextDisabled("Run simulations to see statistics");
}

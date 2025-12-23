#include "ModeEditor.hpp"
#include "imgui.h"
#include <cstring>
#include <cmath>
#include <algorithm>

ModeEditor::ModeEditor(void)
	: _numSimulations(100000), _exported(false), _isSimulating(false)
{
	strncpy(_outputDir, "output", sizeof(_outputDir));
}

ModeEditor::~ModeEditor(void)
{
}

void	ModeEditor::render(ModeManager &modeManager, GLFWwindow *window)
{
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(450, 700), ImGuiCond_FirstUseEver);

	ImGuiWindowFlags	flags = ImGuiWindowFlags_NoCollapse;
	ImGui::Begin("JeanClaude Math - Stake Engine Generator", NULL, flags);

	ImGui::SameLine(ImGui::GetWindowWidth() - 35);
	if (ImGui::Button("X", ImVec2(25, 25)))
		glfwSetWindowShouldClose(window, true);

	renderHeader();
	ImGui::Separator();
	renderSettings();
	ImGui::Separator();
	renderModesList(modeManager);
	ImGui::Separator();
	renderActions(modeManager);
	renderStatus();

	ImGui::End();
}

void	ModeEditor::renderHeader(void)
{
	ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f),
		"Math File Generator for Stake Engine");
	ImGui::TextDisabled("Generate .jsonl.zst + CSV files");
	ImGui::Spacing();
}

void	ModeEditor::renderModePanel(ModeEntry &mode, int index)
{
	ImGui::PushID(index);

	float	rtpPercent = mode.rtp * 100.0f;
	ImVec4	rtpColor;
	if (rtpPercent < 90.0f)
		rtpColor = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
	else if (rtpPercent < 96.0f)
		rtpColor = ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
	else
		rtpColor = ImVec4(0.3f, 1.0f, 0.3f, 1.0f);

	char	header[128];
	snprintf(header, sizeof(header), "%s###mode%d", mode.name, index);

	bool	isOpen = ImGui::CollapsingHeader(header,
		ImGuiTreeNodeFlags_DefaultOpen);

	ImGui::SameLine(ImGui::GetWindowWidth() - 120);
	if (mode.simulated)
	{
		ImGui::TextColored(rtpColor, "RTP: %.2f%%", rtpPercent);
	}
	else
	{
		ImGui::TextDisabled("RTP: --");
	}

	if (isOpen)
	{
		ImGui::Indent();

		ImGui::SetNextItemWidth(200);
		ImGui::InputText("Name", mode.name, sizeof(mode.name));
		ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		ImGui::InputFloat("Cost", &mode.cost, 0.0f, 0.0f, "%.1f");

		ImGui::Spacing();
		renderMultipliersTable(mode);

		ImGui::Unindent();
	}
	ImGui::PopID();
}

void	ModeEditor::renderMultipliersTable(ModeEntry &mode)
{
	ImGui::Text("Mult");
	ImGui::SameLine(90);
	ImGui::Text("Weight");
	ImGui::SameLine(170);
	ImGui::Text("Prob");

	uint64_t	totalWeight = 0;
	for (const auto &m : mode.multipliers)
		totalWeight += m.weight;
	if (totalWeight == 0)
		totalWeight = 1;

	size_t	toDelete = (size_t)-1;

	for (size_t i = 0; i < mode.multipliers.size(); i++)
	{
		ImGui::PushID(static_cast<int>(i));

		float	currentProb = (mode.multipliers[i].weight * 100.0f) / totalWeight;

		ImGui::SetNextItemWidth(70);
		ImGui::DragFloat("##m", &mode.multipliers[i].multiplier,
			0.1f, 0.0f, 1000.0f, "%.2fx");

		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		int	oldWeight = mode.multipliers[i].weight;
		if (ImGui::DragInt("##w", &mode.multipliers[i].weight, 1.0f, 1, 1000000))
		{
			if (mode.multipliers[i].weight < 1)
				mode.multipliers[i].weight = 1;
		}

		ImGui::SameLine();
		ImGui::SetNextItemWidth(70);
		float	newProb = currentProb;
		if (ImGui::DragFloat("##p", &newProb, 0.1f, 0.01f, 99.99f, "%.2f%%"))
		{
			float	otherWeightsTotal = totalWeight - oldWeight;
			if (otherWeightsTotal < 1)
				otherWeightsTotal = 1;
			if (newProb >= 99.99f)
				newProb = 99.99f;
			if (newProb <= 0.01f)
				newProb = 0.01f;
			float	newWeight = (newProb * otherWeightsTotal) / (100.0f - newProb);
			mode.multipliers[i].weight = static_cast<int>(newWeight + 0.5f);
			if (mode.multipliers[i].weight < 1)
				mode.multipliers[i].weight = 1;
		}

		ImGui::SameLine();
		if (ImGui::SmallButton("X") && mode.multipliers.size() > 1)
			toDelete = i;

		ImGui::PopID();
	}

	if (toDelete != (size_t)-1)
		mode.multipliers.erase(mode.multipliers.begin() + toDelete);

	ImGui::Spacing();
	if (ImGui::Button("+ Add Multiplier"))
		mode.multipliers.push_back({0.0f, 100});
}

void	ModeEditor::renderSettings(void)
{
	ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Settings");
	ImGui::Spacing();

	ImGui::Text("Simulations:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(150);
	ImGui::InputInt("##sims", &_numSimulations, 10000, 100000);
	if (_numSimulations < 1000)
		_numSimulations = 1000;
	if (_numSimulations > 10000000)
		_numSimulations = 10000000;

	ImGui::SameLine();
	ImGui::TextDisabled("(1K - 10M)");

	ImGui::Text("Output:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(200);
	ImGui::InputText("##outdir", _outputDir, sizeof(_outputDir));
	ImGui::Spacing();
}

void	ModeEditor::renderModesList(ModeManager &modeManager)
{
	std::vector<ModeEntry>	&modes = modeManager.getModes();

	ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f),
		"Game Modes (%zu)", modeManager.getModeCount());
	ImGui::Spacing();

	for (size_t i = 0; i < modes.size(); i++)
		renderModePanel(modes[i], static_cast<int>(i));

	ImGui::Spacing();
	if (ImGui::Button("+ Add Mode", ImVec2(150, 25)))
		modeManager.addDefaultMode();
	ImGui::SameLine();
	if (ImGui::Button("Remove Last", ImVec2(150, 25))
		&& modeManager.getModeCount() > 0)
		modeManager.removeLastMode();

	ImGui::Spacing();
}

void	ModeEditor::renderActions(ModeManager &modeManager)
{
	ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Actions");
	ImGui::Spacing();

	ImVec4	simBtnColor = ImVec4(0.2f, 0.5f, 0.8f, 1.0f);
	ImVec4	expBtnColor = ImVec4(0.2f, 0.7f, 0.3f, 1.0f);

	ImGui::PushStyleColor(ImGuiCol_Button, simBtnColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
		ImVec4(0.3f, 0.6f, 0.9f, 1.0f));
	if (ImGui::Button("Run Simulations", ImVec2(200, 35)))
	{
		modeManager.runAllSimulations(_numSimulations);
		_statusMsg = "Simulations completed! "
			+ std::to_string(_numSimulations) + " per mode.";
		_exported = false;
	}
	ImGui::PopStyleColor(2);

	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, expBtnColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
		ImVec4(0.3f, 0.8f, 0.4f, 1.0f));
	if (ImGui::Button("Export to Stake Engine", ImVec2(200, 35)))
	{
		if (modeManager.exportFiles(_outputDir))
		{
			_statusMsg = "Exported to " + std::string(_outputDir) + "/";
			_exported = true;
		}
		else
		{
			_statusMsg = "Error: Run simulations first!";
		}
	}
	ImGui::PopStyleColor(2);

	if (_exported)
	{
		ImGui::Spacing();
		renderExportPreview(modeManager);
	}
}

void	ModeEditor::renderExportPreview(const ModeManager &modeManager)
{
	ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.5f, 1.0f), "Exported Files:");
	ImGui::Indent();

	ImGui::BulletText("index.json");
	for (const auto &mode : modeManager.getModes())
	{
		ImGui::BulletText("books_%s.jsonl.zst", mode.name);
		ImGui::BulletText("lookUpTable_%s_0.csv", mode.name);
	}

	ImGui::Unindent();
}

void	ModeEditor::renderStatus(void)
{
	if (!_statusMsg.empty())
	{
		ImGui::Spacing();
		ImGui::Separator();
		ImVec4	color = _exported
			? ImVec4(0.3f, 1.0f, 0.5f, 1.0f)
			: ImVec4(0.0f, 1.0f, 0.5f, 1.0f);
		ImGui::TextColored(color, "%s", _statusMsg.c_str());
	}
}

int	ModeEditor::getNumSimulations(void) const
{
	return (_numSimulations);
}

void	ModeEditor::setNumSimulations(int value)
{
	_numSimulations = value;
}

const char*	ModeEditor::getOutputDir(void) const
{
	return (_outputDir);
}

void	ModeEditor::setOutputDir(const char *dir)
{
	strncpy(_outputDir, dir, sizeof(_outputDir));
}

void	ModeEditor::setStatusMessage(const std::string &msg)
{
	_statusMsg = msg;
}

const std::string&	ModeEditor::getStatusMessage(void) const
{
	return (_statusMsg);
}

bool	ModeEditor::hasExported(void) const
{
	return (_exported);
}

void	ModeEditor::setExported(bool exported)
{
	_exported = exported;
}

#include "ModeEditor.hpp"
#include "imgui.h"
#include <cstring>

ModeEditor::ModeEditor(void)
	: _numSimulations(100000), _exported(false)
{
	strncpy(_outputDir, "output", sizeof(_outputDir));
}

ModeEditor::~ModeEditor(void)
{
}

void	ModeEditor::render(ModeManager &modeManager, GLFWwindow *window)
{
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);

	ImGui::Begin("JeanClaude Math");

	// Bouton de fermeture en haut à droite
	ImGui::SameLine(ImGui::GetWindowWidth() - 35);
	if (ImGui::Button("X", ImVec2(25, 25)))
		glfwSetWindowShouldClose(window, true);

	renderSettings();
	renderModesList(modeManager);
	renderActions(modeManager);
	renderStatus();

	ImGui::End();
}

void	ModeEditor::renderModePanel(ModeEntry &mode, int index)
{
	ImGui::PushID(index);

	char	header[128];
	snprintf(header, sizeof(header), "%s (RTP: %.2f%%)###mode%d",
		mode.name, mode.rtp * 100.0, index);

	if (ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Indent();
		ImGui::InputText("Name", mode.name, sizeof(mode.name));
		ImGui::InputFloat("Cost", &mode.cost, 1.0f, 10.0f, "%.1f");

		ImGui::Separator();
		ImGui::Text("Multipliers:");

		for (size_t i = 0; i < mode.multipliers.size(); i++)
		{
			ImGui::PushID(static_cast<int>(i));
			ImGui::SetNextItemWidth(100);
			ImGui::InputFloat("##mult", &mode.multipliers[i].multiplier,
				0.0f, 0.0f, "%.2fx");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(100);
			ImGui::InputInt("##weight", &mode.multipliers[i].weight);
			ImGui::SameLine();
			if (ImGui::Button("X") && mode.multipliers.size() > 1)
				mode.multipliers.erase(mode.multipliers.begin() + i);
			ImGui::PopID();
		}

		if (ImGui::Button("+ Add Multiplier"))
			mode.multipliers.push_back({0.0f, 100});

		ImGui::Unindent();
	}
	ImGui::PopID();
}

void	ModeEditor::renderSettings(void)
{
	ImGui::Text("Settings");
	ImGui::Separator();
	ImGui::InputInt("Simulations", &_numSimulations, 10000, 100000);
	ImGui::InputText("Output Dir", _outputDir, sizeof(_outputDir));
	ImGui::Spacing();
}

void	ModeEditor::renderModesList(ModeManager &modeManager)
{
	std::vector<ModeEntry>	&modes = modeManager.getModes();

	ImGui::Text("Game Modes (%zu)", modeManager.getModeCount());
	ImGui::Separator();

	for (size_t i = 0; i < modes.size(); i++)
		renderModePanel(modes[i], static_cast<int>(i));

	ImGui::Spacing();
	if (ImGui::Button("+ Add Mode"))
		modeManager.addDefaultMode();
	ImGui::SameLine();
	if (ImGui::Button("Remove Last") && modeManager.getModeCount() > 0)
		modeManager.removeLastMode();

	ImGui::Spacing();
	ImGui::Separator();
}

void	ModeEditor::renderActions(ModeManager &modeManager)
{
	if (ImGui::Button("Run Simulations", ImVec2(180, 30)))
	{
		modeManager.runAllSimulations(_numSimulations);
		_statusMsg = "Simulations completed!";
	}
	ImGui::SameLine();
	if (ImGui::Button("Export Files", ImVec2(180, 30)))
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
}

void	ModeEditor::renderStatus(void)
{
	if (!_statusMsg.empty())
	{
		ImGui::Spacing();
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "%s",
			_statusMsg.c_str());
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

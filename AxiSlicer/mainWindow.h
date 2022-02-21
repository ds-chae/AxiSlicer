#include "stdafx.h"

class mainWindow
{
public:
	mainWindow();
	void onTimer(int e);
	void updateSliceMode();
	void onOneAtATimeSwitch(int e);
	void OnPreferences(int e);
	void OnMachineSettings(int e);
	void OnDropFiles(char* files);
	void OnModelMRU(int e);
	void addToModelMRU(char* file);
	void OnProfileMRU(int e);
	void addToProfileMRU(char* file);
	void updateProfileToAllControls();
	void reloadSettingPanels();
	void updateMachineMenu();
	void OnLoadProfile(int e);
	void OnLoadProfileFromGcode(int e);
	void OnSaveProfile(int e);
	void OnResetProfile(int e);
	void OnSimpleSwitch(int e);
	void OnNormalSwitch(int e);
	void OnDefaultMarlinFirmware(int e);
	void OnCustomFirmware(int e);
	void OnFirstRunWizard(int e);
	void OnSelectMachine(int index);
	void OnBedLevelWizard(int e);
	void OnHeadOffsetWizard(int e);
	void OnExpertOpen(int e);
	void OnMinecraftImport(int e);
	void OnPIDDebugger(int e);
	void onCopyProfileClipboard(int e);
	void OnCheckForUpdate(int e);
	void OnClose(int e);
	void OnQuit(int e);
};

class normalSettingsPanel {
public:
	normalSettingsPanel(int parent, void* callback = NULL);
	void _addSettingsToPanels(char* category, int left, int right);
	void SizeLabelWidths(int left, int right);
	void OnSize(int e);
	void UpdateSize(int configPanel);
	void updateProfileToControls();
};

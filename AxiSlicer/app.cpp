#include "stdafx.h"
#include "app.h"
#include "mesh.h"
#include "profile.h"

CuraApp::CuraApp()
{
}

void CuraApp::MacOpenFile(char* path)
{
}


void CuraApp::afterSplashCallback()
{
		if (strcmp(profile.getMachineSetting("machine_type"), "unknown") == 0) {
				profile.loadPreferences(profile.getPreferencePath());
				CString s;
				s = profile.getDefaultProfilePath();
				profile.loadProfile((LPCTSTR)s);
		}
		if (strcmp(profile.getMachineSetting("machine_type"), "unknown") == 0) {
//			configWizard.configWizard();
		}
/*
		if profile.getPreference("check_for_updates") == "True":
			newVersion = version.checkForNewerVersion()
			if newVersion is not None:
				if self.splash is not None:
					self.splash.Show(False)
				if wx.MessageBox(_("A new version of Cura is available, would you like to download?"), _("New version available"), wx.YES_NO | wx.ICON_INFORMATION) == wx.YES:
					webbrowser.open(newVersion)
					return
*/
/*
		if profile.getMachineSetting("machine_name") == "":
			return
		self.mainWindow = mainWindow.mainWindow()
		if self.splash is not None:
			self.splash.Show(False)
		self.mainWindow.Show()
		self.mainWindow.OnDropFiles(self.loadFiles)
		setFullScreenCapable(self.mainWindow)
*/
}

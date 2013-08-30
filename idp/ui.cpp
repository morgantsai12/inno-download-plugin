#include "ui.h"
#include "timer.h"
#include "trace.h"

//HACK: to allow set parent window for InternetErrorDlg in Url class.
static HWND uiParentWindowHandle = NULL;

HWND uiParentWindow()
{
	return uiParentWindowHandle ? uiParentWindowHandle : GetDesktopWindow();
}

UI::UI()
{
	controls["TotalProgressBar"] = NULL;
	controls["FileProgressBar"]  = NULL;
	controls["TotalDownloaded"]	 = NULL;
	controls["FileDownloaded"]   = NULL;
	controls["FileName"]         = NULL;
	controls["Speed"]            = NULL;
	controls["Status"]           = NULL;
	controls["ElapsedTime"]      = NULL;
	controls["RemainingTime"]    = NULL;
	controls["NextButton"]		 = NULL;
	controls["BackButton"]       = NULL;
	controls["WizardForm"]		 = NULL;

	messages["KB/s"]                   = _T("KB/s");
	messages["%d of %d KB"]            = _T("%d of %d KB");
	messages["Initializing..."]        = _T("Initializing...");
	messages["Querying file sizes..."] = _T("Querying file sizes...");
	messages["Starting download..."]   = _T("Starting download...");
	messages["Connecting..."]          = _T("Connecting...");
	messages["Downloading..."]         = _T("Downloading...");
	messages["Done"]                   = _T("Done");
	messages["Error"]                  = _T("Error");
	messages["Cannot connect"]         = _T("Cannot connect");
	messages["Action cancelled"]       = _T("Action cancelled");

	allowContinue = false;
}

UI::~UI()
{
}

void UI::connectControl(tstring name, HWND handle)
{
	controls[toansi(name)] = handle;

	if(name.compare(_T("WizardForm")))
		uiParentWindowHandle = handle;
}

void UI::addMessage(tstring name, tstring message)
{
	if(message.length())
		messages[toansi(name)] = message;
}

void UI::setFileName(tstring filename)
{
	setLabelText(controls["FileName"], filename);
}

void UI::setProgressInfo(DWORDLONG totalSize, DWORDLONG totalDownloaded, DWORDLONG fileSize, DWORDLONG fileDownloaded)
{
	int filePercents  = (int)(100.0 / ((double)fileSize  / (double)fileDownloaded));
	int totalPercents = (int)(100.0 / ((double)totalSize / (double)totalDownloaded));

	setProgressBarPos(controls["TotalProgressBar"], totalPercents);
	setProgressBarPos(controls["FileProgressBar"],  filePercents);
}

void UI::setSpeedInfo(DWORD speed, DWORD remainingTime)
{
	setLabelText(controls["RemainingTime"], Timer::msecToStr(remainingTime, _T("%02u:%02u:%02u")));
	setLabelText(controls["Speed"],         itotstr((int)((double)speed / 1024.0 * 1000.0)) + _T(" ") + messages["KB/s"]);
}

void UI::setSizeTimeInfo(DWORDLONG totalSize, DWORDLONG totalDownloaded, DWORDLONG fileSize, DWORDLONG fileDownloaded, DWORD elapsedTime)
{
	setLabelText(controls["ElapsedTime"],     Timer::msecToStr(elapsedTime, _T("%02u:%02u:%02u")));
	setLabelText(controls["TotalDownloaded"], tstrprintf(messages["%d of %d KB"], (int)(totalDownloaded / 1024), (int)(totalSize / 1024)));
	setLabelText(controls["FileDownloaded"],  tstrprintf(messages["%d of %d KB"], (int)(fileDownloaded  / 1024), (int)(fileSize  / 1024)));
}

void UI::setStatus(tstring status)
{
	setLabelText(controls["Status"], status);
}

void UI::setMarquee(bool marquee, bool total)
{
	if(total)
		setProgressBarMarquee(controls["TotalProgressBar"], marquee);
	
	setProgressBarMarquee(controls["FileProgressBar"],  marquee);
}

void UI::setLabelText(HWND l, tstring text)
{
	if(l)
		SendMessage(l, WM_SETTEXT, 0, (LPARAM)text.c_str());
}

void UI::setProgressBarPos(HWND pb, int pos)
{
	if(pb)
		PostMessage(pb, PBM_SETPOS, (int)((65535.0 / 100.0) * pos), 0);
}

void UI::setProgressBarMarquee(HWND pb, bool marquee)
{
	if(!pb)
		return;

	LONG style = GetWindowLong(pb, GWL_STYLE);

	if(marquee)
	{
		style |= PBS_MARQUEE;
		SetWindowLong(pb, GWL_STYLE, style);
		SendMessage(pb, PBM_SETMARQUEE, (WPARAM)TRUE, 0);
	}
	else
	{
		style ^= PBS_MARQUEE;
		SendMessage(pb, PBM_SETMARQUEE, (WPARAM)FALSE, 0);
		SetWindowLong(pb, GWL_STYLE, style);
		RedrawWindow(pb, NULL, NULL, RDW_INVALIDATE | RDW_ERASENOW | RDW_UPDATENOW);
	}
}

int UI::messageBox(tstring text, tstring caption, DWORD type)
{
	return MessageBox(controls["WizardForm"], text.c_str(), caption.c_str(), type);
}

void UI::clickNextButton()
{
	if(controls["NextButton"])
	{
		EnableWindow(controls["NextButton"], TRUE);
		SendMessage(controls["WizardForm"], WM_COMMAND, MAKEWPARAM(0, BN_CLICKED), (LPARAM)controls["NextButton"]);
	}
}

void UI::lockButtons()
{ 
	if(controls["BackButton"])
		ShowWindow(controls["BackButton"], SW_HIDE);

	if(controls["NextButton"])
		EnableWindow(controls["NextButton"], FALSE);
}

void UI::unlockButtons()
{ 
	if(controls["BackButton"])
		ShowWindow(controls["BackButton"], SW_SHOW);

	if(controls["NextButton"])
		EnableWindow(controls["NextButton"], allowContinue);
}
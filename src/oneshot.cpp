#include "debugwriter.h"
#include "eventthread.h"
#include "oneshot.h"
#include "bitmap.h"
#include "font.h"

#include <SDL2/SDL.h>

#ifdef _WIN32
#define OS_W32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif
#include <windows.h>
#include <shlobj.h>
#include <security.h>
#include <SDL2/SDL_syswm.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#include <pwd.h>
#ifdef __linux__
#define OS_LINUX
#include "gnome-fun.h"
#include "xdg-user-dir-lookup.h"
#elif defined(__APPLE__)
#define OS_OSX
#include <dispatch/dispatch.h>
#endif
#else
#error "Unsupported platform"
#endif

#define DEF_SCREEN_W 640
#define DEF_SCREEN_H 480

#ifdef OS_W32
static std::string strFromWide(const wchar_t *from)
{
	if (!from)
		return {};
	int size = WideCharToMultiByte(CP_UTF8, 0, from, -1, nullptr, 0, nullptr, nullptr);
	if (size <= 1)
		return {};
	std::string result(size - 1, '\0');
	WideCharToMultiByte(CP_UTF8, 0, from, -1, &result[0], size, nullptr, nullptr);
	return result;
}
#endif // OS_W32
#ifdef OS_LINUX
struct linux_DialogData
{
	// Input
	int type;
	const char *body;
	const char *title;
	const char *btnYes;
	const char *btnNo;

	// Output
	bool result;
};

static int linux_dialog(void *rawData)
{
	linux_DialogData *data = reinterpret_cast<linux_DialogData *>(rawData);

	GtkMessageType gtktype;
	GtkButtonsType gtkbuttons = GTK_BUTTONS_OK;
	switch (data->type)
	{
		case Oneshot::MSG_INFO:
		default:
			gtktype = GTK_MESSAGE_INFO;
			break;
		case Oneshot::MSG_YESNO:
			gtktype = GTK_MESSAGE_QUESTION;
			gtkbuttons = GTK_BUTTONS_YES_NO;
			break;
		case Oneshot::MSG_WARN:
			gtktype = GTK_MESSAGE_WARNING;
			break;
		case Oneshot::MSG_ERR:
			gtktype = GTK_MESSAGE_ERROR;
			break;
	}

	GtkWidget *dialog = dynGtk.gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, gtktype, gtkbuttons, "%s", data->body);
	dynGtk.gtk_window_set_title(GTK_WINDOW(dialog), data->title);

	if (gtkbuttons == GTK_BUTTONS_YES_NO) {
		GtkWidget *btnYes = dynGtk.gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_YES);
		GtkWidget *btnNo = dynGtk.gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_NO);
		if (btnYes)
			dynGtk.gtk_button_set_label(GTK_BUTTON(btnYes), data->btnYes);
		if (btnNo)
			dynGtk.gtk_button_set_label(GTK_BUTTON(btnNo), data->btnNo);
	}

	int result = dynGtk.gtk_dialog_run(GTK_DIALOG(dialog));
	dynGtk.gtk_widget_destroy(dialog);

	data->result = (result == GTK_RESPONSE_OK || result == GTK_RESPONSE_YES);
	dynGtk.gtk_main_quit();
	return 0;
}
#endif // OS_LINUX

struct OneshotPrivate
{
	std::string os;
	std::string lang;
	std::string userName;
	std::string savePath;
	std::string docsPath;
	std::string gamePath;
	std::string journal;
	std::string txtYes;
	std::string txtNo;

	bool allowExit;
	bool exiting;

	bool windowPosChanged;
	int windowPosX;
	int windowPosY;

	bool obscuredCleared;
	std::vector<uint8_t> obscuredMap;

	SDL_Window *window;
	SDL_mutex *windowMutex;

#ifdef OS_LINUX
	bool gtkInit;
#endif

	OneshotPrivate()
		: allowExit(true),
		  exiting(false),
		  windowPosChanged(false),
		  windowPosX(0),
		  windowPosY(0),
		  obscuredCleared(false),
		  window(nullptr),
		  windowMutex(nullptr)
	{
		windowMutex = SDL_CreateMutex();
		if (windowMutex == nullptr)
			Debug () << "Unable to create window mutex:" << SDL_GetError();

#ifdef OS_LINUX
		gtkInit = false;
		initGnomeFunctions();
#endif
	}

	~OneshotPrivate()
	{
		if (windowMutex != nullptr) {
			SDL_DestroyMutex(windowMutex);
			windowMutex = nullptr;
		}
	}
};

Oneshot::Oneshot(RGSSThreadData &threadData) : threadData(threadData)
{
	p = new OneshotPrivate();

	p->window = threadData.window;
	p->savePath = threadData.config.commonDataPath.substr(0, threadData.config.commonDataPath.size() - 1);

	p->obscuredMap.resize(640 * 480, 255);
	obscuredDirty = true;

#ifdef OS_W32
	p->os = "windows";
#elif defined(OS_OSX)
	p->os = "macos";
#else
	p->os = "linux";
#endif

	// Get current locale
#ifdef OS_W32
	wchar_t wcBuf[19] = {L'\0'};
	wchar_t wcLang[9];
	wchar_t wcCountry[9];
	GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, wcLang, sizeof(wcLang) / sizeof(wchar_t));
	GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, wcCountry, sizeof(wcCountry) / sizeof(wchar_t));
	swprintf_s(wcBuf, sizeof(wcBuf) / sizeof(wchar_t), L"%ls_%ls", wcLang, wcCountry);
	p->lang = strFromWide(wcBuf);
#else
	const char *loc = SDL_getenv("LC_ALL");
	if (!loc || loc[0] == '\0')
		loc = SDL_getenv("LC_MESSAGES");
	if (!loc || loc[0] == '\0')
		loc = SDL_getenv("LANG");
	if (loc && loc[0] != '\0') {
		// Find first dot, copy language code
		int end = 0;
		for (; loc[end] != '\0' && loc[end] != '.'; ++end) {}
		p->lang = std::string(loc, end);
	} else {
		p->lang = "en";
	}
#endif

	// Get user name
#ifdef OS_W32
	ULONG size = 0;
	GetUserNameExW(NameDisplay, nullptr, &size);
	if (GetLastError() == ERROR_MORE_DATA && size > 0) {
		// Get the full (display) name
		std::wstring name(size, L'\0');
		GetUserNameExW(NameDisplay, &name[0], &size);
		p->userName = strFromWide(name.c_str());
	} else {
		// Get the login name
		DWORD size2 = 0;
		GetUserNameW(nullptr, &size2);
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER && size2 > 0) {
			std::wstring name(size2, L'\0');
			GetUserNameW(&name[0], &size2);
			p->userName = strFromWide(name.c_str());
		}
	}
#else
#ifdef OS_OSX
	uid_t uid = geteuid();
#else
	uid_t uid = getuid();
#endif
	struct passwd *pw = getpwuid(uid);
	if (pw != nullptr) {
		if (pw->pw_gecos && pw->pw_gecos[0] != '\0' && pw->pw_gecos[0] != ',') {
			// Get the full name from GECOS
			// Find first comma, copy only the full name field
			int end = 0;
			for (; pw->pw_gecos[end] && pw->pw_gecos[end] != ','; ++end) {}
			p->userName = std::string(pw->pw_gecos, end);
		} else if (pw->pw_name && pw->pw_gecos[0] != '\0') {
			// Get the login name
			p->userName = std::string(pw->pw_name);
		}
	}
	if (p->userName.empty()) {
		// Get the name from environment
		const char *userEnv = SDL_getenv("LOGNAME");
		if (!userEnv || userEnv[0] == '\0')
			userEnv = SDL_getenv("USER");
		if (userEnv && userEnv[0] != '\0')
			p->userName = std::string(userEnv);
	}
#endif

	// Get paths to Documents
#ifdef OS_W32
	wchar_t docPath[MAX_PATH];
	HRESULT hr = SHGetFolderPathW(nullptr, CSIDL_PERSONAL, nullptr, 0, docPath);
	if (SUCCEEDED(hr))
		p->docsPath = strFromWide(docPath);
	else
		p->docsPath = std::string(SDL_getenv("USERPROFILE")) + "\\Documents";
	p->gamePath = p->docsPath + "\\My Games";
	p->journal = "_______.exe";
#elif defined(OS_OSX)
	std::string path = std::string(SDL_getenv("HOME")) + "/Documents";
	p->docsPath = path.c_str();
	p->gamePath = path.c_str();
	p->journal = "_______.app";
#else
	char *path = xdg_user_dir_lookup("DOCUMENTS");
	p->docsPath = path;
	p->gamePath = path;
	p->journal = "_______";
#endif

	Debug() << "Game path    :" << p->gamePath;
	Debug() << "Docs path    :" << p->docsPath;
	Debug() << "User name    :" << p->userName;
	Debug() << "Language     :" << p->lang;

#ifdef OS_LINUX
	// Initialize Gtk for dialog and display stuff
	if (dynGtk.gtk_init_check != nullptr) {
		if (dynGtk.gtk_init_check(nullptr, nullptr))
			p->gtkInit = true;
	}

	// Get current Desktop Environment
	const char *currentDesktop = SDL_getenv("XDG_CURRENT_DESKTOP");
	if (currentDesktop == nullptr || currentDesktop[0] == '\0') {
		desktopEnv = "nope";
	} else {
		std::string de(currentDesktop);
		std::transform(de.begin(), de.end(), de.begin(), [](unsigned char c) { return std::tolower(c); });
		if (de.find("gnome") != std::string::npos || de.find("unity") != std::string::npos) {
			desktopEnv = "gnome";
		} else if (de.find("cinnamon") != std::string::npos) {
			desktopEnv = "cinnamon";
		} else if (de.find("mate") != std::string::npos) {
			desktopEnv = "mate";
		} else if (de.find("kde") != std::string::npos) {
			desktopEnv = "kde";
		} else if (de.find("xfce") != std::string::npos) {
			desktopEnv = "xfce";
		} else if (de.find("lxde") != std::string::npos) {
			desktopEnv = "lxde";
		} else if (de.find("deepin") != std::string::npos) {
			desktopEnv = "deepin";
		}
	}

	Debug() << "Desktop env  :" << desktopEnv;
#endif
}

Oneshot::~Oneshot()
{
	delete p;
}

void Oneshot::update()
{
	if (p->windowPosChanged) {
		p->windowPosChanged = false;

		// Map of unobscured pixels in this frame
		static std::vector<bool> obscuredFrame;
		obscuredFrame.resize(p->obscuredMap.size());
		std::fill(obscuredFrame.begin(), obscuredFrame.end(), true);

		SDL_Rect screenRect;
		SDL_LockMutex(p->windowMutex);
		screenRect.x = p->windowPosX;
		screenRect.y = p->windowPosY;
		SDL_UnlockMutex(p->windowMutex);
		screenRect.w = 640;
		screenRect.h = 480;

		// Update obscured map and texture for window portion offscreen
		for (int i = 0, max = SDL_GetNumVideoDisplays(); i < max; ++i)
		{
			SDL_Rect bounds;
			SDL_GetDisplayBounds(i, &bounds);

			// Get intersection of window and the screen
			SDL_Rect intersect;
			if (!SDL_IntersectRect(&screenRect, &bounds, &intersect))
				continue;
			intersect.x -= screenRect.x;
			intersect.y -= screenRect.y;

			// If it's entirely within the bounds of the screen,
			// we don't need to check out any other monitors
			if (intersect.x == 0 && intersect.y == 0 && intersect.w == 640 && intersect.h == 480)
				return;

			for (int y = intersect.y; y < intersect.y + intersect.h; ++y)
			{
				int start = y * 640 + intersect.x;
				std::fill(obscuredFrame.begin() + start, obscuredFrame.begin() + (start + intersect.w), false);
			}
		}

		// Update the obscured map, and return prematurely
		// if we don't have any changes to make to the texture
		bool needsUpdate = false;
		bool cleared = true;
		for (size_t i = 0; i < obscuredFrame.size(); ++i)
		{
			if (obscuredFrame[i]) {
				p->obscuredMap[i] = 0;
				needsUpdate = true;
			}
			if (p->obscuredMap[i] == 255)
				cleared = false;
		}
		p->obscuredCleared = cleared;
		if (!needsUpdate)
			return;

		obscuredDirty = true;
	}
}

void Oneshot::setYesNo(const char *yes, const char *no)
{
	p->txtYes = yes;
	p->txtNo = no;
}

void Oneshot::setWindowPos(int x, int y)
{
	SDL_LockMutex(p->windowMutex);
	p->windowPosX = x;
	p->windowPosY = y;
	p->windowPosChanged = true;
	SDL_UnlockMutex(p->windowMutex);
}

void Oneshot::setAllowExit(bool allowExit)
{
	if (p->allowExit == allowExit)
		return;
	p->allowExit = allowExit;
	if (p->allowExit)
		threadData.allowExit.set();
	else
		threadData.allowExit.clear();
}

void Oneshot::setExiting(bool exiting)
{
	if (p->exiting == exiting)
		return;
	p->exiting = exiting;
	if (p->exiting)
		threadData.exiting.set();
	else
		threadData.exiting.clear();
}

void Oneshot::resetObscured()
{
	std::fill(p->obscuredMap.begin(), p->obscuredMap.end(), 255);
	obscuredDirty = true;
	p->obscuredCleared = false;
}

bool Oneshot::msgbox(int type, const char *body, const char *title)
{
	if (!title)
		title = "";

#ifdef OS_LINUX
	if (p->gtkInit) {
		linux_DialogData data = {type, body, title, p->txtYes.c_str(), p->txtNo.c_str(), 0};
		dynGlib.g_idle_add(linux_dialog, &data);
		dynGtk.gtk_main();
		return data.result;
	}
#endif

	// Button data
	static const SDL_MessageBoxButtonData buttonOk = {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "OK"};
	static const SDL_MessageBoxButtonData buttonsOk[] = {buttonOk};
	SDL_MessageBoxButtonData buttonYes = {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, p->txtYes.c_str()};
	SDL_MessageBoxButtonData buttonNo = {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, p->txtNo.c_str()};
	SDL_MessageBoxButtonData buttonsYesNo[] = {buttonNo, buttonYes};

	// Message box data
	SDL_MessageBoxData data;
	data.window = NULL; // p->window;
	data.colorScheme = 0;
	data.title = title;
	data.message = body;

	switch (type)
	{
		case MSG_INFO:
		case MSG_YESNO:
		default:
			data.flags = SDL_MESSAGEBOX_INFORMATION;
			break;
		case MSG_WARN:
			data.flags = SDL_MESSAGEBOX_WARNING;
			break;
		case MSG_ERR:
			data.flags = SDL_MESSAGEBOX_WARNING;
			break;
	}

	switch (type)
	{
		case MSG_INFO:
		case MSG_WARN:
		case MSG_ERR:
		default:
			data.numbuttons = 1;
			data.buttons = buttonsOk;
			break;
		case MSG_YESNO:
			data.numbuttons = 2;
			data.buttons = buttonsYesNo;
			break;
	}

	int button;
#ifdef OS_OSX
	int *btn = &button;
	// Message boxes and UI changes must be performed from the main thread on macOS Mojave and above.
	// This block ensures the message box will show from the main thread.
	dispatch_sync(dispatch_get_main_queue(), ^{
		SDL_ShowMessageBox(&data, btn);
	});
#else
	SDL_ShowMessageBox(&data, &button);
#endif

	return button ? true : false;
}

std::string Oneshot::textinput(const char* prompt, int char_limit, const char* fontName)
{
	std::vector<std::string> *fontNames = new std::vector<std::string>();
	fontNames->push_back(fontName);
	fontNames->push_back("VL Gothic");
	Font *font = new Font(fontNames, 18);

	Bitmap *promptBmp = new Bitmap(DEF_SCREEN_W, DEF_SCREEN_H);
	promptBmp->setInitFont(font);
	promptBmp->drawText(0, 0, DEF_SCREEN_W, DEF_SCREEN_H, prompt, 1);

	Bitmap *inputBmp = new Bitmap(DEF_SCREEN_W, DEF_SCREEN_H);
	inputBmp->setInitFont(font);
	inputBmp->drawText(0, 0, DEF_SCREEN_W, DEF_SCREEN_H, "", 1);

	std::string inputTextPrev = std::string("");
	threadData.acceptingTextInput.set();
	threadData.inputTextLimit = char_limit;
	threadData.inputText.clear();
	SDL_StartTextInput();

	// Main loop
	while (threadData.acceptingTextInput) {
		if (inputTextPrev != threadData.inputText) {
			inputBmp->clear();
			inputBmp->drawText(DEF_SCREEN_W / 2, DEF_SCREEN_H / 2, DEF_SCREEN_W, DEF_SCREEN_H, threadData.inputText.c_str(), 1);
			inputTextPrev = threadData.inputText;
		}
	}

	// Disable text input
	SDL_StopTextInput();

	return threadData.inputText;
}

const std::string &Oneshot::os() const
{
	return p->os;
}

const std::string &Oneshot::lang() const
{
	return p->lang;
}

const std::string &Oneshot::userName() const
{
	return p->userName;
}

const std::string &Oneshot::savePath() const
{
	return p->savePath;
}

const std::string &Oneshot::docsPath() const
{
	return p->docsPath;
}

const std::string &Oneshot::gamePath() const
{
	return p->gamePath;
}

const std::string &Oneshot::journal() const
{
	return p->journal;
}

bool Oneshot::allowExit() const
{
	return p->allowExit;
}

bool Oneshot::exiting() const
{
	return p->exiting;
}

bool Oneshot::obscuredCleared() const
{
	return p->obscuredCleared;
}

const std::vector<uint8_t> &Oneshot::obscuredMap() const
{
	return p->obscuredMap;
}

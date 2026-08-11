#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include "etc.h"
#include "sharedstate.h"
#include "binding-util.h"
#include "binding-types.h"
#include "config.h"
#include "oneshot.h"
#include "debugwriter.h"

#ifdef _WIN32
	#include <windows.h>
	static WCHAR szStyle[8] = {0};
	static WCHAR szTile[8] = {0};
	static WCHAR szFile[MAX_PATH+1] = {0};
	static DWORD oldcolor = 0;
	static DWORD szStyleSize = sizeof(szStyle) - 1;
	static DWORD szTileSize = sizeof(szTile) - 1;
	static bool setStyle = false;
	static bool setTile = false;
	static bool isCached = false;
#else
	#ifdef __APPLE__
		#include "mac-desktop.h"
		static bool isCached = false;
	#else
		#include <unistd.h>
		#include <algorithm>
		#include <iostream>
		#include <string>
		#include <sstream>

		#include "gnome-fun.h"
		#include "xfconf-fun.h"

		struct XfceRGBA
		{
			double red;
			double green;
			double blue;
			double alpha;
		};

		static std::string desktop = "uninitialized";
		// GNOME settings
		static GSettings *bgsetting;
		static std::string defPictureURI, defPictureDarkURI, defPictureOptions, defPrimaryColor, defColorShading;
		// XFCE settings
		static XfconfChannel* bgchannel;
		static std::vector<std::string> monitorNames;
		static std::vector<std::string> propsImage, propsImageStyle, propsColor1, propsColorStyle;
		static std::vector<std::string> defImages;
		static std::vector<int> defImageStyles, defColorStyles;
		static std::vector<bool> defColorExists;
		static std::vector<XfceRGBA> defXfColors;
		// KDE settings
		static std::map<std::string, std::string> defPlugins, defPictures, defColors, defModes;
		static std::map<std::string, bool> defBlurs;
		// Fallback settings
		static std::string fallbackPath;
	#endif
#endif

#ifdef __linux__

	bool xfconfLibraryInit() {
		initXfconfFunctions();
		return (dynXfconf.xfconf_init != nullptr);
	}

	void xfconfLibraryShutdown() {
		if (desktop == "xfce" && dynXfconf.xfconf_shutdown != nullptr)
			dynXfconf.xfconf_shutdown();
	}

	bool tryGetGSettings(const char* schema_id, GSettings** outSetting) {
		GSettingsSchemaSource* schemaSource = dynGio.g_settings_schema_source_get_default();
		if (schemaSource == NULL) {
			return false;
		}
		GSettingsSchema* schema = dynGio.g_settings_schema_source_lookup(schemaSource, schema_id, true);
		if (schema == NULL) {
			return false;
		}
		*outSetting = dynGio.g_settings_new(schema_id);
		return true;
	}

	void desktopEnvironmentInit()
	{
		if (desktop != "uninitialized") {
			return;
		}
		desktop = shState->oneshot().desktopEnv;
		if (desktop == "cinnamon" || desktop == "gnome" || desktop == "mate" || desktop == "deepin") {
			bool gSettingSuccess = false;
			const char* picUriSettingKey = NULL;
			if (desktop == "cinnamon") {
				gSettingSuccess = tryGetGSettings("org.cinnamon.desktop.background", &bgsetting);
				picUriSettingKey = "picture-uri";
			} else if (desktop == "deepin") {
				gSettingSuccess = tryGetGSettings("com.deepin.wrap.gnome.desktop.background", &bgsetting);
				picUriSettingKey = "picture-uri";
			} else if (desktop == "gnome") {
				gSettingSuccess = tryGetGSettings("org.gnome.desktop.background", &bgsetting);
				picUriSettingKey = "picture-uri";
			} else {
				gSettingSuccess = tryGetGSettings("org.mate.background", &bgsetting);
				picUriSettingKey = "picture-filename";
			}
			if (gSettingSuccess) {
				defPictureURI = dynGio.g_settings_get_string(bgsetting, picUriSettingKey);
				if (desktop == "gnome")
					defPictureDarkURI = dynGio.g_settings_get_string(bgsetting, "picture-uri-dark");
				defPictureOptions = dynGio.g_settings_get_string(bgsetting, "picture-options");
				defPrimaryColor = dynGio.g_settings_get_string(bgsetting, "primary-color");
				defColorShading = dynGio.g_settings_get_string(bgsetting, "color-shading-type");
			} else {
				desktop = "no_desktop";
			}
		} else if (desktop == "xfce") {
			GError *xferror = NULL;
			if (xfconfLibraryInit() && dynXfconf.xfconf_init(&xferror)) {
				// Get all monitors from current display
				GdkDisplay *display = dynGdk.gdk_display_get_default();
				if (!display) {
					Debug() << "Unable to get default Gdk display";
					desktop = "no_desktop";
				} else {
					bgchannel = dynXfconf.xfconf_channel_get("xfce4-desktop");

					int monitorCount = dynGdk.gdk_display_get_n_monitors(display);
					for (int i = 0; i < monitorCount; i++)
					{
						GdkMonitor *monitor = dynGdk.gdk_display_get_monitor(display, i);
						const char *model = dynGdk.gdk_monitor_get_model(monitor);
						std::string name;
						if (!model) {
							name = std::to_string(i);
						} else {
							name = std::string(model);
							name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
							monitorNames.push_back(name);
						}

						std::string propPrefix = "/backdrop/screen0/monitor" + name + "/workspace0/";
						std::string propLastImage = propPrefix + "last-image";
						std::string propColor1 = propPrefix + "rgba1";
						std::string propImageStyle = propPrefix + "image-style";
						std::string propColorStyle = propPrefix + "color-style";

						propsImage.push_back(propLastImage);
						propsColor1.push_back(propColor1);
						propsImageStyle.push_back(propImageStyle);
						propsColorStyle.push_back(propColorStyle);

						const char *xfcLastImage = dynXfconf.xfconf_channel_get_string(bgchannel, propLastImage.c_str(), "");
						int xfcImageStyle = dynXfconf.xfconf_channel_get_int(bgchannel, propImageStyle.c_str(), -1);
						int xfcColorStyle = dynXfconf.xfconf_channel_get_int(bgchannel, propColorStyle.c_str(), -1);

						defXfColors.push_back({ 0, 0, 0, 0 });
						bool xfcHasColor1 = dynXfconf.xfconf_channel_get_array(bgchannel, propColor1.c_str(),
							G_TYPE_DOUBLE, &defXfColors.back().red,
							G_TYPE_DOUBLE, &defXfColors.back().green,
							G_TYPE_DOUBLE, &defXfColors.back().blue,
							G_TYPE_DOUBLE, &defXfColors.back().alpha,
							G_TYPE_INVALID
						);

						defImages.push_back(std::string(xfcLastImage));
						defImageStyles.push_back(xfcImageStyle);
						defColorExists.push_back(xfcHasColor1);
						defColorStyles.push_back(xfcColorStyle);
					}
				}
			} else {
				// Configuration failed to initialize, we won't set the wallpaper
				desktop = "no_desktop";
				dynGlib.g_error_free(xferror);
			}
		} else if (desktop == "kde") {
			std::ifstream configFile;
			configFile.open(std::string(getenv("HOME")) + "/.config/plasma-org.kde.plasma.desktop-appletsrc", std::ios::in);
			if (configFile.is_open()) {
				std::string line;
				std::vector<std::string> sections;
				std::size_t undefined = 999999999;
				bool readPlugin = false, readOther = false;
				std::string containment;
				while (getline(configFile, line)) {
					std::size_t index = undefined, lastIndex = undefined;
					if (line.size() == 0) {
						readPlugin = false;
						readOther = false;
					} else if (readPlugin) {
						index = line.find('=');
						if (line.substr(0, index) == "wallpaperplugin") {
							defPlugins[containment] = line.substr(index + 1);
						}
					} else if (readOther) {
						index = line.find('=');
						std::string key = line.substr(0, index);
						std::string val = line.substr(index + 1);
						if (key == "Image") {
							defPictures[containment] = val;
						} else if (key == "Color") {
							defColors[containment] = val;
						} else if (key == "FillMode") {
							defModes[containment] = val;
						} else if (key == "Blur") {
							defBlurs[containment] = (val == "true");
						}
					} else if (line.at(0) == '[') {
						sections.clear();
						while (true) {
							index = line.find(lastIndex == undefined ? '[' : ']', index == undefined ? 0 : index);
							if (index == std::string::npos) {
								break;
							}
							if (lastIndex == undefined) {
								lastIndex = index;
							} else {
								sections.push_back(line.substr(lastIndex + 1, index - lastIndex - 1));
								lastIndex = undefined;
							}
						}
						if (sections.size() == 2 && sections[0] == "Containments") {
							readPlugin = true;
							containment = sections[1];
						} else if (
							sections.size() == 5 &&
							sections[0] == "Containments" &&
							sections[2] == "Wallpaper" &&
							sections[3] == "org.kde.image" &&
							sections[4] == "General"
						) {
							readOther = true;
							containment = sections[1];
						}
					}
				}
				configFile.close();
			} else {
				Debug() << "FATAL: Cannot find desktop configuration!";
				desktop = "kde_error";
			}
		}
		fallbackPath = std::string(getenv("HOME")) + "/Desktop/ONESHOT_hint.png";
	}
#endif

inline void replace_all(std::string &str, const std::string &from, const std::string &to)
{
    if (from.empty())
		return;

    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos)
	{
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
}

RB_METHOD(wallpaperSet)
{
	RB_UNUSED_PARAM;
	const char *name;
	int color;
	rb_get_args(argc, argv, "zi", &name, &color RB_ARG_END);
	std::string path;
#ifdef _WIN32
	path = shState->config().gameFolder + "\\Wallpaper\\" + name + ".bmp";
	Debug() << "Setting wallpaper to" << path;
	// Crapify the slashes
	size_t index = 0;
	for (;;) {
		index = path.find("/", index);
		if (index == std::string::npos)
			break;
		path.replace(index, 1, "\\");
		index += 1;
	}
	WCHAR imgnameW[MAX_PATH];
	WCHAR imgnameFull[MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, imgnameW, MAX_PATH);
	GetFullPathNameW(imgnameW, MAX_PATH, imgnameFull, NULL);


	int colorId = COLOR_BACKGROUND;
	WCHAR zero[2] = L"0";
	DWORD zeroSize = 4;

	HKEY hKey = NULL;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Control Panel\\Desktop", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
		goto end;

	if (!isCached) {
		// QUERY

		// Style
		setStyle = RegQueryValueExW(hKey, L"WallpaperStyle", 0, NULL, (LPBYTE)(szStyle), &szStyleSize) == ERROR_SUCCESS;

		// Tile
		setTile = RegQueryValueExW(hKey, L"TileWallpaper", 0, NULL, (LPBYTE)(szTile), &szTileSize) == ERROR_SUCCESS;

		// File path
		if (!SystemParametersInfoW(SPI_GETDESKWALLPAPER, MAX_PATH, (PVOID)szFile, 0))
			goto end;

		// Color
		oldcolor = GetSysColor(COLOR_BACKGROUND);

		isCached = true;
	}

	RegCloseKey(hKey);
	hKey = NULL;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Control Panel\\Desktop", 0, KEY_WRITE, &hKey) != ERROR_SUCCESS)
		goto end;

	// SET

	// Set the style
	if (RegSetValueExW(hKey, L"WallpaperStyle", 0, REG_SZ, (const BYTE*)zero, zeroSize) != ERROR_SUCCESS)
		goto end;

	if (RegSetValueExW(hKey, L"TileWallpaper", 0, REG_SZ, (const BYTE*)zero, zeroSize) != ERROR_SUCCESS)
		goto end;

	// Set the wallpaper
	if (!SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (PVOID)imgnameFull, SPIF_UPDATEINIFILE))
		goto end;

	// Set the color
	if (!SetSysColors(1, &colorId, (const COLORREF *)&color))
		goto end;
end:
	if (hKey)
		RegCloseKey(hKey);
#else
	std::string nameFix(name);
	std::size_t found = nameFix.find("w32");
	if (found != std::string::npos) {
		nameFix.replace(nameFix.end()-3, nameFix.end(), "unix");
	}
	path = "/Wallpaper/" + nameFix + ".png";

	Debug() << "Setting wallpaper to " << path;

	#ifdef __APPLE__
		if (!isCached) {
			MacDesktop::CacheCurrentBackground();
			isCached = true;
		}
		MacDesktop::ChangeBackground(shState->config().gameFolder + path, ((color >> 16) & 0xFF) / 255.0, ((color >> 8) & 0xFF) / 255.0, (color & 0xFF) / 255.0);
	#else
		char gameDir[PATH_MAX];
		if (getcwd(gameDir, sizeof(gameDir)) == NULL) {
			return Qnil;
		}
		std::string gameDirStr(gameDir);
		desktopEnvironmentInit();
		if (desktop == "cinnamon" || desktop == "gnome" || desktop == "mate" || desktop == "deepin") {
			std::stringstream hexColor;
			hexColor << "#" << std::hex << color;
			dynGio.g_settings_set_string(bgsetting, "picture-options", "scaled");
			dynGio.g_settings_set_string(bgsetting, "primary-color", hexColor.str().c_str());
			dynGio.g_settings_set_string(bgsetting, "color-shading-type", "solid");
			if (desktop == "cinnamon" || desktop == "gnome" || desktop == "deepin") {
				dynGio.g_settings_set_string(bgsetting, "picture-uri", ("file://" + gameDirStr + path).c_str());
				if (desktop == "gnome" && !defPictureDarkURI.empty())
					dynGio.g_settings_set_string(bgsetting, "picture-uri-dark", ("file://" + gameDirStr + path).c_str());
			} else {
				dynGio.g_settings_set_string(bgsetting, "picture-filename", (gameDirStr + path).c_str());
			}
		} else if (desktop == "xfce") {
			std::string concatPath(gameDirStr + path);
			double r = ((color >> 16) & 0xFF) / 255.0;
			double g = ((color >> 8) & 0xFF) / 255.0;
			double b = (color & 0xFF) / 255.0;
			double a = 1.0;
			for (std::size_t i = 0; i < monitorNames.size(); i++)
			{
				dynXfconf.xfconf_channel_set_string(bgchannel, propsImage.at(i).c_str(), concatPath.c_str());
				dynXfconf.xfconf_channel_set_int(bgchannel, propsImageStyle.at(i).c_str(), 4);
				dynXfconf.xfconf_channel_set_int(bgchannel, propsColorStyle.at(i).c_str(), 0);
				dynXfconf.xfconf_channel_set_array(bgchannel, propsColor1.at(i).c_str(),
					G_TYPE_DOUBLE, &r,
					G_TYPE_DOUBLE, &g,
					G_TYPE_DOUBLE, &b,
					G_TYPE_DOUBLE, &a,
					G_TYPE_INVALID
				);
			}
		} else if (desktop == "kde") {
			std::stringstream command;
			std::string concatPath(gameDirStr + path);
			replace_all(concatPath, "\\", "\\\\");
			replace_all(concatPath, "\"", "\\\"");
			replace_all(concatPath, "'", "\\x27");
			command << "qdbus org.kde.plasmashell /PlasmaShell org.kde.PlasmaShell.evaluateScript 'string:" <<
				"var allDesktops = desktops();" <<
				"for (var i = 0, l = allDesktops.length; i < l; ++i) {" <<
					"var d = allDesktops[i];" <<
					"d.wallpaperPlugin = \"org.kde.image\";" <<
					"d.currentConfigGroup = [\"Wallpaper\", \"org.kde.image\", \"General\"];" <<
					"d.writeConfig(\"Image\", \"file://" << concatPath << "\");" <<
					"d.writeConfig(\"FillMode\", \"6\");" <<
					"d.writeConfig(\"Blur\", false);" <<
					"d.writeConfig(\"Color\", [\"" <<
						std::to_string((color >> 16) & 0xFF) << "\", \"" <<
						std::to_string((color >> 8) & 0xFF) << "\", \"" <<
						std::to_string(color & 0xFF) <<
					"\"]);" <<
				"}" <<
			"'";
			Debug() << "Wallpaper command:" << command.str();
			int result = system(command.str().c_str());
			Debug() << "Result:" << result;
		} else {
			std::ifstream srcHint(gameDirStr + path);
			std::ofstream dstHint(fallbackPath);
			Debug() << "Copying hint file from " << gameDirStr + path << " to " << fallbackPath;
			dstHint << srcHint.rdbuf();
			srcHint.close();
			dstHint.close();
		}
	#endif
#endif
	return Qnil;
}

RB_METHOD(wallpaperReset)
{
	RB_UNUSED_PARAM;
#ifdef _WIN32
	if (isCached) {
		int colorId = COLOR_BACKGROUND;
		HKEY hKey = NULL;
		if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Control Panel\\Desktop", 0, KEY_WRITE, &hKey) != ERROR_SUCCESS)
			goto end;

		// Set the style
		if (setStyle)
			RegSetValueExW(hKey, L"WallpaperStyle", 0, REG_SZ, (const BYTE*)szStyle, szStyleSize);

		if (setTile)
			RegSetValueExW(hKey, L"TileWallpaper", 0, REG_SZ, (const BYTE*)szTile, szTileSize);

		// Set the wallpaper
		if (!SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, (PVOID)szFile, SPIF_UPDATEINIFILE))
			goto end;

		// Set the color
		if (!SetSysColors(1, &colorId, (const COLORREF *)&oldcolor))
			goto end;
	end:
		if (hKey)
			RegCloseKey(hKey);
	}
#else
	#ifdef __APPLE__
		MacDesktop::ResetBackground();
	#else
		desktopEnvironmentInit();
		if (desktop == "cinnamon" || desktop == "gnome" || desktop == "mate" || desktop == "deepin") {
			if (desktop == "cinnamon" || desktop == "gnome" || desktop == "deepin") {
				dynGio.g_settings_set_string(bgsetting, "picture-uri", defPictureURI.c_str());
				if (desktop == "gnome" && !defPictureDarkURI.empty())
					dynGio.g_settings_set_string(bgsetting, "picture-uri-dark", defPictureDarkURI.c_str());
			} else {
				dynGio.g_settings_set_string(bgsetting, "picture-filename", defPictureURI.c_str());
			}
			dynGio.g_settings_set_string(bgsetting, "picture-options", defPictureOptions.c_str());
			dynGio.g_settings_set_string(bgsetting, "primary-color", defPrimaryColor.c_str());
			dynGio.g_settings_set_string(bgsetting, "color-shading-type", defColorShading.c_str());
		} else if (desktop == "xfce") {
			for (std::size_t i = 0; i < monitorNames.size(); i++)
			{
				if (defColorExists.at(i))
					dynXfconf.xfconf_channel_set_array(bgchannel, propsColor1.at(i).c_str(),
					G_TYPE_DOUBLE, &defXfColors.at(i).red,
					G_TYPE_DOUBLE, &defXfColors.at(i).green,
					G_TYPE_DOUBLE, &defXfColors.at(i).blue,
					G_TYPE_DOUBLE, &defXfColors.at(i).alpha,
					G_TYPE_INVALID
				);
				else
					dynXfconf.xfconf_channel_reset_property(bgchannel, propsColor1.at(i).c_str(), false);

				if (!defImages.at(i).empty())
					dynXfconf.xfconf_channel_set_string(bgchannel, propsImage.at(i).c_str(), defImages.at(i).c_str());
				else
					dynXfconf.xfconf_channel_reset_property(bgchannel, propsImage.at(i).c_str(), false);

				if (defImageStyles.at(i) == -1)
					dynXfconf.xfconf_channel_reset_property(bgchannel, propsImageStyle.at(i).c_str(), false);
				else
					dynXfconf.xfconf_channel_set_int(bgchannel, propsImageStyle.at(i).c_str(), defImageStyles.at(i));

				if (defColorStyles.at(i) == -1)
					dynXfconf.xfconf_channel_reset_property(bgchannel, propsColorStyle.at(i).c_str(), false);
				else
					dynXfconf.xfconf_channel_set_int(bgchannel, propsColorStyle.at(i).c_str(), defColorStyles.at(i));
			}
		} else if (desktop == "kde") {
			std::stringstream command;
			command << "qdbus org.kde.plasmashell /PlasmaShell org.kde.PlasmaShell.evaluateScript 'string:" <<
					"var allDesktops = desktops();" <<
					"var data = {";
			// Plugin, picture, color, mode, blur
			for (auto const& x : defPlugins) {
				command << "\"" << x.first << "\": {"
						<< "plugin: \"" << x.second << "\"";
				if (defPictures.find(x.first) != defPictures.end()) {
					std::string picture = defPictures[x.first];
					replace_all(picture, "\\", "\\\\");
					replace_all(picture, "\"", "\\\"");
					replace_all(picture, "'", "\\x27");
					command << ", picture: \"" << picture << "\"";
				}
				if (defColors.find(x.first) != defColors.end()) {
					command << ", color: \"" << defColors[x.first] << "\"";
				}
				if (defModes.find(x.first) != defModes.end()) {
					command << ", mode: \"" << defModes[x.first] << "\"";
				}
				if (defBlurs.find(x.first) != defBlurs.end() && defBlurs[x.first]) {
					command << ", blur: true";
				}
				command << "},";
			}
			command << "\"no\": {}};" <<
				"for (var i = 0, l = allDesktops.length; i < l; ++i) {" <<
					"var d = allDesktops[i];" <<
					"var dat = data[d.id];" <<
					"d.wallpaperPlugin = dat.plugin;" <<
					"d.currentConfigGroup = [\"Wallpaper\", \"org.kde.image\", \"General\"];" <<
					"if (dat.picture) {" <<
						"d.writeConfig(\"Image\", dat.picture);" <<
					"}" <<
					"if (dat.color) {" <<
						"d.writeConfig(\"Color\", dat.color.split(\",\"));" <<
					"}" <<
					"if (dat.mode) {" <<
						"d.writeConfig(\"FillMode\", dat.mode);" <<
					"}" <<
					"if (dat.blur) {" <<
						"d.writeConfig(\"Blur\", dat.blur);" <<
					"}" <<
				"}" <<
			"'";
			Debug() << "Reset wallpaper command:" << command.str();
			int result = system(command.str().c_str());
			Debug() << "Reset result:" << result;
		} else {
			if (remove(fallbackPath.c_str()) != 0) {
				Debug() << "Failed to delete:" << fallbackPath << "(" << strerror(errno) << ")";
			} else {
				Debug() << "Removed desktop hint at " << fallbackPath;
			}
		}
	#endif
#endif
	return Qnil;
}

void wallpaperBindingInit()
{
	VALUE module = rb_define_module("Wallpaper");

	// Functions
	_rb_define_module_function(module, "set", wallpaperSet);
	_rb_define_module_function(module, "reset", wallpaperReset);
}

#ifdef __linux__
void wallpaperBindingTerminate()
{
	// Clean up.
	xfconfLibraryShutdown();
}
#endif

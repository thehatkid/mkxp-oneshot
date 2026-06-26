/*
** config.cpp
**
** This file is part of mkxp.
**
** Copyright (C) 2013 Jonas Kulla <Nyocurio@gmail.com>
**
** mkxp is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 2 of the License, or
** (at your option) any later version.
**
** mkxp is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with mkxp.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "config.h"

#include <physfs.h>

#include <fstream>
#include <stdint.h>
#include <cstdlib>

#include "debugwriter.h"
#include "util.h"
#include "sdl-util.h"

#include "CLI11.hpp"

namespace std
{
	std::ostream& operator<<(std::ostream &os, const std::vector<std::string> &vec)
	{
		for (auto item : vec)
		{
			os << item << " ";
		}
		return os;
	}
}

static std::string prefPath(const char *org, const char *app)
{
	const char *path = PHYSFS_getPrefDir(org, app);

	if (!path)
		return std::string();

	return path;
}

template<typename T>
std::set<T> setFromVec(const std::vector<T> &vec)
{
	return std::set<T>(vec.begin(), vec.end());
}

typedef std::vector<std::string> StringVec;

#define CONF_FILE "oneshot.conf"

Config::Config()
{}

void Config::read(int argc, char *argv[])
{
#define PO_DESC_ALL \
	PO_DESC(debugMode, bool, false) \
	PO_DESC(screenMode, bool, false) \
	PO_DESC(printFPS, bool, false) \
	PO_DESC(fullscreen, bool, false) \
	PO_DESC(fixedAspectRatio, bool, true) \
	PO_DESC(smoothScaling, bool, true) \
	PO_DESC(vsync, bool, true) \
	PO_DESC(defScreenW, int, 0) \
	PO_DESC(defScreenH, int, 0) \
	PO_DESC(windowTitle, std::string, "") \
	PO_DESC(fixedFramerate, int, 0) \
	PO_DESC(frameSkip, bool, true) \
	PO_DESC(syncToRefreshrate, bool, false) \
	PO_DESC(solidFonts, bool, false) \
	PO_DESC(subImageFix, bool, false) \
	PO_DESC(enableBlitting, bool, true) \
	PO_DESC(maxTextureSize, int, 0) \
	PO_DESC(gameFolder, std::string, ".") \
	PO_DESC(allowSymlinks, bool, false) \
	PO_DESC(pathCache, bool, true) \
	PO_DESC(iconPath, std::string, "") \
	PO_DESC(SE.sourceCount, int, 6) \
	PO_DESC(useScriptNames, bool, false)

	editor.debug = false;
	editor.battleTest = false;

	/* Read arguments sent from the editor */
	if (argc > 1)
	{
		std::string argv1 = argv[1];
		/* RGSS1 uses "debug", 2 and 3 use "test" */
		if (argv1 == "debug" || argv1 == "test")
			editor.debug = true;
		else if (argv1 == "btest")
			editor.battleTest = true;

		/* Fix offset */
		if (editor.debug || editor.battleTest)
		{
			argc--;
			argv++;
		}
	}

	CLI::App app {};

	/* Don't care about console help message */
	app.set_help_flag();

	/* Use TOML format for parsing config files */
	app.config_formatter(std::make_shared<CLI::ConfigTOML>());

	/* Add all config options */
#define PO_DESC(key, type, def) app.add_option<type>("--" #key, key)->default_val(def);
	PO_DESC_ALL
	app.add_option("--preloadScript", preloadScripts)->allow_extra_args();
	app.add_option("--fontSub", fontSubs)->allow_extra_args();
	app.add_option("--rubyLoadpath", rubyLoadpaths)->allow_extra_args();
#undef PO_DESC

	/* Initialize config variables */
#define PO_DESC(key, type, def) key = def;
	PO_DESC_ALL
#undef PO_DESC

#undef PO_DESC_ALL

	/* Parse command line options */
	try
	{
		app.parse(argc, argv);
	}
	catch (const CLI::ParseError &error)
	{
		Debug() << "Command line:" << error.what();
	}

	/* Parse configuration file */
	SDLRWStream confFile(CONF_FILE, "r");

	if (confFile)
	{
		try
		{
			app.parse_from_stream(confFile.stream());
		}
		catch (const CLI::Error &error)
		{
			Debug() << CONF_FILE":" << error.what();
		}
	}

	SE.sourceCount = clamp(SE.sourceCount, 1, 64);

	commonDataPath = prefPath(".", "Oneshot");

	//Hardcode some ini/version settings
	rgssVersion = 1;
	game.title = "OneShot";
	game.scripts = "Data/xScripts.rxdata";
	defScreenW = 640;
	defScreenH = 480;

#ifdef STEAM
	/* Override fullscreen config if Big Picture */
	if (const char *env = std::getenv("SteamTenfoot"))
	{
		if (!strcmp(env, "1"))
			fullscreen = true;
	}
#endif
}

#include "gnome-fun.h"
#include "debugwriter.h"

#include <SDL_error.h>
#include <SDL_loadso.h>

#ifdef __linux__
#define SONAME_GLIB "libglib-2.0.so.0"
#define SONAME_GIO "libgio-2.0.so.0"
#define SONAME_GOBJECT "libgobject-2.0.so.0"
#define SONAME_GTK_3 "libgtk-3.so.0"
#define SONAME_GTK_2 "libgtk-x11-2.0.so.0"
#else
#error "Platform not supported"
#endif

struct GlibFunctions dynGlib {};
struct GObjectFunctions dynGObject {};
struct GioFunctions dynGio {};
struct GtkFunctions dynGtk {};

static void *soGlib = nullptr;
static void *soGObject = nullptr;
static void *soGio = nullptr;
static void *soGtk = nullptr;

void initGlibFunctions()
{
	soGlib = SDL_LoadObject(SONAME_GLIB);
	if (soGlib == nullptr) {
		Debug() << "[gnome-fun] Unable to load Glib shared object:" << SDL_GetError();
		return;
	}

#define GNOME_FUNC(name, type) \
	dynGlib.name = (type)SDL_LoadFunction(soGlib, #name); \
	if (dynGlib.name == NULL) \
		Debug() << "[gnome-fun] Unable to load Glib function:" << SDL_GetError();

	DYN_GLIB_FUNCS

#undef GNOME_FUNC
}

void initGObjectFunctions()
{
	soGObject = SDL_LoadObject(SONAME_GOBJECT);
	if (soGlib == nullptr) {
		Debug() << "[gnome-fun] Unable to load GObject shared object:" << SDL_GetError();
		return;
	}

#define GNOME_FUNC(name, type) \
	dynGObject.name = (type)SDL_LoadFunction(soGObject, #name); \
	if (dynGObject.name == nullptr) \
		Debug() << "[gnome-fun] Unable to load GObject function:" << SDL_GetError();

	DYN_GOBJECT_FUNCS

#undef GNOME_FUNC
}

void initGioFunctions()
{
	if (soGio != nullptr)
		return;

	soGio = SDL_LoadObject(SONAME_GIO);
	if (soGio == nullptr) {
		Debug() << "[gnome-fun] Unable to load Gio shared object:" << SDL_GetError();
		return;
	}

#define GNOME_FUNC(name, type) \
	dynGio.name = (type)SDL_LoadFunction(soGio, #name); \
	if (dynGio.name == nullptr) \
		Debug() << "[gnome-fun] Unable to load Gio function:" << SDL_GetError();

	DYN_GIO_FUNCS

#undef GNOME_FUNC
}

void initGtkFunctions()
{
	soGtk = SDL_LoadObject(SONAME_GTK_3);
	if (soGtk == nullptr) {
		Debug() << "[gnome-fun] Unable to load Gtk shared object:" << SDL_GetError();
		return;
	}

#define GNOME_FUNC(name, type) \
	dynGtk.name = (type)SDL_LoadFunction(soGtk, #name); \
	if (dynGtk.name == nullptr) \
		Debug() << "[gnome-fun] Unable to load Gtk function:" << SDL_GetError();

	DYN_GTK_FUNCS

#undef GNOME_FUNC
}

void initGnomeFunctions()
{
	initGlibFunctions();
	initGObjectFunctions();
	initGioFunctions();
	initGtkFunctions();
}

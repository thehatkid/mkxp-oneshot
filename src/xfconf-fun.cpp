#include "xfconf-fun.h"
#include "debugwriter.h"

#include <SDL_error.h>
#include <SDL_loadso.h>

#ifdef __linux__
#define SONAME_XFCONF "libxfconf-0.so.3"
#else
#error "Platform not supported"
#endif

struct XfconfFunctions dynXfconf {};

static void *soXfconf = nullptr;

void initXfconfFunctions()
{
	soXfconf = SDL_LoadObject(SONAME_XFCONF);
	if (soXfconf == nullptr) {
		Debug() << "[xfconf-fun] Unable to load Xfconf shared object:" << SDL_GetError();
		return;
	}

#define XFCONF_FUNC(name, type) \
	dynXfconf.name = (type)SDL_LoadFunction(soXfconf, #name); \
	if (dynXfconf.name == NULL) \
		Debug() << "[xfconf-fun] Unable to load Xfconf function:" << SDL_GetError();

	DYN_XFCONF_FUNCS

#undef XFCONF_FUNC
}

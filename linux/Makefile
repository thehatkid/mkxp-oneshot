# Makefile for building dependencies on Linux.

# Specifies the number of jobs executed concurrently for each target.
# "make NPROC=2" to build targets with only 2 processing units (threads).
NPROC ?= $(shell nproc)

# Whether use Link-Time Optimization (LTO) for Ruby to improve performance,
# but drastically increases build time and is resource intensive at build time.
# "make LTO=1" to build targets with LTO.
LTO ?= 0

# Define build tools
CC  ?= gcc
CXX ?= g++

# Flags for builds
CFLAGS  := -O2 -mtune=generic
LDFLAGS :=

# Specifies the architecture name for builds
ARCH := $(shell uname -m)

# Specifies the target triplet for builds
TRIPLET := $(shell $(CC) -dumpmachine)

# Specifies the OpenSSL configure target name
OPENSSL_TARGET := linux-$(ARCH)

# Define build paths
MKFDIR := $(realpath $(shell dirname $(firstword $(MAKEFILE_LIST))))
BDIR   := build-$(ARCH)
PREFIX := $(MKFDIR)/build-$(ARCH)
DLDIR  := $(MKFDIR)/downloads
BINDIR := $(PREFIX)/bin
LIBDIR := $(PREFIX)/lib
INCDIR := $(PREFIX)/include
PKGDIR := $(PREFIX)/lib/pkgconfig

# Get system pkg-config packages paths
PC_PATH := $(shell pkg-config --variable pc_path pkg-config)

# Environment variables
ENV := \
	PKG_CONFIG_LIBDIR="$(PKGDIR):$(PC_PATH)" \
	CC="$(CC)" CXX="$(CXX)" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)"

# Autoconf arguments
AC_ARGS := \
	--prefix="$(PREFIX)" \
	--host="$(TRIPLET)"

# CMake arguments
CMAKE_ARGS := \
	-DCMAKE_PREFIX_PATH="$(PREFIX)" \
	-DCMAKE_INSTALL_PREFIX="$(PREFIX)" \
	-DCMAKE_BUILD_TYPE=Release

# Shortcut commands
GIT           := git clone -q -c advice.detachedHead=false --depth 1 --no-tags --single-branch
CONFIGURE     := $(ENV) ../configure $(AC_ARGS)
CMAKE         := $(ENV) cmake -S . -B "$(BDIR)" -G "Ninja" $(CMAKE_ARGS)
CMAKE_BUILD   := cmake --build "$(BDIR)" -- -j $(NPROC)
CMAKE_INSTALL := cmake --install "$(BDIR)"

# ================================ Build Options ===============================

OPTS_OGG := \
	-DBUILD_SHARED_LIBS=OFF \
	-DBUILD_TESTING=OFF \
	-DINSTALL_DOCS=OFF

OPTS_VORBIS := \
	-DBUILD_SHARED_LIBS=OFF

OPTS_ZLIB := \
	-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
	-DZLIB_BUILD_SHARED=OFF \
	-DZLIB_BUILD_TESTING=OFF

OPTS_PHYSFS := \
	-DPHYSFS_BUILD_SHARED=OFF \
	-DPHYSFS_BUILD_TEST=OFF \
	-DPHYSFS_BUILD_DOCS=OFF

OPTS_PNG := \
	-DPNG_SHARED=OFF \
	-DPNG_TESTS=OFF \
	-DPNG_TOOLS=OFF

OPTS_JPEG := \
	-DENABLE_SHARED=OFF \
	-DWITH_JPEG8=ON \
	-DWITH_TOOLS=OFF

OPTS_PIXMAN := \
	--disable-shared \
	--enable-static \
	--disable-arm-neon \
	--disable-arm-a64-neon \
	--disable-gtk \
	--disable-libpng

OPTS_SDL := \
	-DSDL_SHARED=OFF \
	-DSDL_STATIC=ON \
	-DSDL_TEST=OFF \
	-DSDL_SNDIO=OFF

OPTS_SDL_IMAGE := \
	-DBUILD_SHARED_LIBS=OFF \
	-DSDL2IMAGE_DEPS_SHARED=OFF \
	-DSDL2IMAGE_VENDORED=OFF \
	-DSDL2IMAGE_STRICT=ON \
	-DSDL2IMAGE_SAMPLES=OFF \
	-DSDL2IMAGE_TESTS=OFF \
	-DSDL2IMAGE_BACKEND_STB=OFF \
	-DSDL2IMAGE_AVIF=OFF \
	-DSDL2IMAGE_TIF=OFF \
	-DSDL2IMAGE_WEBP=OFF

OPTS_SDL_TTF := \
	-DBUILD_SHARED_LIBS=OFF \
	-DSDL2TTF_SAMPLES=OFF \
	-DSDL2TTF_VENDORED=ON \
	-DSDL2TTF_HARFBUZZ=ON

OPTS_SDL_SOUND := \
	-DSDLSOUND_BUILD_SHARED=OFF \
	-DSDLSOUND_BUILD_TEST=OFF \
	-DSDLSOUND_BUILD_DOCS=OFF

OPTS_OPENAL := \
	-DLIBTYPE=STATIC \
	-DALSOFT_STATIC_LIBGCC=ON \
	-DALSOFT_STATIC_STDCXX=ON \
	-DALSOFT_UTILS=OFF \
	-DALSOFT_EXAMPLES=OFF

OPTS_LIBYAML := \
	-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
	-DBUILD_TESTING=OFF \
	-DINSTALL_CMAKE_DIR="lib/cmake/yaml"

OPTS_LIBFFI := \
	--disable-shared \
	--enable-static \
	--disable-docs \
	--disable-multi-os-directory \
	--disable-symvers \
	--enable-pax_emutramp

OPTS_OPENSSL := \
	--prefix="$(PREFIX)" \
	--libdir="lib" \
	--openssldir="ssl" \
	$(OPENSSL_TARGET) \
	no-shared \
	no-tests \
	no-legacy \
	no-engine \
	no-makedepend

OPTS_RUBY := \
	ac_cv_lib_crypt_crypt=no \
	--prefix="$(PREFIX)" \
	--host="$(TRIPLET)" \
	--enable-shared \
	--disable-rpath \
	--enable-load-relative \
	--disable-install-doc \
	--disable-rubygems \
	--without-gmp \
	--with-zlib-dir="$(PREFIX)" \
	--with-libyaml-dir="$(PREFIX)" \
	--with-libffi-dir="$(PREFIX)" \
	--with-openssl-dir="$(PREFIX)" \
	--with-out-ext=readline,pty,syslog,win32,win32ole

# ==============================================================================

.PHONY: \
	all init clean clean-download clean-prefix download prepare build \
	ogg vorbis zlib physfs png jpeg pixman sdl sdl_image sdl_ttf sdl_sound \
	openal libyaml libffi openssl ruby

all: init download prepare build

init: $(DLDIR) $(BINDIR) $(LIBDIR) $(INCDIR)

$(DLDIR):
	@mkdir -p "$(DLDIR)"

$(BINDIR):
	@mkdir -p "$(BINDIR)"

$(LIBDIR):
	@mkdir -p "$(LIBDIR)"

$(INCDIR):
	@mkdir -p "$(INCDIR)"

clean: clean-download clean-prefix

clean-download:
	@printf "\e[91m=>\e[0m \e[35mCleaning downloads...\e[0m\n"
	@rm -rf "$(DLDIR)"

clean-prefix:
	@printf "\e[91m=>\e[0m \e[35mCleaning prefix...\e[0m\n"
	@rm -rf "$(PREFIX)"

download: \
	$(DLDIR) \
	$(DLDIR)/ogg/CMakeLists.txt \
	$(DLDIR)/vorbis/CMakeLists.txt \
	$(DLDIR)/zlib/CMakeLists.txt \
	$(DLDIR)/physfs/CMakeLists.txt \
	$(DLDIR)/libpng/CMakeLists.txt \
	$(DLDIR)/libjpeg/CMakeLists.txt \
	$(DLDIR)/pixman/autogen.sh \
	$(DLDIR)/sdl/CMakeLists.txt \
	$(DLDIR)/sdl_image/CMakeLists.txt \
	$(DLDIR)/sdl_ttf/CMakeLists.txt \
	$(DLDIR)/sdl_ttf/external/freetype/CMakeLists.txt \
	$(DLDIR)/sdl_ttf/external/harfbuzz/CMakeLists.txt \
	$(DLDIR)/sdl_sound/CMakeLists.txt \
	$(DLDIR)/openal/CMakeLists.txt \
	$(DLDIR)/libyaml/CMakeLists.txt \
	$(DLDIR)/libffi/configure.ac \
	$(DLDIR)/openssl/Configure \
	$(DLDIR)/ruby/configure.ac

prepare: \
	download \
	$(DLDIR)/libffi/configure \
	$(DLDIR)/ruby/configure

build: \
	ogg \
	vorbis \
	zlib \
	physfs \
	png \
	jpeg \
	pixman \
	sdl \
	sdl_image \
	sdl_ttf \
	sdl_sound \
	openal \
	libyaml \
	libffi \
	openssl \
	ruby

# =============================== Xiph.Org codecs ==============================

# ------------------------------------- Ogg ------------------------------------
ogg: init $(LIBDIR)/libogg.a

$(LIBDIR)/libogg.a: $(DLDIR)/ogg/$(BDIR)/libogg.a
	@printf "\e[94m=>\e[0m \e[36mInstalling Ogg...\e[0m\n"
	@cd "$(DLDIR)/ogg"; $(CMAKE_INSTALL)
	@touch -c "$@"

$(DLDIR)/ogg/$(BDIR)/libogg.a: $(DLDIR)/ogg/$(BDIR)/build.ninja
	@printf "\e[94m=>\e[0m \e[36mBuilding Ogg...\e[0m\n"
	@cd "$(DLDIR)/ogg"; $(CMAKE_BUILD)

$(DLDIR)/ogg/$(BDIR)/build.ninja: $(DLDIR)/ogg/CMakeLists.txt
	@printf "\e[94m=>\e[0m \e[36mConfiguring Ogg...\e[0m\n"
	@cd "$(DLDIR)/ogg"; $(CMAKE) $(OPTS_OGG)

$(DLDIR)/ogg/CMakeLists.txt:
	@printf "\e[94m=>\e[0m \e[36mDownloading Ogg 1.3.6...\e[0m\n"
	@$(GIT) -b v1.3.6 https://github.com/xiph/ogg "$(DLDIR)/ogg"

# ----------------------------------- Vorbis -----------------------------------
vorbis: init ogg $(LIBDIR)/libvorbis.a

$(LIBDIR)/libvorbis.a: $(DLDIR)/vorbis/$(BDIR)/lib/libvorbis.a
	@printf "\e[94m=>\e[0m \e[36mInstalling Vorbis...\e[0m\n"
	@cd "$(DLDIR)/vorbis"; $(CMAKE_INSTALL)
	@touch -c "$@"

$(DLDIR)/vorbis/$(BDIR)/lib/libvorbis.a: $(DLDIR)/vorbis/$(BDIR)/build.ninja
	@printf "\e[94m=>\e[0m \e[36mBuilding Vorbis...\e[0m\n"
	@cd "$(DLDIR)/vorbis"; $(CMAKE_BUILD)

$(DLDIR)/vorbis/$(BDIR)/build.ninja: $(DLDIR)/vorbis/CMakeLists.txt
	@printf "\e[94m=>\e[0m \e[36mConfiguring Vorbis...\e[0m\n"
	@cd "$(DLDIR)/vorbis"; $(CMAKE) $(OPTS_VORBIS)

$(DLDIR)/vorbis/CMakeLists.txt:
	@printf "\e[94m=>\e[0m \e[36mDownloading Vorbis 1.3.7...\e[0m\n"
	@$(GIT) -b v1.3.7 https://github.com/xiph/vorbis "$(DLDIR)/vorbis"

# =================================== General ==================================

# ------------------------------------ zlib ------------------------------------
zlib: init $(LIBDIR)/libz.a

$(LIBDIR)/libz.a: $(DLDIR)/zlib/$(BDIR)/libz.a
	@printf "\e[94m=>\e[0m \e[36mInstalling zlib...\e[0m\n"
	@cd "$(DLDIR)/zlib"; $(CMAKE_INSTALL)
	@touch -c "$@"

$(DLDIR)/zlib/$(BDIR)/libz.a: $(DLDIR)/zlib/$(BDIR)/build.ninja
	@printf "\e[94m=>\e[0m \e[36mBuilding zlib...\e[0m\n"
	@cd "$(DLDIR)/zlib"; $(CMAKE_BUILD)

$(DLDIR)/zlib/$(BDIR)/build.ninja: $(DLDIR)/zlib/CMakeLists.txt
	@printf "\e[94m=>\e[0m \e[36mConfiguring zlib...\e[0m\n"
	@cd "$(DLDIR)/zlib"; $(CMAKE) $(OPTS_ZLIB)

$(DLDIR)/zlib/CMakeLists.txt:
	@printf "\e[94m=>\e[0m \e[36mDownloading zlib 1.3.2...\e[0m\n"
	@$(GIT) -b v1.3.2 https://github.com/madler/zlib "$(DLDIR)/zlib"

# ----------------------------------- PhysFS -----------------------------------
physfs: init $(LIBDIR)/libphysfs.a

$(LIBDIR)/libphysfs.a: $(DLDIR)/physfs/$(BDIR)/libphysfs.a
	@printf "\e[94m=>\e[0m \e[36mInstalling PhysFS...\e[0m\n"
	@cd "$(DLDIR)/physfs"; $(CMAKE_INSTALL)
	@touch -c "$@"

$(DLDIR)/physfs/$(BDIR)/libphysfs.a: $(DLDIR)/physfs/$(BDIR)/build.ninja
	@printf "\e[94m=>\e[0m \e[36mBuilding PhysFS...\e[0m\n"
	@cd "$(DLDIR)/physfs"; $(CMAKE_BUILD)

$(DLDIR)/physfs/$(BDIR)/build.ninja: $(DLDIR)/physfs/CMakeLists.txt
	@printf "\e[94m=>\e[0m \e[36mConfiguring PhysFS...\e[0m\n"
	@cd "$(DLDIR)/physfs"; $(CMAKE) $(OPTS_PHYSFS)

$(DLDIR)/physfs/CMakeLists.txt:
	@printf "\e[94m=>\e[0m \e[36mDownloading PhysFS 3.2.0...\e[0m\n"
	@$(GIT) -b release-3.2.0 https://github.com/icculus/physfs "$(DLDIR)/physfs"

# =============================== Image libraries ==============================

# ------------------------------------- PNG ------------------------------------
png: init zlib $(LIBDIR)/libpng.a

$(LIBDIR)/libpng.a: $(DLDIR)/libpng/$(BDIR)/libpng.a
	@printf "\e[94m=>\e[0m \e[36mInstalling libpng...\e[0m\n"
	@cd "$(DLDIR)/libpng"; $(CMAKE_INSTALL)
	@touch -c "$@"

$(DLDIR)/libpng/$(BDIR)/libpng.a: $(DLDIR)/libpng/$(BDIR)/build.ninja
	@printf "\e[94m=>\e[0m \e[36mBuilding libpng...\e[0m\n"
	@cd "$(DLDIR)/libpng"; $(CMAKE_BUILD)

$(DLDIR)/libpng/$(BDIR)/build.ninja: $(DLDIR)/libpng/CMakeLists.txt
	@printf "\e[94m=>\e[0m \e[36mConfiguring libpng...\e[0m\n"
	@cd "$(DLDIR)/libpng"; $(CMAKE) $(OPTS_PNG)

$(DLDIR)/libpng/CMakeLists.txt:
	@printf "\e[94m=>\e[0m \e[36mDownloading libpng 1.6.58...\e[0m\n"
	@$(GIT) -b v1.6.58 https://github.com/pnggroup/libpng "$(DLDIR)/libpng"

# ------------------------------------ JPEG ------------------------------------
jpeg: init $(LIBDIR)/libjpeg.a

$(LIBDIR)/libjpeg.a: $(DLDIR)/libjpeg/$(BDIR)/libjpeg.a
	@printf "\e[94m=>\e[0m \e[36mInstalling libjpeg-turbo...\e[0m\n"
	@cd "$(DLDIR)/libjpeg"; $(CMAKE_INSTALL)
	@touch -c "$@"

$(DLDIR)/libjpeg/$(BDIR)/libjpeg.a: $(DLDIR)/libjpeg/$(BDIR)/build.ninja
	@printf "\e[94m=>\e[0m \e[36mBuilding libjpeg-turbo...\e[0m\n"
	@cd "$(DLDIR)/libjpeg"; $(CMAKE_BUILD)

$(DLDIR)/libjpeg/$(BDIR)/build.ninja: $(DLDIR)/libjpeg/CMakeLists.txt
	@printf "\e[94m=>\e[0m \e[36mConfiguring libjpeg-turbo...\e[0m\n"
	@cd "$(DLDIR)/libjpeg"; $(CMAKE) $(OPTS_JPEG)

$(DLDIR)/libjpeg/CMakeLists.txt:
	@printf "\e[94m=>\e[0m \e[36mDownloading libjpeg-turbo 3.1.4.1...\e[0m\n"
	@$(GIT) -b 3.1.4.1 https://github.com/libjpeg-turbo/libjpeg-turbo "$(DLDIR)/libjpeg"

# ----------------------------------- Pixman -----------------------------------
pixman: init $(LIBDIR)/libpixman-1.a

$(LIBDIR)/libpixman-1.a: $(DLDIR)/pixman/$(BDIR)/pixman/.libs/libpixman-1.a
	@printf "\e[94m=>\e[0m \e[36mInstalling Pixman...\e[0m\n"
	@cd "$(DLDIR)/pixman/$(BDIR)"; make install

$(DLDIR)/pixman/$(BDIR)/pixman/.libs/libpixman-1.a: $(DLDIR)/pixman/$(BDIR)/Makefile
	@printf "\e[94m=>\e[0m \e[36mBuilding Pixman...\e[0m\n"
	@cd "$(DLDIR)/pixman/$(BDIR)"; make -j $(NPROC)

$(DLDIR)/pixman/$(BDIR)/Makefile: $(DLDIR)/pixman/autogen.sh
	@printf "\e[94m=>\e[0m \e[36mConfiguring Pixman...\e[0m\n"
	@mkdir -p "$(DLDIR)/pixman/$(BDIR)"
	@cd "$(DLDIR)/pixman/$(BDIR)"; $(ENV) ../autogen.sh $(AC_ARGS) $(OPTS_PIXMAN)

$(DLDIR)/pixman/autogen.sh:
	@printf "\e[94m=>\e[0m \e[36mDownloading Pixman 0.42.2...\e[0m\n"
	@$(GIT) -b pixman-0.42.2 https://gitlab.freedesktop.org/pixman/pixman.git "$(DLDIR)/pixman"

# =============================== SDL and modules ==============================

# ------------------------------------- SDL ------------------------------------
sdl: init $(LIBDIR)/libSDL2.a

$(LIBDIR)/libSDL2.a: $(DLDIR)/sdl/$(BDIR)/libSDL2.a
	@printf "\e[94m=>\e[0m \e[36mInstalling SDL...\e[0m\n"
	@cd "$(DLDIR)/sdl"; $(CMAKE_INSTALL)
	@touch -c "$@"

$(DLDIR)/sdl/$(BDIR)/libSDL2.a: $(DLDIR)/sdl/$(BDIR)/build.ninja
	@printf "\e[94m=>\e[0m \e[36mBuilding SDL...\e[0m\n"
	@cd "$(DLDIR)/sdl"; $(CMAKE_BUILD)

$(DLDIR)/sdl/$(BDIR)/build.ninja: $(DLDIR)/sdl/CMakeLists.txt
	@printf "\e[94m=>\e[0m \e[36mConfiguring SDL...\e[0m\n"
	@cd "$(DLDIR)/sdl"; $(CMAKE) $(OPTS_SDL)

$(DLDIR)/sdl/CMakeLists.txt:
	@printf "\e[94m=>\e[0m \e[36mDownloading SDL 2.32.10...\e[0m\n"
	@$(GIT) -b release-2.32.10 https://github.com/libsdl-org/SDL "$(DLDIR)/sdl"

# ---------------------------------- SDL_image ---------------------------------
sdl_image: init sdl png jpeg $(LIBDIR)/libSDL2_image.a

$(LIBDIR)/libSDL2_image.a: $(DLDIR)/sdl_image/$(BDIR)/libSDL2_image.a
	@printf "\e[94m=>\e[0m \e[36mInstalling SDL_image...\e[0m\n"
	@cd "$(DLDIR)/sdl_image"; $(CMAKE_INSTALL)
	@touch -c "$@"

$(DLDIR)/sdl_image/$(BDIR)/libSDL2_image.a: $(DLDIR)/sdl_image/$(BDIR)/build.ninja
	@printf "\e[94m=>\e[0m \e[36mBuilding SDL_image...\e[0m\n"
	@cd "$(DLDIR)/sdl_image"; $(CMAKE_BUILD)

$(DLDIR)/sdl_image/$(BDIR)/build.ninja: $(DLDIR)/sdl_image/CMakeLists.txt
	@printf "\e[94m=>\e[0m \e[36mConfiguring SDL_image...\e[0m\n"
	@cd "$(DLDIR)/sdl_image"; $(CMAKE) $(OPTS_SDL_IMAGE)

$(DLDIR)/sdl_image/CMakeLists.txt:
	@printf "\e[94m=>\e[0m \e[36mDownloading SDL_image 2.8.12...\e[0m\n"
	@$(GIT) -b release-2.8.12 https://github.com/libsdl-org/SDL_image "$(DLDIR)/sdl_image"

# ----------------------------------- SDL_ttf ----------------------------------
sdl_ttf: init sdl $(LIBDIR)/libSDL2_ttf.a

$(LIBDIR)/libSDL2_ttf.a: $(DLDIR)/sdl_ttf/$(BDIR)/libSDL2_ttf.a
	@printf "\e[94m=>\e[0m \e[36mInstalling SDL_ttf...\e[0m\n"
	@cd "$(DLDIR)/sdl_ttf"; $(CMAKE_INSTALL)
	@touch -c "$@"

$(DLDIR)/sdl_ttf/$(BDIR)/libSDL2_ttf.a: $(DLDIR)/sdl_ttf/$(BDIR)/build.ninja
	@printf "\e[94m=>\e[0m \e[36mBuilding SDL_ttf...\e[0m\n"
	@cd "$(DLDIR)/sdl_ttf"; $(CMAKE_BUILD)

$(DLDIR)/sdl_ttf/$(BDIR)/build.ninja: $(DLDIR)/sdl_ttf/CMakeLists.txt

$(DLDIR)/sdl_ttf/$(BDIR)/build.ninja: $(DLDIR)/sdl_ttf/external/freetype/CMakeLists.txt

$(DLDIR)/sdl_ttf/$(BDIR)/build.ninja: $(DLDIR)/sdl_ttf/external/harfbuzz/CMakeLists.txt

$(DLDIR)/sdl_ttf/$(BDIR)/build.ninja:
	@printf "\e[94m=>\e[0m \e[36mConfiguring SDL_ttf...\e[0m\n"
	@cd "$(DLDIR)/sdl_ttf"; $(CMAKE) $(OPTS_SDL_TTF)

$(DLDIR)/sdl_ttf/external/harfbuzz/CMakeLists.txt: $(DLDIR)/sdl_ttf/CMakeLists.txt
	@printf "\e[94m=>\e[0m \e[36mDownloading SDL_ttf vendor: Harfbuzz...\e[0m\n"
	@$(GIT) -b 8.1.1-SDL https://github.com/libsdl-org/harfbuzz "$(DLDIR)/sdl_ttf/external/harfbuzz"

$(DLDIR)/sdl_ttf/external/freetype/CMakeLists.txt: $(DLDIR)/sdl_ttf/CMakeLists.txt
	@printf "\e[94m=>\e[0m \e[36mDownloading SDL_ttf vendor: Freetype...\e[0m\n"
	@$(GIT) -b VER-2-13-2-SDL https://github.com/libsdl-org/freetype "$(DLDIR)/sdl_ttf/external/freetype"

$(DLDIR)/sdl_ttf/CMakeLists.txt:
	@printf "\e[94m=>\e[0m \e[36mDownloading SDL_ttf 2.24.0...\e[0m\n"
	@$(GIT) -b release-2.24.0 https://github.com/libsdl-org/SDL_ttf "$(DLDIR)/sdl_ttf"

# ---------------------------------- SDL_sound ---------------------------------
sdl_sound: init sdl $(LIBDIR)/libSDL2_sound.a

$(LIBDIR)/libSDL2_sound.a: $(DLDIR)/sdl_sound/$(BDIR)/libSDL2_sound.a
	@printf "\e[94m=>\e[0m \e[36mInstalling SDL_sound...\e[0m\n"
	@cd "$(DLDIR)/sdl_sound"; $(CMAKE_INSTALL)
	@touch -c "$@"

$(DLDIR)/sdl_sound/$(BDIR)/libSDL2_sound.a: $(DLDIR)/sdl_sound/$(BDIR)/build.ninja
	@printf "\e[94m=>\e[0m \e[36mBuilding SDL_sound...\e[0m\n"
	@cd "$(DLDIR)/sdl_sound"; $(CMAKE_BUILD)

$(DLDIR)/sdl_sound/$(BDIR)/build.ninja: $(DLDIR)/sdl_sound/CMakeLists.txt
	@printf "\e[94m=>\e[0m \e[36mConfiguring SDL_sound...\e[0m\n"
	@cd "$(DLDIR)/sdl_sound"; $(CMAKE) $(OPTS_SDL_SOUND)

$(DLDIR)/sdl_sound/CMakeLists.txt:
	@printf "\e[94m=>\e[0m \e[36mDownloading SDL_sound 2.0.6...\e[0m\n"
	@$(GIT) -b v2.0.6 https://github.com/icculus/SDL_sound "$(DLDIR)/sdl_sound"

# =============================== Audio backends ===============================

# ----------------------------------- OpenAL -----------------------------------
openal: init $(LIBDIR)/libopenal.a

$(LIBDIR)/libopenal.a: $(DLDIR)/openal/$(BDIR)/libopenal.a
	@printf "\e[94m=>\e[0m \e[36mInstalling OpenAL...\e[0m\n"
	@cd "$(DLDIR)/openal"; $(CMAKE_INSTALL)
	@touch -c "$@"

$(DLDIR)/openal/$(BDIR)/libopenal.a: $(DLDIR)/openal/$(BDIR)/build.ninja
	@printf "\e[94m=>\e[0m \e[36mBuilding OpenAL...\e[0m\n"
	@cd "$(DLDIR)/openal"; $(CMAKE_BUILD)

$(DLDIR)/openal/$(BDIR)/build.ninja: $(DLDIR)/openal/CMakeLists.txt
	@printf "\e[94m=>\e[0m \e[36mConfiguring OpenAL...\e[0m\n"
	@cd "$(DLDIR)/openal"; $(CMAKE) $(OPTS_OPENAL)

$(DLDIR)/openal/CMakeLists.txt:
	@printf "\e[94m=>\e[0m \e[36mDownloading OpenAL 1.25.2...\e[0m\n"
	@$(GIT) -b 1.25.2 https://github.com/kcat/openal-soft "$(DLDIR)/openal"

# ============================== Ruby 3.1 and etc. =============================

# ----------------------------- libyaml (for Ruby) -----------------------------

libyaml: init $(LIBDIR)/libyaml.a

$(LIBDIR)/libyaml.a: $(DLDIR)/libyaml/$(BDIR)/libyaml.a
	@printf "\e[94m=>\e[0m \e[36mInstalling libyaml...\e[0m\n"
	@cd "$(DLDIR)/libyaml"; $(CMAKE_INSTALL)
	@touch -c "$@"

$(DLDIR)/libyaml/$(BDIR)/libyaml.a: $(DLDIR)/libyaml/$(BDIR)/build.ninja
	@printf "\e[94m=>\e[0m \e[36mBuilding libyaml...\e[0m\n"
	@cd "$(DLDIR)/libyaml"; $(CMAKE_BUILD)

$(DLDIR)/libyaml/$(BDIR)/build.ninja: $(DLDIR)/libyaml/CMakeLists.txt
	@printf "\e[94m=>\e[0m \e[36mConfiguring libyaml...\e[0m\n"
	@cd "$(DLDIR)/libyaml"; $(CMAKE) $(OPTS_LIBYAML)

$(DLDIR)/libyaml/CMakeLists.txt:
	@printf "\e[94m=>\e[0m \e[36mDownloading libyaml 0.2.5...\e[0m\n"
	@$(GIT) -b 0.2.5 https://github.com/yaml/libyaml "$(DLDIR)/libyaml"

# --------------------------- libffi (for Fiddle ext) --------------------------
libffi: init $(LIBDIR)/libffi.a

$(LIBDIR)/libffi.a: $(DLDIR)/libffi/$(BDIR)/.libs/libffi.a
	@printf "\e[94m=>\e[0m \e[36mInstalling libffi...\e[0m\n"
	@cd "$(DLDIR)/libffi/$(BDIR)"; make install

$(DLDIR)/libffi/$(BDIR)/.libs/libffi.a: $(DLDIR)/libffi/$(BDIR)/Makefile
	@printf "\e[94m=>\e[0m \e[36mBuilding libffi...\e[0m\n"
	@cd "$(DLDIR)/libffi/$(BDIR)"; make -j $(NPROC)

$(DLDIR)/libffi/$(BDIR)/Makefile: $(DLDIR)/libffi/configure
	@printf "\e[94m=>\e[0m \e[36mConfiguring libffi...\e[0m\n"
	@mkdir -p "$(DLDIR)/libffi/$(BDIR)"
	@cd "$(DLDIR)/libffi/$(BDIR)"; \
	export $(ENV); \
	export CFLAGS="-fPIC $$CFLAGS"; \
	../configure $(AC_ARGS) $(OPTS_LIBFFI)

$(DLDIR)/libffi/configure: $(DLDIR)/libffi/configure.ac
	@printf "\e[94m=>\e[0m \e[36mPreparing libffi configuration files...\e[0m\n"
	@cd "$(DLDIR)/libffi"; autoreconf -fi

$(DLDIR)/libffi/configure.ac:
	@printf "\e[94m=>\e[0m \e[36mDownloading libffi 3.5.2...\e[0m\n"
	@$(GIT) -b v3.5.2 https://github.com/libffi/libffi "$(DLDIR)/libffi"

# ----------------------------------- OpenSSL ----------------------------------
openssl: init $(LIBDIR)/libssl.a

$(LIBDIR)/libssl.a: $(DLDIR)/openssl/$(BDIR)/libssl.a
	@printf "\e[94m=>\e[0m \e[36mInstalling OpenSSL...\e[0m\n"
	@cd "$(DLDIR)/openssl/$(BDIR)"; make install_sw

$(DLDIR)/openssl/$(BDIR)/libssl.a: $(DLDIR)/openssl/$(BDIR)/Makefile
	@printf "\e[94m=>\e[0m \e[36mBuilding OpenSSL...\e[0m\n"
	@cd "$(DLDIR)/openssl/$(BDIR)"; make -j $(NPROC)

$(DLDIR)/openssl/$(BDIR)/Makefile: $(DLDIR)/openssl/Configure
	@printf "\e[94m=>\e[0m \e[36mConfiguring OpenSSL...\e[0m\n"
	@mkdir -p "$(DLDIR)/openssl/$(BDIR)"
	@cd "$(DLDIR)/openssl/$(BDIR)"; CC="$(CC)" perl ../Configure $(OPTS_OPENSSL)

$(DLDIR)/openssl/Configure:
	@printf "\e[94m=>\e[0m \e[36mDownloading OpenSSL 3.0.21...\e[0m\n"
	@$(GIT) -b openssl-3.0.21 https://github.com/openssl/openssl "$(DLDIR)/openssl"

# ------------------------------------ Ruby ------------------------------------
ruby: init zlib libyaml libffi openssl $(LIBDIR)/libruby.so.3.1

$(LIBDIR)/libruby.so.3.1: $(DLDIR)/ruby/$(BDIR)/libruby.so.3.1
	@printf "\e[94m=>\e[0m \e[36mInstalling Ruby...\e[0m\n"
	@cd "$(DLDIR)/ruby/$(BDIR)"; \
	export DEBIAN_DISABLE_RUBYGEMS_INTEGRATION=1; \
	make install

$(DLDIR)/ruby/$(BDIR)/libruby.so.3.1: $(DLDIR)/ruby/$(BDIR)/Makefile
	@printf "\e[94m=>\e[0m \e[36mBuilding Ruby...\e[0m\n"
	@cd "$(DLDIR)/ruby/$(BDIR)"; make -j $(NPROC)

$(DLDIR)/ruby/$(BDIR)/Makefile: $(DLDIR)/ruby/configure
	@printf "\e[94m=>\e[0m \e[36mConfiguring Ruby...\e[0m\n"
	@mkdir -p "$(DLDIR)/ruby/$(BDIR)"
ifeq ($(LTO),1)
	@cd "$(DLDIR)/ruby/$(BDIR)"; \
	export $(ENV); \
	export CFLAGS="-flto $$CFLAGS"; \
	export LDFLAGS="-flto -L$(LIBDIR) $$LDFLAGS"; \
	../configure $(OPTS_RUBY)
else
	@cd "$(DLDIR)/ruby/$(BDIR)"; \
	export $(ENV); \
	export LDFLAGS="-L$(LIBDIR) $$LDFLAGS"; \
	../configure $(OPTS_RUBY)
endif

$(DLDIR)/ruby/configure: $(DLDIR)/ruby/configure.ac
	@printf "\e[94m=>\e[0m \e[36mPreparing Ruby configuration files...\e[0m\n"
	@cd "$(DLDIR)/ruby"; autoreconf -fi

$(DLDIR)/ruby/configure.ac:
	@printf "\e[94m=>\e[0m \e[36mDownloading Ruby 3.1.7...\e[0m\n"
	@$(GIT) -b v3_1_7 https://github.com/ruby/ruby "$(DLDIR)/ruby"

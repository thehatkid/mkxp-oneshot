project(
    'mkxp-oneshot',
    'c', 'cpp',
    version: '1.0.0',
    license: 'GPL-2.0-or-later',
    meson_version: '>=0.58.0',
    default_options: [
        'cpp_std=c++14',
        'c_std=c11',
        'warning_level=1',
        'buildtype=debug',
        'b_ndebug=if-release',
        'prefer_static=true'
    ]
)

host_system = host_machine.system()
host_cpu_family = host_machine.cpu_family()

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

is_debug = get_option('debug')
is_steam = get_option('steam')
is_64bit = host_cpu_family in ['x86_64', 'aarch64', 'mips64', 'ppc64']

# Semantic versioning format: MAJOR.MINOR.PATCH[-PRE_RELEASE]
version_parts = meson.project_version().split('-')
version_split = version_parts[0].split('.')
version_major = version_split[0]
version_minor = version_split[1]
version_patch = version_split[2]
version_suffix = version_parts.length() > 1 ? version_parts[1] : ''

ruby = find_program('ruby', native: true, required: true)

embed = generator(
    ruby,
    arguments: [
        meson.project_source_root() / 'tools' / 'embed.rb',
        '@INPUT@',
        '@OUTPUT@',
        '@EXTRA_ARGS@'
    ],
    output: '@PLAINNAME@.xxd'
)

mkxp_name = 'oneshot'
mkxp_sources = []
mkxp_includes = []
mkxp_dependencies = []
mkxp_cflags = []
mkxp_ldflags = []

if not is_debug
    mkxp_cflags += '-DONESHOT_DISABLE_GLDEBUG'
endif

gfx_backend = get_option('gfx_backend')
if gfx_backend == 'gl'
    mkxp_dependencies += dependency('gl', static: false)
elif gfx_backend == 'gles'
    mkxp_cflags += '-DGLES2_HEADER'
else
    error('Unknown "gfx_backend" option value: @0@'.format(gfx_backend))
endif

if get_option('workdir_current')
    mkxp_cflags += '-DWORKDIR_CURRENT'
endif

if host_system == 'windows'
	# Use widechar versions of functions in Win32 API
	mkxp_cflags += ['-DUNICODE', '-D_UNICODE']

	# Per Windows:
	#   "Math Constants are not defined in Standard C/C++.
	#   To use them, you must first define _USE_MATH_DEFINES
	#   and then include cmath or math.h."
	mkxp_cflags += '-D_USE_MATH_DEFINES'

    # Static executable
    mkxp_ldflags += '-static'
endif

subdir('assets')
subdir('shader')
subdir('scripts')
subdir('src')
subdir('binding-mri')

if is_steam
    # Steamshim target for Steamworks integration
    steamshim_name = 'steamshim'
    steamshim_game = mkxp_name
    steamshim_sources = files(
        'steamshim_parent/steamshim_parent.cpp'
    )
    steamshim_includes = []
    steamshim_cflags = [
        '-DGAME_LAUNCH_NAME="@0@"'.format(steamshim_game)
    ]
    steamshim_ldflags = []
    steamshim_subsystem = 'windows,6.00'

    if is_debug
        steamshim_cflags += '-DSTEAMSHIM_DEBUG'
        steamshim_subsystem = 'console,6.00'
    endif

    if host_system == 'windows'
        steamshim_ldflags += '-static'
    else
        steamshim_ldflags += ['-static-libgcc', '-static-libstdc++']
    endif

    # Steamworks SDK library declaration
    steamworks_root = meson.project_source_root() / 'steamworks_sdk'
    steamworks_include = include_directories('steamworks_sdk' / 'public')
    steamworks_libpath = steamworks_root / 'redistributable_bin'

    steamworks_libname = 'steam_api'
    if host_system == 'windows'
        if is_64bit
            steamworks_libpath = steamworks_libpath / 'win64'
            steamworks_libname += '64'
        endif
    elif host_system == 'linux'
        if 'aarch64' in host_cpu_family
            steamworks_libpath = steamworks_libpath / 'linuxarm64'
        elif is_64bit
            steamworks_libpath = steamworks_libpath / 'linux64'
        else
            steamworks_libpath = steamworks_libpath / 'linux32'
        endif
    elif host_system == 'darwin'
        steamworks_libpath = steamworks_libpath / 'osx'
    else
        error('Unsupported host system for Steamworks SDK')
    endif

    steamworks_lib = cc.find_library(
        steamworks_libname,
        dirs: [steamworks_libpath],
        required: false
    )
    if not steamworks_lib.found()
        error('Could not locate Steamworks SDK library in @0@'.format(steamworks_libpath))
    endif
endif

if host_system == 'windows'
    subdir('windows')
endif

if get_option('copy_gamefiles')
	# Copy game files to the build directory
	install_subdir(
		'oneshot',
		install_dir: '',
		strip_directory: true
	)
endif

# mkxp executable
executable(
    mkxp_name,
    sources: mkxp_sources,
    include_directories: mkxp_includes,
    dependencies: mkxp_dependencies,
    c_args: mkxp_cflags,
    cpp_args: mkxp_cflags,
    link_args: mkxp_ldflags,
    install: true,
    win_subsystem: 'windows,6.00',
    build_rpath: '$ORIGIN',
    install_rpath: '$ORIGIN'
)

if is_steam and steamworks_lib.found()
    # Steamshim executable
    executable(
        steamshim_name,
        sources: steamshim_sources,
        include_directories: [steamworks_include, steamshim_includes],
        dependencies: [steamworks_lib],
        c_args: steamshim_cflags,
        cpp_args: steamshim_cflags,
        link_args: steamshim_ldflags,
        install: true,
        win_subsystem: steamshim_subsystem,
        build_rpath: '$ORIGIN',
        install_rpath: '$ORIGIN'
    )
endif

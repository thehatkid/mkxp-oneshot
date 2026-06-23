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

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

is_debug = get_option('debug')
is_steam = get_option('steam')
is_64bit = (cc.sizeof('void*') == 8)

xxd = find_program('xxd', native: true, required: true)
ruby = find_program('ruby', native: true, required: true)

mkxp_name = 'oneshot'
mkxp_sources = []
mkxp_includes = []
mkxp_dependencies = []
mkxp_cflags = [
    '-DHAVE_NANOSLEEP'
]
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
endif

subdir('assets')
subdir('shader')
subdir('scripts')
subdir('src')
subdir('binding-mri')

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
    win_subsystem: 'windows,6.0',
    build_rpath: '$ORIGIN',
    install_rpath: '$ORIGIN'
)

mkxp_dependencies += [
    dependency('ogg'),
    dependency('vorbis'),
    dependency('vorbisfile'),
    dependency('zlib'),
    dependency('physfs', version: '>=2.1'),
    dependency('libpng'),
    dependency('libjpeg'),
    dependency('pixman-1'),
    dependency('SDL2'),
    dependency('SDL2_image'),
    dependency('SDL2_ttf', method: 'cmake', modules: [
        'SDL2_ttf::SDL2_ttf-static',
        'SDL2_ttf::freetype',
        'SDL2_ttf::harfbuzz'
    ]),
    dependency('SDL2_sound'),
    dependency('openal'),
    dependency('sigc++-2.0', static: true, default_options: [
        'build-tests=false',
        'build-examples=false'
    ])
]

if host_system == 'windows'
    # Newer versions of Ruby will refuse to link without these
    explicit_libs = 'libmsvcrt;libgcc;libmingwex;libgmp;'
    foreach l : explicit_libs.split(';')
        if l != ''
            mkxp_ldflags += '-l:' + l + '.a'
        endif
    endforeach
endif

src_files = files(
    'main.cpp',
    'config.cpp',
    'sharedstate.cpp',
    'eventthread.cpp',
    'filesystem.cpp',
    'keybindings.cpp',
    'settingsmenu.cpp',
    'rgssad.cpp',
    'gl-fun.cpp',
    'gl-meta.cpp',
    'gl-debug.cpp',
    'glstate.cpp',
    'vertex.cpp',
    'texpool.cpp',
    'input.cpp',
    'graphics.cpp',
    'font.cpp',
    'audio.cpp',
    'audiostream.cpp',
    'alstream.cpp',
    'vorbissource.cpp',
    'sdlsoundsource.cpp',
    'soundemitter.cpp',
    'etc.cpp',
    'table.cpp',
    'viewport.cpp',
    'scene.cpp',
    'bitmap.cpp',
    'plane.cpp',
    'sprite.cpp',
    'window.cpp',
    'shader.cpp',
    'tilemap.cpp',
    'tileatlas.cpp',
    'tilequad.cpp',
    'autotiles.cpp',
    'oneshot.cpp',
    'i18n.cpp',
    'screen.cpp'
)

src_includes = [
    include_directories('.')
]

if is_steam
    src_files += files(
        'steam.cpp',
        '../steamshim/steamshim_child.c'
    )
    src_includes += include_directories('../steamshim')
    mkxp_cflags += '-DSTEAM'
endif

if host_system == 'linux'
    src_files += files(
        'gnome-fun.cpp',
        'xfconf-fun.cpp',
        'xdg-user-dir-lookup.c'
    )
endif

mkxp_sources += src_files

mkxp_includes += src_includes

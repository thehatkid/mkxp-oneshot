mri_version = get_option('mri_version')
mri_libname = get_option('mri_libname')
mri_libpath = get_option('mri_libpath')
mri_includes = get_option('mri_includes')

if mri_libpath == '' and mri_includes == []
    if mri_version.version_compare('>=3.0') and cxx.get_id() == 'clang'
        mkxp_cflags += '-fdeclspec'
    endif
    dep_ruby = dependency('ruby-' + mri_version, static: false)
    mkxp_dependencies += dep_ruby
else
    libruby = cc.find_library(
        mri_libname,
        dirs: [mri_libpath],
        has_headers: ['ruby.h'],
        header_include_directories: [include_directories(mri_includes)]
    )
    mkxp_dependencies += libruby
    mkxp_includes += include_directories(mri_includes)
endif

binding_mri_files = files(
    'binding-mri.cpp',
    'binding-util.cpp',
    'module_rpg.cpp',
    'etc-binding.cpp',
    'table-binding.cpp',
    'graphics-binding.cpp',
    'input-binding.cpp',
    'audio-binding.cpp',
    'filesystem-binding.cpp',
    'screen-binding.cpp',
    'viewport-binding.cpp',
    'bitmap-binding.cpp',
    'plane-binding.cpp',
    'sprite-binding.cpp',
    'font-binding.cpp',
    'tilemap-binding.cpp',
    'window-binding.cpp',
    'steam-binding.cpp',
    'oneshot-binding.cpp',
    'niko-binding.cpp',
    'wallpaper-binding.cpp',
    'journal-binding.cpp',
    'chroma-binding.cpp'
)

binding_mri_includes = [
    include_directories('.')
]

mkxp_sources += binding_mri_files

mkxp_includes += binding_mri_includes

win = import('windows')

win_rc_args = []
if is_debug
    win_rc_args += '-DDEBUG'
endif

win_config = configuration_data()
win_config.set('PROJECT_NAME', meson.project_name())
win_config.set('PROJECT_VERSION', meson.project_version())
win_config.set('VERSION_MAJOR', version_major)
win_config.set('VERSION_MINOR', version_minor)
win_config.set('VERSION_PATCH', version_patch)
win_config.set('AUTHOR', 'Future Cat Games')
win_config.set('COPYRIGHT', 'Copyright (c) 2026 Future Cat Games. Licensed under GPL-2.0 or later.')
win_config.set('COMMENT', '')

win_config_mkxp = configuration_data()
win_config_mkxp.merge_from(win_config)
win_config_mkxp.set('FILE_DESC', 'OneShot')
win_config_mkxp.set('PRODUCT_NAME', 'OneShot')
win_config_mkxp.set('FILENAME', '@0@.exe'.format(mkxp_name))
win_config_mkxp.set('ICON_FILE', 'icon.ico')
win_config_mkxp.set('MANIFEST_FILE', 'manifest.xml')
win_config_mkxp.set('MANIFEST_DESC', 'OneShot')
win_config_mkxp.set('ASSEMBLY_NAME', 'FutureCatGames.OneShot')

# mkxp Windows resources
win_mkxp_manifest = configure_file(
    input: 'manifest.xml.in',
    output: 'manifest.xml',
    configuration: win_config_mkxp
)
win_mkxp_resource = configure_file(
    input: 'resource.rc.in',
    output: 'resource.rc',
    configuration: win_config_mkxp
)

win_resources_mkxp = win.compile_resources(
    win_mkxp_resource,
    args: win_rc_args,
    include_directories: [
        include_directories('.')
    ],
    depend_files: [
        'icon.ico',
        win_mkxp_manifest
    ]
)

mkxp_sources += win_resources_mkxp

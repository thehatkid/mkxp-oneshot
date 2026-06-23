pack_scripts = custom_target(
    'xScripts.rxdata',
    output: 'xScripts.rxdata',
    command: [
        ruby,
        meson.project_source_root() / 'rpgscript.rb',
        '@CURRENT_SOURCE_DIR@',
        '@OUTPUT@'
    ],
    install: true,
    install_dir: 'Data',
    build_always_stale: true
)

mkxp_sources += pack_scripts

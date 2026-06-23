mkxp_assets = files(
    'icon.png',
    'gamecontrollerdb.txt'
)

assets_gen = generator(
    xxd,
    arguments: ['-i', '-n', 'assets_@PLAINNAME@', '@INPUT@', '@OUTPUT@'],
    output: '@PLAINNAME@.xxd'
)

mkxp_assets_xxd = assets_gen.process(mkxp_assets)

mkxp_sources += mkxp_assets_xxd

mkxp_shaders = files(
    'common.h',
    'minimal.vert',
    'simple.vert',
    'simple.frag',
    'simpleMatrix.vert',
    'simpleColor.vert',
    'simpleColor.frag',
    'simpleAlpha.frag',
    'simpleAlphaUni.frag',
    'flatColor.frag',
    'bitmapBlit.frag',
    'sprite.vert',
    'sprite.frag',
    'plane.frag',
    'tilemap.vert',
    'flashMap.frag',
    'gray.frag',
    'hue.frag',
    'blur.frag',
    'blurH.vert',
    'blurV.vert',
    'trans.frag',
    'transSimple.frag',
    'obscured.frag'
)

shader_gen = generator(
    xxd,
    arguments: ['-i', '-n', 'shader_@PLAINNAME@', '@INPUT@', '@OUTPUT@'],
    output: '@PLAINNAME@.xxd'
)

mkxp_shaders_xxd = shader_gen.process(mkxp_shaders)

mkxp_sources += mkxp_shaders_xxd

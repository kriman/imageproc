nhf_c_sources = [
  'main.c',
  'ppm.c',
  'addmath.c',
  'imagefunc.c',
]

nhf_c_deps = [
  dependency('glib-2.0'),
  meson.get_compiler('c').find_library('m', required: false)
]
executable('imageproc', nhf_c_sources,
  dependencies: nhf_c_deps,
  install: true,
)

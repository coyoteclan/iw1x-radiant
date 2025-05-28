project('iw1x-radiant', ['cpp', 'c'],
  default_options : ['cpp_std=c++20'],
  meson_version : '>=1.0.1'
)

# Subdirectory for client
subdir('src/radiant')

# Set the build type if not specified
if not meson.is_cross_build()
  buildtype = get_option('buildtype')
  if buildtype == 'plain'
    buildtype = 'debug'
  endif
  add_project_arguments('-DDEBUG', language : 'cpp')
endif

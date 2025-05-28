# Client source files

client_sources = files(
  'main.cpp',
  'pch.cpp',
  'loader/loader.cpp'
)

########### Include directories ###########

msvc_dir = run_command('sh', '-c', 'echo $MSVC_DIR', check : false).stdout().strip()
if msvc_dir == ''
  error('The MSVC_DIR environment variable is not set')
endif
client_includes = include_directories(
  '.',
  '../utils',
  './stock',
  '../../submodules/GSL/include',
  '../../submodules/minhook/include',
  '../../submodules/rapidjson/include',
  '../../submodules/discord-rpc/include',
  msvc_dir + '/VC/Tools/MSVC/14.44.35207/include',
  msvc_dir + '/Windows Kits/10/Include/10.0.26100.0/um',
  msvc_dir + '/Windows Kits/10/Include/10.0.26100.0/shared',
  msvc_dir + '/Windows Kits/10/Include/10.0.26100.0/ucrt',
  msvc_dir + '/VC/Tools/MSVC/14.44.35207/atlmfc/include'
)

########### Dependencies ###########

### gsl ###
gsl_dep = declare_dependency(
  include_directories : include_directories('../../submodules/GSL/include')
)

### minhook ###
minhook_sources = files(
  '../../submodules/minhook/src/buffer.c',
  '../../submodules/minhook/src/hook.c',
  '../../submodules/minhook/src/trampoline.c',
  '../../submodules/minhook/src/hde/hde32.c'
)

minhook_lib = static_library('minhook', minhook_sources,
  include_directories : include_directories('../../submodules/minhook/include', '../../submodules/minhook/src/hde', '../../submodules/minhook/src')
)

minhook_dep = declare_dependency(
  include_directories : include_directories('../../submodules/minhook/include'),
  link_with : minhook_lib
)

### utils ###
grab_utils_src = '''#!/bin/sh
for i in ../utils/*.cpp; do
  echo $i
done
'''
utils_sources = files(run_command('sh', '-c', grab_utils_src, check : false).stdout().strip().split('\n'))

grab_utils_headers = '''#!/bin/sh
for i in ../utils/*.h; do
  echo $i
done
'''
utils_headers = files(run_command('sh', '-c', grab_utils_headers, check : false).stdout().strip().split('\n'))

utils_lib = static_library('utils', utils_sources, utils_headers,
  include_directories : include_directories('../utils'),
  dependencies : [gsl_dep, minhook_dep]
)

utils_dep = declare_dependency(
  include_directories : include_directories('../utils'),
  link_with : utils_lib
)

### discord-rpc ###
discord_rpc_sources = files(
  '../../submodules/discord-rpc/src/connection_win.cpp',
  '../../submodules/discord-rpc/src/discord_rpc.cpp',
  '../../submodules/discord-rpc/src/discord_register_win.cpp',
  '../../submodules/discord-rpc/src/dllmain.cpp',
  '../../submodules/discord-rpc/src/rpc_connection.cpp',
  '../../submodules/discord-rpc/src/serialization.cpp'
)

discord_rpc_headers = files(
  '../../submodules/discord-rpc/include/discord_rpc.h'
)

discord_rpc_lib = static_library('discord-rpc', discord_rpc_sources, discord_rpc_headers,
  include_directories : include_directories('../../submodules/discord-rpc/include', '../../submodules/rapidjson/include')
)

discord_rpc_dep = declare_dependency(
  include_directories : include_directories('../../submodules/discord-rpc/include', '../../submodules/rapidjson/include'),
  link_with : discord_rpc_lib
)

### rapidjson ###
rapidjson_include = include_directories('../../submodules/rapidjson/include')

rapidjson_dep = declare_dependency(
  include_directories : rapidjson_include
)

### DbgHelp ###
dbghelp_dep = meson.get_compiler('cpp').find_library('DbgHelp', required : true)

### opengl32 ###
#opengl_dep = meson.get_compiler('cpp').find_library('opengl32', required : true)

# resource file
windres = meson.get_external_property('windres', '')

resource = custom_target('resource',
  input : 'resource.rc',
  output : 'resource.res',
  command : [windres, '@INPUT@', '-O', 'coff', '-o', '@OUTPUT@']
)

cod_dir = run_command('sh', '-c', 'echo $COD_DIR', check : false).stdout().strip()
if cod_dir == ''
  #cod_dir = meson.current_source_dir() + '/../../build'
  cod_dir = '/home/kazam/games/cod/Tools/bin'
  warning('The `COD_DIR` environment variable is not set. Defaulting to ' + run_command('sh', '-c', 'echo $(realpath "' + cod_dir + '")', check : true).stdout().strip())
endif

########### Executable ###########

iw1x_lib = shared_library('iw1x',
  sources: files('iw1x_core.cpp', 'loader/component_loader.cpp'),
  include_directories: client_includes,
  #dependencies: [utils_dep],
  #cpp_pch: './pch.h',
  install: true,
  install_dir: cod_dir,
  link_args: ['/DYNAMICBASE:NO'],
  cpp_args: ['/DBUILDING_IW1X_DLL'],
  name_prefix: 'lib',
  name_suffix: 'dll'
)
iw1x_dep = declare_dependency(link_with: iw1x_lib, include_directories: client_includes)

executable('iw1x-radiant', client_sources + resource,# client_headers, # Apparently not needed
  include_directories : client_includes,
  dependencies : [iw1x_dep, utils_dep, minhook_dep, gsl_dep],
  cpp_pch : './pch.h',
  install_dir : cod_dir,
  install : true,
  win_subsystem : 'windows',
  link_args : ['/DYNAMICBASE:NO', '/LAST:._text'],
)

scheduler_lib = shared_library('scheduler', files('./components/scheduler.cpp'),
  #include_directories : client_includes,
  #cpp_pch : './pch.h',
  install_dir : cod_dir,
  install : true,
  link_args : ['/DYNAMICBASE:NO'],
  cpp_args : ['/DSCHEDULER_LIB_EXPORTS'],
  name_prefix : '',
  name_suffix : 'dll'
)
scheduler_dep = declare_dependency(link_with: scheduler_lib)

shared_library('discord', files('./iw1x_plugins/discord.cpp'),
  include_directories : client_includes,
  dependencies : [iw1x_dep, utils_dep, minhook_dep, scheduler_dep, discord_rpc_dep, rapidjson_dep],
  cpp_pch : './pch.h',
  install_dir : cod_dir + '/iw1x_plugins',
  install : true,
  link_args : ['/DYNAMICBASE:NO'],
  cpp_args : ['/D_MSVC_WINE', '/DBUILDING_DLL'],
  name_prefix : '',
  name_suffix : 'dll'
)

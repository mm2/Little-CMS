project(
  'Little-CMS',
  'c',
  version: '2.14',
  meson_version: '>=0.48.0',
)

library_version = '2.0.13'

add_project_arguments('-D_POSIX_C_SOURCE', language: 'c')

cc = meson.get_compiler('c')

lcms2_srcs = files(
  'src/cmsalpha.c',
  'src/cmscam02.c',
  'src/cmscgats.c',
  'src/cmscnvrt.c',
  'src/cmserr.c',
  'src/cmsgamma.c',
  'src/cmsgmt.c',
  'src/cmshalf.c',
  'src/cmsintrp.c',
  'src/cmsio0.c',
  'src/cmsio1.c',
  'src/cmslut.c',
  'src/cmsmd5.c',
  'src/cmsmtrx.c',
  'src/cmsnamed.c',
  'src/cmsopt.c',
  'src/cmspack.c',
  'src/cmspcs.c',
  'src/cmsplugin.c',
  'src/cmsps2.c',
  'src/cmssamp.c',
  'src/cmssm.c',
  'src/cmstypes.c',
  'src/cmsvirt.c',
  'src/cmswtpnt.c',
  'src/cmsxform.c',
)

install_headers(
  'include/lcms2.h',
  'include/lcms2_plugin.h',
)

inc_dirs = include_directories('include')

m_dep = cc.find_library('m', required: false)
thread_dep = dependency('threads')

cargs = ['-DHasTHREADS=1']

# Check for threadsafe variants of gmtime
if cc.has_header_symbol('time.h', 'gmtime_r', args: '-D_POSIX_THREAD_SAFE_FUNCTIONS')
  cargs += '-DHAVE_GMTIME_R=1'
elif cc.has_function('gmtime_s', prefix: '#include <time.h>')
  cargs += '-DHAVE_GMTIME_S=1'
endif

if cc.has_function_attribute('visibility:hidden')
  cargs += '-DHAVE_FUNC_ATTRIBUTE_VISIBILITY=1'
endif

if host_machine.system() == 'windows'
  win = import('windows')
  lcms2_srcs += win.compile_resources('Projects/VC2019/lcms2.rc')
  cargs += '-DCMS_DLL_BUILD=1'
endif

liblcms2_lib = library(
  'lcms2',
  lcms2_srcs,
  include_directories: inc_dirs,
  gnu_symbol_visibility: 'hidden',
  dependencies: [m_dep, thread_dep],
  c_args: cargs,
  name_prefix: host_machine.system() == 'windows' ? '' : 'lib',
  version: library_version,
  soversion: host_machine.system() == 'windows' ? '' : '2',
  vs_module_defs: 'src/lcms2.def',
  install: true,
)

liblcms2_dep = declare_dependency(
  link_with: liblcms2_lib,
  include_directories: inc_dirs,
)

pkg = import('pkgconfig')
pkg.generate(liblcms2_lib)

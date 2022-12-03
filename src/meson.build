lcms2_srcs = files(
  'cmsalpha.c',
  'cmscam02.c',
  'cmscgats.c',
  'cmscnvrt.c',
  'cmserr.c',
  'cmsgamma.c',
  'cmsgmt.c',
  'cmshalf.c',
  'cmsintrp.c',
  'cmsio0.c',
  'cmsio1.c',
  'cmslut.c',
  'cmsmd5.c',
  'cmsmtrx.c',
  'cmsnamed.c',
  'cmsopt.c',
  'cmspack.c',
  'cmspcs.c',
  'cmsplugin.c',
  'cmsps2.c',
  'cmssamp.c',
  'cmssm.c',
  'cmstypes.c',
  'cmsvirt.c',
  'cmswtpnt.c',
  'cmsxform.c',
)

if host_machine.system() == 'windows'
  cargs += '-DCMS_DLL_BUILD=1'
endif

liblcms2_lib = library(
  'lcms2',
  lcms2_srcs,
  include_directories: inc_dirs,
  gnu_symbol_visibility: 'hidden',
  dependencies: deps,
  c_args: cargs,
  version: library_version,
  install: true,
)

liblcms2_dep = declare_dependency(
  link_with: liblcms2_lib,
  include_directories: [inc_dirs, include_directories('.')],
)

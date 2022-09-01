#project('jpegicc', 'c')

jpegicc_srcs = [
  'jpgicc.c',
  'iccjpeg.c',
  '../common/xgetopt.c',
  '../common/vprf.c',
  '../common/utils.h',
]

inc_common_dirs = include_directories('../common')

libjpeg_dep = dependency('libjpeg', required: get_option('jpeg'))
if libjpeg_dep.found()
  executable('jpegicc',
    jpegicc_srcs,
    c_args: cargs,
    dependencies : [liblcms2_dep, libjpeg_dep],
    include_directories : inc_common_dirs,
    install : true)
endif

install_man('jpgicc.1')
#EXTRA_DIST = iccjpeg.h $(man_MANS)

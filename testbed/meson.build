testcms_srcs = files(
  'testcms2.c',
  'testplugin.c',
  'zoo_icc.c',
)

# copy all iccs to the build dir and run the tests there
iccs = [
  'bad.icc', 'crayons.icc', 'new.icc', 'test2.icc', 'test4.icc', 
  'bad_mpe.icc', 'ibm-t61.icc', 'test1.icc', 'test3.icc', 'test5.icc',
  'toosmall.icc'
]
fs=import('fs')
foreach icc : iccs
  # fs.copyfile(icc) # DOES NOT WORK ON FEDORA 40
  configure_file(input : icc,  output : icc, copy : true)
endforeach

testcms = executable(
  'testcms',
  testcms_srcs,
  dependencies: [liblcms2_dep, m_dep],
  c_args: cargs,
)

test(
  'testcms',
  testcms,
  workdir: meson.current_build_dir(),
  timeout: 600,
)

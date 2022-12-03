project(
  'Little-CMS',
  'c',
  version: '2.14',
  meson_version: '>=0.48.0',
)

version_components = meson.project_version().split('.')

library_version = '@0@.0.@1@'.format(
  version_components.get(0, 0),
  version_components.get(1, 0),
)

cc = meson.get_compiler('c')

cargs = []

if cc.has_function_attribute('visibility:hidden')
  cargs += '-DHAVE_FUNC_ATTRIBUTE_VISIBILITY=1'
endif

# Check for threadsafe variants of gmtime
if cc.has_function('gmtime_r', prefix: '#include <time.h>')
  cargs += '-DHAVE_GMTIME_R=1'
elif cc.has_function('gmtime_s', prefix: '#include <time.h>')
  cargs += '-DHAVE_GMTIME_S=1'
endif

m_dep = cc.find_library('m', required: false)

threads_dep = dependency('threads')

if cc.has_function('pthread_mutex_lock', dependencies: threads_dep)
  cargs += '-DHasTHREADS=1'
else
  cargs += '-DHasTHREADS=0'
endif
deps = [m_dep, threads_dep]

subdir('include')
subdir('src')

pkg = import('pkgconfig')
pkg.generate(liblcms2_lib)

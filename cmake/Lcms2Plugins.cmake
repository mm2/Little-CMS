include(GNUInstallDirs)

function(_lcms2_add_plugin lib_name)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs SOURCES INCLUDE_DIRS)
  cmake_parse_arguments(PLUG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(LCMS2_BUILD_SHARED)
    add_library(${lib_name} SHARED ${PLUG_SOURCES})
  else()
    add_library(${lib_name} STATIC ${PLUG_SOURCES})
  endif()

  target_include_directories(${lib_name}
    PRIVATE
      "${PROJECT_SOURCE_DIR}/include"
      ${PLUG_INCLUDE_DIRS}
  )

  if(DEFINED LCMS2_LIBRARY_TARGET)
    target_link_libraries(${lib_name} PRIVATE "${LCMS2_LIBRARY_TARGET}")
  endif()

  if(WIN32)
    # Ensure plugin entrypoints using CMSEXPORT are exported from the DLL.
    target_compile_definitions(${lib_name} PRIVATE CMS_DLL_BUILD)
  endif()

  set_target_properties(${lib_name} PROPERTIES
    VERSION "${PROJECT_VERSION}.0"
    SOVERSION "2"
  )

  set(_lcms2_plugin_targets "${LCMS2_PLUGIN_TARGETS}")
  list(APPEND _lcms2_plugin_targets "${lib_name}")
  set(LCMS2_PLUGIN_TARGETS "${_lcms2_plugin_targets}" PARENT_SCOPE)
endfunction()

function(lcms2_add_plugins)
  include(CMakeParseArguments)

  # Populated by _lcms2_add_plugin() via PARENT_SCOPE into this function scope.
  set(LCMS2_PLUGIN_TARGETS "")

  if(LCMS2_WITH_FASTFLOAT)
    _lcms2_add_plugin(lcms2_fast_float
      SOURCES
        "${PROJECT_SOURCE_DIR}/plugins/fast_float/src/fast_8_curves.c"
        "${PROJECT_SOURCE_DIR}/plugins/fast_float/src/fast_8_matsh_sse.c"
        "${PROJECT_SOURCE_DIR}/plugins/fast_float/src/fast_8_matsh.c"
        "${PROJECT_SOURCE_DIR}/plugins/fast_float/src/fast_8_tethra.c"
        "${PROJECT_SOURCE_DIR}/plugins/fast_float/src/fast_16_tethra.c"
        "${PROJECT_SOURCE_DIR}/plugins/fast_float/src/fast_float_15bits.c"
        "${PROJECT_SOURCE_DIR}/plugins/fast_float/src/fast_float_15mats.c"
        "${PROJECT_SOURCE_DIR}/plugins/fast_float/src/fast_float_cmyk.c"
        "${PROJECT_SOURCE_DIR}/plugins/fast_float/src/fast_float_curves.c"
        "${PROJECT_SOURCE_DIR}/plugins/fast_float/src/fast_float_matsh.c"
        "${PROJECT_SOURCE_DIR}/plugins/fast_float/src/fast_float_separate.c"
        "${PROJECT_SOURCE_DIR}/plugins/fast_float/src/fast_float_sup.c"
        "${PROJECT_SOURCE_DIR}/plugins/fast_float/src/fast_float_tethra.c"
        "${PROJECT_SOURCE_DIR}/plugins/fast_float/src/fast_float_lab.c"
        "${PROJECT_SOURCE_DIR}/plugins/fast_float/src/fast_float_internal.h"
      INCLUDE_DIRS
        "${PROJECT_SOURCE_DIR}/plugins/fast_float/include"
        "${PROJECT_SOURCE_DIR}/plugins/fast_float/src/../include"
    )
  endif()

  if(LCMS2_WITH_THREADED_PLUGIN)
    _lcms2_add_plugin(lcms2_threaded
      SOURCES
        "${PROJECT_SOURCE_DIR}/plugins/threaded/src/threaded_split.c"
        "${PROJECT_SOURCE_DIR}/plugins/threaded/src/threaded_core.c"
        "${PROJECT_SOURCE_DIR}/plugins/threaded/src/threaded_main.c"
        "${PROJECT_SOURCE_DIR}/plugins/threaded/src/threaded_scheduler.c"
        "${PROJECT_SOURCE_DIR}/plugins/threaded/src/threaded_internal.h"
      INCLUDE_DIRS
        "${PROJECT_SOURCE_DIR}/plugins/threaded/include"
        "${PROJECT_SOURCE_DIR}/plugins/threaded/src/../include"
    )
  endif()

  # Propagate plugin target list to the root scope so packaging (install + pc) can see it.
  set(LCMS2_PLUGIN_TARGETS "${LCMS2_PLUGIN_TARGETS}" PARENT_SCOPE)
endfunction()



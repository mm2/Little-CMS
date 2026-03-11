function(lcms2_setup_packaging)
  include(GNUInstallDirs)
  include(CMakePackageConfigHelpers)

  # Headers
  install(
    FILES
      "${PROJECT_SOURCE_DIR}/include/lcms2.h"
      "${PROJECT_SOURCE_DIR}/include/lcms2_plugin.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  )

  # Collect installable targets.
  set(_install_targets "")

  foreach(_t IN ITEMS lcms2 lcms2_static)
    if(TARGET ${_t})
      list(APPEND _install_targets ${_t})
    endif()
  endforeach()

  if(DEFINED LCMS2_TOOL_TARGETS)
    foreach(_t IN LISTS LCMS2_TOOL_TARGETS)
      if(TARGET ${_t})
        list(APPEND _install_targets ${_t})
      endif()
    endforeach()
  endif()

  if(DEFINED LCMS2_PLUGIN_TARGETS)
    foreach(_t IN LISTS LCMS2_PLUGIN_TARGETS)
      if(TARGET ${_t})
        list(APPEND _install_targets ${_t})
      endif()
    endforeach()
  endif()

  if(_install_targets)
    install(
      TARGETS ${_install_targets}
      EXPORT lcms2-targets
      RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
      LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
      ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
      INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )
  endif()

  # Manpages (installed regardless of whether the corresponding tool is built).
  if(DEFINED LCMS2_TOOL_MANPAGES)
    install(
      FILES ${LCMS2_TOOL_MANPAGES}
      DESTINATION "${CMAKE_INSTALL_MANDIR}/man1"
    )
  endif()

  # CMake package config.
  set(_lcms2_cmake_config_dir "${CMAKE_INSTALL_LIBDIR}/cmake/lcms2")

  install(
    EXPORT lcms2-targets
    NAMESPACE lcms2::
    DESTINATION "${_lcms2_cmake_config_dir}"
  )

  configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/lcms2-config.cmake.in"
    "${PROJECT_BINARY_DIR}/lcms2-config.cmake"
    INSTALL_DESTINATION "${_lcms2_cmake_config_dir}"
  )

  write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/lcms2-config-version.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
  )

  install(
    FILES
      "${PROJECT_BINARY_DIR}/lcms2-config.cmake"
      "${PROJECT_BINARY_DIR}/lcms2-config-version.cmake"
    DESTINATION "${_lcms2_cmake_config_dir}"
  )

  # Build-tree export for local consumption.
  export(
    EXPORT lcms2-targets
    NAMESPACE lcms2::
    FILE "${PROJECT_BINARY_DIR}/lcms2-targets.cmake"
  )

  # pkg-config generation (mirrors lcms2.pc.in semantics).
  set(LIB_PLUGINS "")
  if(DEFINED LCMS2_PLUGIN_TARGETS)
    foreach(_t IN LISTS LCMS2_PLUGIN_TARGETS)
      if(TARGET ${_t})
        get_target_property(_out_name ${_t} OUTPUT_NAME)
        if(NOT _out_name)
          set(_out_name "${_t}")
        endif()
        set(LIB_PLUGINS "${LIB_PLUGINS} -l${_out_name}")
      endif()
    endforeach()
    string(STRIP "${LIB_PLUGINS}" LIB_PLUGINS)
  endif()

  set(LIB_MATH "")
  set(LIB_THREAD "")
  if(NOT WIN32)
    find_library(_lcms2_math_lib m)
    if(_lcms2_math_lib)
      set(LIB_MATH "-lm")
    endif()

    if(LCMS2_WITH_THREADS)
      find_package(Threads)
      if(CMAKE_THREAD_LIBS_INIT)
        set(LIB_THREAD "${CMAKE_THREAD_LIBS_INIT}")
      endif()
    endif()
  endif()

  set(prefix "${CMAKE_INSTALL_PREFIX}")
  set(exec_prefix "\${prefix}")
  if(IS_ABSOLUTE "${CMAKE_INSTALL_LIBDIR}")
    set(libdir "${CMAKE_INSTALL_LIBDIR}")
  else()
    set(libdir "\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}")
  endif()
  if(IS_ABSOLUTE "${CMAKE_INSTALL_INCLUDEDIR}")
    set(includedir "${CMAKE_INSTALL_INCLUDEDIR}")
  else()
    set(includedir "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
  endif()
  set(PACKAGE "${PROJECT_NAME}")
  set(VERSION "${PROJECT_VERSION}")
  configure_file(
    "${PROJECT_SOURCE_DIR}/lcms2.pc.in"
    "${PROJECT_BINARY_DIR}/lcms2.pc"
    @ONLY
  )
  install(
    FILES "${PROJECT_BINARY_DIR}/lcms2.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
  )
endfunction()



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
      EXPORT lcms2Targets
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
    EXPORT lcms2Targets
    NAMESPACE lcms2::
    DESTINATION "${_lcms2_cmake_config_dir}"
  )

  configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/lcms2Config.cmake.in"
    "${PROJECT_BINARY_DIR}/lcms2Config.cmake"
    INSTALL_DESTINATION "${_lcms2_cmake_config_dir}"
  )

  write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/lcms2ConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
  )

  install(
    FILES
      "${PROJECT_BINARY_DIR}/lcms2Config.cmake"
      "${PROJECT_BINARY_DIR}/lcms2ConfigVersion.cmake"
    DESTINATION "${_lcms2_cmake_config_dir}"
  )

  # Build-tree export for local consumption.
  export(
    EXPORT lcms2Targets
    NAMESPACE lcms2::
    FILE "${PROJECT_BINARY_DIR}/lcms2Targets.cmake"
  )

  # pkg-config generation (mirrors lcms2.pc.in semantics).
  set(LCMS2_PC_LIBS_PLUGINS "")
  if(DEFINED LCMS2_PLUGIN_TARGETS)
    foreach(_t IN LISTS LCMS2_PLUGIN_TARGETS)
      if(TARGET ${_t})
        get_target_property(_out_name ${_t} OUTPUT_NAME)
        if(NOT _out_name)
          set(_out_name "${_t}")
        endif()
        set(LCMS2_PC_LIBS_PLUGINS "${LCMS2_PC_LIBS_PLUGINS} -l${_out_name}")
      endif()
    endforeach()
    string(STRIP "${LCMS2_PC_LIBS_PLUGINS}" LCMS2_PC_LIBS_PLUGINS)
  endif()

  set(LCMS2_PC_LIBS_PRIVATE "")
  if(NOT WIN32)
    find_library(_lcms2_math_lib m)
    if(_lcms2_math_lib)
      set(LCMS2_PC_LIBS_PRIVATE "${LCMS2_PC_LIBS_PRIVATE} -lm")
    endif()

    if(LCMS2_WITH_THREADS)
      find_package(Threads)
      if(CMAKE_THREAD_LIBS_INIT)
        set(LCMS2_PC_LIBS_PRIVATE "${LCMS2_PC_LIBS_PRIVATE} ${CMAKE_THREAD_LIBS_INIT}")
      endif()
    endif()
  endif()
  string(STRIP "${LCMS2_PC_LIBS_PRIVATE}" LCMS2_PC_LIBS_PRIVATE)

  configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/lcms2.pc.in"
    "${PROJECT_BINARY_DIR}/lcms2.pc"
    @ONLY
  )
  install(
    FILES "${PROJECT_BINARY_DIR}/lcms2.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
  )
endfunction()



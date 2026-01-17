function(lcms2_add_tests)
  if(NOT LCMS2_BUILD_TESTS)
    return()
  endif()

  set(_testbed_bin_dir "${PROJECT_BINARY_DIR}/testbed")

  if(DEFINED LCMS2_STATIC_LIBRARY_TARGET)
    set(_lcms2_test_lib "${LCMS2_STATIC_LIBRARY_TARGET}")
  else()
    # Ensure tests can link statically even if the user disabled the installed static target.
    if(COMMAND _lcms2_collect_sources)
      _lcms2_collect_sources(_lcms2_sources_for_tests)
      add_library(lcms2_test_static STATIC ${_lcms2_sources_for_tests})
      if(COMMAND _lcms2_apply_common_settings)
        _lcms2_apply_common_settings(lcms2_test_static)
      endif()
      if(COMMAND _lcms2_apply_thread_settings)
        _lcms2_apply_thread_settings(lcms2_test_static)
      endif()
      set(_lcms2_test_lib lcms2_test_static)
    else()
      set(_lcms2_test_lib "${LCMS2_LIBRARY_TARGET}")
    endif()
  endif()

  add_executable(testcms
    "${PROJECT_SOURCE_DIR}/testbed/testcms2.c"
    "${PROJECT_SOURCE_DIR}/testbed/testplugin.c"
    "${PROJECT_SOURCE_DIR}/testbed/zoo_icc.c"
    "${PROJECT_SOURCE_DIR}/testbed/testcms2.h"
  )

  target_include_directories(testcms
    PRIVATE
      "${PROJECT_SOURCE_DIR}/include"
      "${PROJECT_SOURCE_DIR}/src"
      "${PROJECT_SOURCE_DIR}/testbed"
  )

  if(_lcms2_test_lib)
    target_link_libraries(testcms PRIVATE "${_lcms2_test_lib}")
  endif()

  set_target_properties(testcms PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${_testbed_bin_dir}"
  )

  # Copy ICC fixtures into the build testbed dir (mirrors autotools make check behavior).
  set(_icc_files
    "${PROJECT_SOURCE_DIR}/testbed/test1.icc"
    "${PROJECT_SOURCE_DIR}/testbed/test2.icc"
    "${PROJECT_SOURCE_DIR}/testbed/test3.icc"
    "${PROJECT_SOURCE_DIR}/testbed/test4.icc"
    "${PROJECT_SOURCE_DIR}/testbed/test5.icc"
    "${PROJECT_SOURCE_DIR}/testbed/ibm-t61.icc"
    "${PROJECT_SOURCE_DIR}/testbed/crayons.icc"
    "${PROJECT_SOURCE_DIR}/testbed/bad.icc"
    "${PROJECT_SOURCE_DIR}/testbed/bad_mpe.icc"
    "${PROJECT_SOURCE_DIR}/testbed/toosmall.icc"
  )

  set(_icc_outputs "")
  foreach(_f IN LISTS _icc_files)
    get_filename_component(_name "${_f}" NAME)
    set(_out "${_testbed_bin_dir}/${_name}")
    list(APPEND _icc_outputs "${_out}")

    add_custom_command(
      OUTPUT "${_out}"
      COMMAND "${CMAKE_COMMAND}" -E make_directory "${_testbed_bin_dir}"
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${_f}" "${_out}"
      DEPENDS "${_f}"
      VERBATIM
    )
  endforeach()

  add_custom_target(lcms2_test_fixtures ALL DEPENDS ${_icc_outputs})
  add_dependencies(testcms lcms2_test_fixtures)

  add_test(NAME lcms2.testcms COMMAND "${_testbed_bin_dir}/testcms")
  set_tests_properties(lcms2.testcms PROPERTIES WORKING_DIRECTORY "${_testbed_bin_dir}")
endfunction()



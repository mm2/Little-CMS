include(GNUInstallDirs)

function(_lcms2_collect_sources out_var)
  set(_src
    "${PROJECT_SOURCE_DIR}/src/cmsalpha.c"
    "${PROJECT_SOURCE_DIR}/src/cmscam02.c"
    "${PROJECT_SOURCE_DIR}/src/cmscgats.c"
    "${PROJECT_SOURCE_DIR}/src/cmscnvrt.c"
    "${PROJECT_SOURCE_DIR}/src/cmserr.c"
    "${PROJECT_SOURCE_DIR}/src/cmsgamma.c"
    "${PROJECT_SOURCE_DIR}/src/cmsgmt.c"
    "${PROJECT_SOURCE_DIR}/src/cmsintrp.c"
    "${PROJECT_SOURCE_DIR}/src/cmsio0.c"
    "${PROJECT_SOURCE_DIR}/src/cmsio1.c"
    "${PROJECT_SOURCE_DIR}/src/cmslut.c"
    "${PROJECT_SOURCE_DIR}/src/cmsplugin.c"
    "${PROJECT_SOURCE_DIR}/src/cmssm.c"
    "${PROJECT_SOURCE_DIR}/src/cmsmd5.c"
    "${PROJECT_SOURCE_DIR}/src/cmsmtrx.c"
    "${PROJECT_SOURCE_DIR}/src/cmspack.c"
    "${PROJECT_SOURCE_DIR}/src/cmspcs.c"
    "${PROJECT_SOURCE_DIR}/src/cmswtpnt.c"
    "${PROJECT_SOURCE_DIR}/src/cmsxform.c"
    "${PROJECT_SOURCE_DIR}/src/cmssamp.c"
    "${PROJECT_SOURCE_DIR}/src/cmsnamed.c"
    "${PROJECT_SOURCE_DIR}/src/cmsvirt.c"
    "${PROJECT_SOURCE_DIR}/src/cmstypes.c"
    "${PROJECT_SOURCE_DIR}/src/cmsps2.c"
    "${PROJECT_SOURCE_DIR}/src/cmsopt.c"
    "${PROJECT_SOURCE_DIR}/src/cmshalf.c"
    "${PROJECT_SOURCE_DIR}/src/lcms2_internal.h"
  )
  set(${out_var} "${_src}" PARENT_SCOPE)
endfunction()

function(_lcms2_apply_common_settings tgt)
  target_include_directories(${tgt}
    PUBLIC
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )

  # Feature defines derived from configure.ac parity.
  if(LCMS2_DEFINE_HAVE_GMTIME_R)
    target_compile_definitions(${tgt} PRIVATE HAVE_GMTIME_R=1)
  endif()
  if(LCMS2_DEFINE_HAVE_GMTIME_S)
    target_compile_definitions(${tgt} PRIVATE HAVE_GMTIME_S=1)
  endif()

  if(LCMS2_DEFINE_WORDS_BIGENDIAN)
    target_compile_definitions(${tgt} PRIVATE WORDS_BIGENDIAN=1)
  endif()

  if(LCMS2_DEFINE_CMS_DONT_USE_SSE2)
    target_compile_definitions(${tgt} PRIVATE CMS_DONT_USE_SSE2=1)
  endif()

  if(LCMS2_DEFINE_HAVE_FUNC_ATTRIBUTE_VISIBILITY)
    target_compile_definitions(${tgt} PRIVATE HAVE_FUNC_ATTRIBUTE_VISIBILITY=1)
  endif()

  if(LCMS2_USE_FLAG_FVISIBILITY_HIDDEN AND NOT WIN32)
    target_compile_options(${tgt} PRIVATE "-fvisibility=hidden")
  endif()

  # libm (primarily needed on Linux). Make it PUBLIC so dependents (tools)
  # also link it, matching autotools *_DEPLIBS behavior.
  find_library(_lcms2_math_lib m)
  if(_lcms2_math_lib)
    target_link_libraries(${tgt} PUBLIC "${_lcms2_math_lib}")
  endif()
endfunction()

function(_lcms2_apply_thread_settings tgt)
  # Autotools defines HasTHREADS (0/1) on non-Windows only. On Windows, upstream
  # VC projects do not define HasTHREADS, leaving Win32 critical-section path.
  if(WIN32)
    return()
  endif()

  if(NOT LCMS2_WITH_THREADS)
    target_compile_definitions(${tgt} PRIVATE HasTHREADS=0)
    return()
  endif()

  find_package(Threads)
  if(Threads_FOUND)
    target_compile_definitions(${tgt} PRIVATE HasTHREADS=1)
    if(TARGET Threads::Threads)
      # PUBLIC so dependents also pick up pthread flags/libs when needed.
      target_link_libraries(${tgt} PUBLIC Threads::Threads)
    elseif(CMAKE_THREAD_LIBS_INIT)
      target_link_libraries(${tgt} PUBLIC "${CMAKE_THREAD_LIBS_INIT}")
    endif()
  else()
    target_compile_definitions(${tgt} PRIVATE HasTHREADS=0)
  endif()
endfunction()

function(lcms2_add_library)
  _lcms2_collect_sources(_lcms2_sources)

  set(_lcms2_shared_target "")
  set(_lcms2_static_target "")

  if(LCMS2_BUILD_SHARED)
    add_library(lcms2 SHARED ${_lcms2_sources})
    set(_lcms2_shared_target lcms2)

    if(WIN32)
      target_compile_definitions(lcms2 PRIVATE CMS_DLL_BUILD)
      target_compile_definitions(lcms2 INTERFACE CMS_DLL)
    endif()

    set_target_properties(lcms2 PROPERTIES
      VERSION "${PROJECT_VERSION}.0"
      SOVERSION "2"
    )

    _lcms2_apply_common_settings(lcms2)
    _lcms2_apply_thread_settings(lcms2)
  endif()

  if(LCMS2_BUILD_STATIC)
    if(LCMS2_BUILD_SHARED AND WIN32)
      add_library(lcms2_static STATIC ${_lcms2_sources})
      set(_lcms2_static_target lcms2_static)
      set_target_properties(lcms2_static PROPERTIES OUTPUT_NAME "lcms2_static")
    else()
      if(LCMS2_BUILD_SHARED)
        add_library(lcms2_static STATIC ${_lcms2_sources})
        set(_lcms2_static_target lcms2_static)
        set_target_properties(lcms2_static PROPERTIES OUTPUT_NAME "lcms2")
      else()
        add_library(lcms2 STATIC ${_lcms2_sources})
        set(_lcms2_static_target lcms2)
      endif()
    endif()

    set(_static_tgt "${_lcms2_static_target}")
    set_target_properties(${_static_tgt} PROPERTIES
      VERSION "${PROJECT_VERSION}.0"
      SOVERSION "2"
    )

    _lcms2_apply_common_settings(${_static_tgt})
    _lcms2_apply_thread_settings(${_static_tgt})
  endif()

  # Expose target names for other modules.
  if(_lcms2_shared_target)
    set(LCMS2_LIBRARY_TARGET "${_lcms2_shared_target}" PARENT_SCOPE)
  else()
    set(LCMS2_LIBRARY_TARGET "${_lcms2_static_target}" PARENT_SCOPE)
  endif()

  if(_lcms2_static_target)
    set(LCMS2_STATIC_LIBRARY_TARGET "${_lcms2_static_target}" PARENT_SCOPE)
  endif()
endfunction()



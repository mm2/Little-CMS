function(lcms2_setup_options)
  # Core build toggles (explicitly support building both shared and static).
  option(LCMS2_BUILD_SHARED "Build shared library" ON)
  option(LCMS2_BUILD_STATIC "Build static library" ON)

  option(LCMS2_BUILD_TOOLS "Build command-line tools" ON)
  option(LCMS2_BUILD_TESTS "Build tests (testbed)" ON)

  # Tool-specific toggles (only meaningful when LCMS2_BUILD_TOOLS=ON).
  option(LCMS2_BUILD_JPGICC "Build jpgicc tool (requires JPEG)" ON)
  option(LCMS2_BUILD_TIFICC "Build tificc tool (requires TIFF, optionally ZLIB)" ON)

  # Dependency toggles (only consulted when the dependent tool is enabled).
  option(LCMS2_WITH_JPEG "Enable JPEG support for tools (find_package only when needed)" ON)
  option(LCMS2_WITH_TIFF "Enable TIFF support for tools (find_package only when needed)" ON)
  option(LCMS2_WITH_ZLIB "Enable ZLIB support for tools (find_package only when needed)" ON)

  # Threads (POSIX pthreads on non-Windows).
  option(LCMS2_WITH_THREADS "Enable thread support where applicable" ON)

  # Plugins (GPL).
  option(LCMS2_WITH_FASTFLOAT "Build and install fast_float plugin (GPL-3.0)" OFF)
  option(LCMS2_WITH_THREADED_PLUGIN "Build and install threaded plugin (GPL-3.0)" OFF)

  # Default debug postfix for MSVC builds.
  if(MSVC AND NOT DEFINED CMAKE_DEBUG_POSTFIX)
    set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "Debug postfix for binaries and libraries")
  endif()

  if(NOT LCMS2_BUILD_SHARED AND NOT LCMS2_BUILD_STATIC)
    message(FATAL_ERROR "At least one of LCMS2_BUILD_SHARED or LCMS2_BUILD_STATIC must be ON")
  endif()
endfunction()



include(GNUInstallDirs)
include(CMakeParseArguments)

function(_lcms2_add_tool exe_name)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs SOURCES)
  cmake_parse_arguments(TOOL "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  add_executable(${exe_name} ${TOOL_SOURCES})

  target_include_directories(${exe_name}
    PRIVATE
      "${PROJECT_SOURCE_DIR}/include"
      "${PROJECT_SOURCE_DIR}/utils/common"
  )

  if(DEFINED LCMS2_LIBRARY_TARGET)
    target_link_libraries(${exe_name} PRIVATE "${LCMS2_LIBRARY_TARGET}")
  endif()

  set(_lcms2_tool_targets "${LCMS2_TOOL_TARGETS}")
  list(APPEND _lcms2_tool_targets "${exe_name}")
  set(LCMS2_TOOL_TARGETS "${_lcms2_tool_targets}" PARENT_SCOPE)
endfunction()

function(lcms2_add_tools)
  if(NOT LCMS2_BUILD_TOOLS)
    return()
  endif()

  # Populated by _lcms2_add_tool() via PARENT_SCOPE into this function scope.
  set(LCMS2_TOOL_TARGETS "")

  set(_common_sources
    "${PROJECT_SOURCE_DIR}/utils/common/xgetopt.c"
    "${PROJECT_SOURCE_DIR}/utils/common/vprf.c"
    "${PROJECT_SOURCE_DIR}/utils/common/utils.h"
  )

  # Always-built tools (per autotools).
  _lcms2_add_tool(transicc
    SOURCES
      "${PROJECT_SOURCE_DIR}/utils/transicc/transicc.c"
      ${_common_sources}
  )
  set_property(TARGET transicc PROPERTY OUTPUT_NAME "transicc")

  _lcms2_add_tool(linkicc
    SOURCES
      "${PROJECT_SOURCE_DIR}/utils/linkicc/linkicc.c"
      ${_common_sources}
  )
  set_property(TARGET linkicc PROPERTY OUTPUT_NAME "linkicc")

  _lcms2_add_tool(psicc
    SOURCES
      "${PROJECT_SOURCE_DIR}/utils/psicc/psicc.c"
      ${_common_sources}
  )
  set_property(TARGET psicc PROPERTY OUTPUT_NAME "psicc")

  # Optional jpgicc (requires JPEG).
  if(LCMS2_BUILD_JPGICC)
    if(LCMS2_WITH_JPEG)
      find_package(JPEG)
    endif()

    if(JPEG_FOUND)
      _lcms2_add_tool(jpgicc
        SOURCES
          "${PROJECT_SOURCE_DIR}/utils/jpgicc/jpgicc.c"
          "${PROJECT_SOURCE_DIR}/utils/jpgicc/iccjpeg.c"
          ${_common_sources}
      )
      if(TARGET JPEG::JPEG)
        target_link_libraries(jpgicc PRIVATE JPEG::JPEG)
      else()
        # Do not quote list variables: they may contain debug/optimized keywords
        # on multi-config generators (e.g. Ninja Multi-Config).
        target_include_directories(jpgicc PRIVATE ${JPEG_INCLUDE_DIRS})
        target_link_libraries(jpgicc PRIVATE ${JPEG_LIBRARIES})
      endif()
    else()
      message(STATUS "JPEG not found or disabled; skipping jpgicc")
    endif()
  endif()

  # Optional tificc (requires TIFF; may also use ZLIB/JPEG if present).
  if(LCMS2_BUILD_TIFICC)
    if(LCMS2_WITH_TIFF)
      find_package(TIFF)
    endif()

    if(TIFF_FOUND)
      _lcms2_add_tool(tificc
        SOURCES
          "${PROJECT_SOURCE_DIR}/utils/tificc/tificc.c"
          ${_common_sources}
      )
      if(TARGET TIFF::TIFF)
        target_link_libraries(tificc PRIVATE TIFF::TIFF)
      else()
        # Do not quote list variables: they may contain debug/optimized keywords.
        target_include_directories(tificc PRIVATE ${TIFF_INCLUDE_DIR})
        target_link_libraries(tificc PRIVATE ${TIFF_LIBRARIES})
      endif()

      if(LCMS2_WITH_ZLIB)
        find_package(ZLIB)
        if(ZLIB_FOUND)
          if(TARGET ZLIB::ZLIB)
            target_link_libraries(tificc PRIVATE ZLIB::ZLIB)
          else()
            target_include_directories(tificc PRIVATE ${ZLIB_INCLUDE_DIRS})
            target_link_libraries(tificc PRIVATE ${ZLIB_LIBRARIES})
          endif()
        else()
          message(STATUS "ZLIB not found or disabled; building tificc without ZLIB")
        endif()
      endif()

      if(LCMS2_WITH_JPEG)
        find_package(JPEG)
        if(JPEG_FOUND)
          if(TARGET JPEG::JPEG)
            target_link_libraries(tificc PRIVATE JPEG::JPEG)
          else()
            target_include_directories(tificc PRIVATE ${JPEG_INCLUDE_DIRS})
            target_link_libraries(tificc PRIVATE ${JPEG_LIBRARIES})
          endif()
        endif()
      endif()
    else()
      message(STATUS "TIFF not found or disabled; skipping tificc")
    endif()
  endif()

  # Manpages (install handled by packaging module).
  set(_manpages
    "${PROJECT_SOURCE_DIR}/utils/jpgicc/jpgicc.1"
    "${PROJECT_SOURCE_DIR}/utils/linkicc/linkicc.1"
    "${PROJECT_SOURCE_DIR}/utils/psicc/psicc.1"
    "${PROJECT_SOURCE_DIR}/utils/tificc/tificc.1"
    "${PROJECT_SOURCE_DIR}/utils/transicc/transicc.1"
  )
  set(LCMS2_TOOL_MANPAGES "${_manpages}" PARENT_SCOPE)

  # Propagate tool target list to the root scope so packaging can install them.
  set(LCMS2_TOOL_TARGETS "${LCMS2_TOOL_TARGETS}" PARENT_SCOPE)
endfunction()



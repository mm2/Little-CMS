cmake_minimum_required(VERSION 3.5)

project(lcms2
  VERSION 2.18
  LANGUAGES C
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(Lcms2Options)
include(Lcms2Features)
include(Lcms2Library)
include(Lcms2Tools)
include(Lcms2Plugins)
include(Lcms2Tests)
include(Lcms2Packaging)

lcms2_setup_options()
lcms2_detect_features()

if(LCMS2_BUILD_TESTS)
  enable_testing()
endif()

lcms2_add_library()

if(LCMS2_BUILD_TOOLS)
  lcms2_add_tools()
endif()

lcms2_add_plugins()

if(LCMS2_BUILD_TESTS)
  lcms2_add_tests()
endif()

lcms2_setup_packaging()



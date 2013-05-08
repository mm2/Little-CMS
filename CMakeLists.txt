##################################################################################
#
#  Little Color Management System
#  Copyright (c) 2013 Esteban Tovagliari
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the Software
# is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
# THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
##################################################################################
#

CMAKE_MINIMUM_REQUIRED( VERSION 2.8.0 FATAL_ERROR)

##################################################################################
# project

PROJECT( Little-CMS)

##################################################################################
# options

OPTION( BUILD_SHARED "Build shared libs"    ON)
OPTION( BUILD_UTILS  "Build utility tools"  ON)
OPTION( BUILD_TESTS  "Build tests"          ON)

##################################################################################
# cmake stuff

# disable in source builds
IF( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    MESSAGE(FATAL_ERROR "CMake generation for Little-CMS is not allowed within the source directory!")
ENDIF()

SET( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)

IF( NOT CMAKE_BUILD_TYPE)
	SET( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF()

##################################################################################
# platform

IF( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   # Mac OS X
ELSEIF( ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   # Linux
ELSEIF( ${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   # Windows
ELSE()
    MESSAGE( FATAL_ERROR "Platform ${CMAKE_SYSTEM_NAME} not supported")
ENDIF()

##################################################################################
# compiler flags

IF( CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX) # OSX and linux
    SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -std=gnu99")
ELSEIF( MSVC)
ELSE()
    #MESSAGE( FATAL_ERROR "Compiler ${CMAKE_CXX_COMPILER} not supported yet")
ENDIF()

##################################################################################
# dependencies

FIND_PACKAGE( JPEG)
FIND_PACKAGE( TIFF)
FIND_PACKAGE( ZLIB)

##################################################################################
# definitions

IF( JPEG_FOUND)
  ADD_DEFINITIONS( -DHasJPEG=1)
ENDIF()

IF( TIFF_FOUND)
  ADD_DEFINITIONS( -DHasTIFF=1)
ENDIF()

IF( ZLIB_FOUND)
  ADD_DEFINITIONS( -DHasZLIB=1)
ENDIF()

##################################################################################
# includes

INCLUDE_DIRECTORIES( include)

IF( JPEG_FOUND)
  INCLUDE_DIRECTORIES( ${JPEG_INCLUDE_DIR})
ENDIF()

IF( TIFF_FOUND)
  INCLUDE_DIRECTORIES( ${TIFF_INCLUDE_DIR})
ENDIF()

IF( ZLIB_FOUND)
  INCLUDE_DIRECTORIES( ${ZLIB_INCLUDE_DIRS})
ENDIF()

##################################################################################
# lcms lib

ADD_SUBDIRECTORY( src)

IF( BUILD_UTILS)
    ADD_SUBDIRECTORY( utils)
ENDIF()

IF( BUILD_TESTS)
    ADD_SUBDIRECTORY( testbed)
    ENABLE_TESTING()
ENDIF()

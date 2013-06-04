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

##################################################################################
# lcms2 library

FILE(GLOB_RECURSE LCMS2_INCLUDE_HEADERS ../include/*.h)
FILE(GLOB_RECURSE LCMS2_INTERNAL_HEADERS ./*.h)
FILE( GLOB LCMS2_SOURCES ./*.c)

IF( BUILD_SHARED)
	ADD_DEFINITIONS( -DCMS_DLL_BUILD)
	ADD_LIBRARY( lcms2 SHARED   ${LCMS2_INCLUDE_HEADERS}
                                    ${LCMS2_INTERNAL_HEADERS}
        	                    ${LCMS2_SOURCES}
                                    )

    SET_TARGET_PROPERTIES( lcms2 PROPERTIES VERSION "2.5")
    SET_TARGET_PROPERTIES( lcms2 PROPERTIES SOVERSION "2.5")
ELSE()
  ADD_LIBRARY( lcms2 ${LCMS2_INCLUDE_HEADERS}
                     ${LCMS2_INTERNAL_HEADERS}
                     ${LCMS2_SOURCES}
                     )
ENDIF()

INSTALL( DIRECTORY ../include DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
INSTALL( TARGETS lcms2 DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

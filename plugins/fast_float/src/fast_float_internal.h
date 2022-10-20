//---------------------------------------------------------------------------------
//
//  Little Color Management System, fast floating point extensions
//  Copyright (c) 1998-2022 Marti Maria Saguer, all rights reserved
//
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------------

#ifndef _FAST_INTERNAL_H
#define _FAST_INTERNAL_H

#include "lcms2_internal.h"
#include "lcms2_fast_float.h"
#include <stdint.h>

#define REQUIRED_LCMS_VERSION 2120

// Unused parameter warning suppression
#define UNUSED_PARAMETER(x) ((void)x) 

// For testbed
#define CMSCHECKPOINT CMSAPI

// The specification for "inline" is section 6.7.4 of the C99 standard (ISO/IEC 9899:1999).
// unfortunately VisualC++ does not conform that
#if defined(_MSC_VER) || defined(__BORLANDC__)
#   define cmsINLINE __inline
#else
#   define cmsINLINE static inline
#endif

/// Properly define some macros to accommodate
/// older MSVC versions.
# if defined(_MSC_VER) && _MSC_VER <= 1700
#include <float.h>
#define isnan _isnan
#define isinf(x) (!_finite((x)))
# endif

#if !defined(_MSC_VER) && (defined(__STDC_VERSION__) && __STDC_VERSION__ < 199901L)
#if !defined(isinf)
#define isinf(x) (!finite((x)))
#endif
#endif

// This macro return words stored as big endian
#define CHANGE_ENDIAN(w)    (cmsUInt16Number) ((cmsUInt16Number) ((w)<<8)|((w)>>8))

// This macro changes the polarity of a word
#define REVERSE_FLAVOR_16(x)    ((cmsUInt16Number)(0xffff-(x)))

// Fixed point
#define FIXED_TO_INT(x)         ((x)>>16)
#define FIXED_REST_TO_INT(x)    ((x)&0xFFFFU)

#define cmsFLAGS_CAN_CHANGE_FORMATTER     0x02000000   // Allow change buffer format

// This is the upper part of internal transform structure. Only format specifiers are used
typedef struct {

       cmsUInt32Number InputFormat, OutputFormat; // Keep formats for further reference

} _xform_head;


#define MAX_NODES_IN_CURVE 0x8001  

// To prevent out of bounds indexing
cmsINLINE cmsFloat32Number fclamp(cmsFloat32Number v)
{
    return ((v < 1.0e-9f) || isnan(v)) ? 0.0f : (v > 1.0f ? 1.0f : v);
}

// Floor to word, taking care of saturation. This is not critical in terms of performance
cmsINLINE cmsUInt16Number _cmsSaturateWord(cmsFloat64Number d)
{
       d += 0.5;

       if (d <= 0) return 0;
       if (d >= 65535.0) return 0xffff;

       return (cmsUInt16Number)floor(d);
}


cmsINLINE cmsFloat32Number flerp(const cmsFloat32Number LutTable[], cmsFloat32Number v)
{
       cmsFloat32Number y1, y0;
       cmsFloat32Number rest;
       int cell0, cell1;
      
       if ((v < 1.0e-9f) || isnan(v)) {
              return LutTable[0];
       }
       else
              if (v >= 1.0) {
              return LutTable[MAX_NODES_IN_CURVE - 1];
              }

       v *= (MAX_NODES_IN_CURVE - 1);

       cell0 = _cmsQuickFloor(v);
       cell1 = (int)ceilf(v);

       // Rest is 16 LSB bits
       rest = v - cell0;

       y0 = LutTable[cell0];
       y1 = LutTable[cell1];

       return y0 + (y1 - y0) * rest;
}



// Some secret sauce from lcms
CMSAPI cmsUInt32Number  CMSEXPORT _cmsReasonableGridpointsByColorspace(cmsColorSpaceSignature Colorspace, cmsUInt32Number dwFlags);



// Compute the increments to be used by the transform functions
CMSCHECKPOINT void CMSEXPORT _cmsComputeComponentIncrements(cmsUInt32Number Format,
                                                            cmsUInt32Number BytesPerPlane,
                                                            cmsUInt32Number* nChannels,
                                                            cmsUInt32Number* nAlpha,
                                                            cmsUInt32Number ComponentStartingOrder[],
                                                            cmsUInt32Number ComponentPointerIncrements[]);

// 15 bits formatters
CMSCHECKPOINT cmsFormatter CMSEXPORT Formatter_15Bit_Factory(cmsUInt32Number Type,
                                                             cmsFormatterDirection Dir,
                                                             cmsUInt32Number dwFlags);

// Optimizers

//  8 bits on input allows matrix-shaper boost up a little bit
cmsBool Optimize8MatrixShaper(_cmsTransform2Fn* TransformFn,
                              void** UserData,
                              _cmsFreeUserDataFn* FreeUserData,
                              cmsPipeline** Lut,
                              cmsUInt32Number* InputFormat,
                              cmsUInt32Number* OutputFormat,
                              cmsUInt32Number* dwFlags);

//  8 bits using SSE
cmsBool Optimize8MatrixShaperSSE(_cmsTransform2Fn* TransformFn,
                              void** UserData,
                              _cmsFreeUserDataFn* FreeUserData,
                              cmsPipeline** Lut,
                              cmsUInt32Number* InputFormat,
                              cmsUInt32Number* OutputFormat,
                              cmsUInt32Number* dwFlags);

cmsBool OptimizeMatrixShaper15(_cmsTransform2Fn* TransformFn,
                               void** UserData,
                               _cmsFreeUserDataFn* FreeUserData,
                               cmsPipeline** Lut,
                               cmsUInt32Number* InputFormat,
                               cmsUInt32Number* OutputFormat,
                               cmsUInt32Number* dwFlags);


cmsBool Optimize8ByJoiningCurves(_cmsTransform2Fn* TransformFn,
                                 void** UserData,
                                 _cmsFreeUserDataFn* FreeUserData,
                                 cmsPipeline** Lut,
                                 cmsUInt32Number* InputFormat,
                                 cmsUInt32Number* OutputFormat,
                                 cmsUInt32Number* dwFlags);

cmsBool OptimizeFloatByJoiningCurves(_cmsTransform2Fn* TransformFn,                                  
                                   void** UserData,
                                   _cmsFreeUserDataFn* FreeUserData,
                                   cmsPipeline** Lut, 
                                   cmsUInt32Number* InputFormat, 
                                   cmsUInt32Number* OutputFormat, 
                                   cmsUInt32Number* dwFlags);    

cmsBool OptimizeFloatMatrixShaper(_cmsTransform2Fn* TransformFn,                                  
                                   void** UserData,
                                   _cmsFreeUserDataFn* FreeUserData,
                                   cmsPipeline** Lut, 
                                   cmsUInt32Number* InputFormat, 
                                   cmsUInt32Number* OutputFormat, 
                                   cmsUInt32Number* dwFlags);

cmsBool Optimize8BitRGBTransform(_cmsTransform2Fn* TransformFn,
                                   void** UserData,
                                   _cmsFreeUserDataFn* FreeDataFn,
                                   cmsPipeline** Lut,
                                   cmsUInt32Number* InputFormat,
                                   cmsUInt32Number* OutputFormat,
                                   cmsUInt32Number* dwFlags);

cmsBool Optimize16BitRGBTransform(_cmsTransform2Fn* TransformFn,
                                   void** UserData,
                                   _cmsFreeUserDataFn* FreeDataFn,
                                   cmsPipeline** Lut,
                                   cmsUInt32Number* InputFormat,
                                   cmsUInt32Number* OutputFormat,
                                   cmsUInt32Number* dwFlags);

cmsBool OptimizeCLUTRGBTransform(_cmsTransform2Fn* TransformFn,
                                  void** UserData,
                                  _cmsFreeUserDataFn* FreeDataFn,
                                  cmsPipeline** Lut, 
                                  cmsUInt32Number* InputFormat, 
                                  cmsUInt32Number* OutputFormat, 
                                  cmsUInt32Number* dwFlags);      

cmsBool OptimizeCLUTCMYKTransform(_cmsTransform2Fn* TransformFn,
					              void** UserData,
					              _cmsFreeUserDataFn* FreeDataFn,
					              cmsPipeline** Lut,
					              cmsUInt32Number* InputFormat,
					              cmsUInt32Number* OutputFormat,
					              cmsUInt32Number* dwFlags);


cmsBool OptimizeCLUTLabTransform(_cmsTransform2Fn* TransformFn,
                                 void** UserData,
                                 _cmsFreeUserDataFn* FreeDataFn,
                                 cmsPipeline** Lut, 
                                 cmsUInt32Number* InputFormat, 
                                 cmsUInt32Number* OutputFormat, 
                                 cmsUInt32Number* dwFlags);      


#endif

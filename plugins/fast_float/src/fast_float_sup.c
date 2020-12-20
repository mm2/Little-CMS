//---------------------------------------------------------------------------------
//
//  Little Color Management System, fast floating point extensions
//  Copyright (c) 1998-2020 Marti Maria Saguer, all rights reserved
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


#include "fast_float_internal.h"


// This is the main dispatcher
static
cmsBool Floating_Point_Transforms_Dispatcher(_cmsTransform2Fn* TransformFn,
                                  void** UserData,
                                  _cmsFreeUserDataFn* FreeUserData,
                                  cmsPipeline** Lut, 
                                  cmsUInt32Number* InputFormat, 
                                  cmsUInt32Number* OutputFormat, 
                                  cmsUInt32Number* dwFlags) 
{

    // Try to optimize as a set of curves plus a matrix plus a set of curves
    if (OptimizeMatrixShaper15(TransformFn, UserData, FreeUserData, Lut, InputFormat, OutputFormat, dwFlags)) return TRUE;

    // Try to optimize by joining curves
    if (Optimize8ByJoiningCurves(TransformFn, UserData, FreeUserData, Lut, InputFormat, OutputFormat, dwFlags)) return TRUE;

#ifndef CMS_DONT_USE_SSE2
    // Try to use SSE2 to optimize as a set of curves plus a matrix plus a set of curves
    if (Optimize8MatrixShaperSSE(TransformFn, UserData, FreeUserData, Lut, InputFormat, OutputFormat, dwFlags)) return TRUE;
#endif
    // Try to optimize as a set of curves plus a matrix plus a set of curves
    if (Optimize8MatrixShaper(TransformFn, UserData, FreeUserData, Lut, InputFormat, OutputFormat, dwFlags)) return TRUE;

    // Try to optimize by joining curves
    if (OptimizeFloatByJoiningCurves(TransformFn, UserData, FreeUserData, Lut, InputFormat, OutputFormat, dwFlags)) return TRUE;

    // Try to optimize as a set of curves plus a matrix plus a set of curves
    if (OptimizeFloatMatrixShaper(TransformFn, UserData, FreeUserData, Lut, InputFormat, OutputFormat, dwFlags)) return TRUE;

    // Try to optimize using prelinearization plus tetrahedral
    if (Optimize8BitRGBTransform(TransformFn, UserData, FreeUserData, Lut, InputFormat, OutputFormat, dwFlags)) return TRUE;

    // Try to optimize using prelinearization plus tetrahedral
    if (Optimize16BitRGBTransform(TransformFn, UserData, FreeUserData, Lut, InputFormat, OutputFormat, dwFlags)) return TRUE;

    // Try to optimize using prelinearization plus tetrahedral
    if (OptimizeCLUTRGBTransform(TransformFn, UserData, FreeUserData, Lut, InputFormat, OutputFormat, dwFlags)) return TRUE;

    // Try to optimize using prelinearization plus tetrahedral
    if (OptimizeCLUTCMYKTransform(TransformFn, UserData, FreeUserData, Lut, InputFormat, OutputFormat, dwFlags)) return TRUE;

    // Try to optimize for Lab float as input
    if (OptimizeCLUTLabTransform(TransformFn, UserData, FreeUserData, Lut, InputFormat, OutputFormat, dwFlags)) return TRUE;

    // Cannot optimize, use lcms normal process 
    return FALSE;
}

// The Plug-in entry points
static cmsPluginFormatters PluginFastFloat = {
              { cmsPluginMagicNumber, REQUIRED_LCMS_VERSION, cmsPluginFormattersSig, NULL },

              Formatter_15Bit_Factory
};

static cmsPluginTransform PluginList = {

              { cmsPluginMagicNumber, REQUIRED_LCMS_VERSION, cmsPluginTransformSig, (cmsPluginBase *) &PluginFastFloat },

              // When initializing a union, the initializer list must have only one member, which initializes the first member of 
              // the union unless a designated initializer is used (C99)

              { (_cmsTransformFactory) Floating_Point_Transforms_Dispatcher }
};

// This is the main plug-in installer. 
// Using a function to retrieve the plug-in entry point allows us to execute initialization data. 
void* CMSEXPORT cmsFastFloatExtensions(void)
{
       return (void*)&PluginList;
}


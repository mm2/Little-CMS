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

// Optimization for floating point tetrahedral interpolation
typedef struct {

    cmsContext ContextID;
    const cmsInterpParams* p;   // Tetrahedrical interpolation parameters. This is a not-owned pointer.

} FloatCLUTData;

// Precomputes tables for 8-bit on input devicelink. 
static
FloatCLUTData* FloatCLUTAlloc(cmsContext ContextID, const cmsInterpParams* p)
{
    FloatCLUTData* fd;

    fd = (FloatCLUTData*) _cmsMallocZero(ContextID, sizeof(FloatCLUTData));
    if (fd == NULL) return NULL;
    
    fd ->ContextID = ContextID;
    fd ->p = p;
    return fd;
}


// Sampler implemented by another LUT. This is a clean way to precalculate the devicelink 3D CLUT for 
// almost any transform. We use floating point precision and then convert from floating point to 16 bits.
static
int XFormSampler(register const cmsFloat32Number In[], register cmsFloat32Number Out[], register void* Cargo)
{
    // Evaluate in 16 bits
    cmsPipelineEvalFloat(In, Out, (cmsPipeline*) Cargo);

    // Always succeed
    return TRUE;
}



// A optimized interpolation for 8-bit input.
#define DENS(i,j,k) (LutTable[(i)+(j)+(k)+OutChan])

static
void FloatCLUTEval(struct _cmstransform_struct *CMMcargo,
                      const cmsFloat32Number* Input,
                      cmsFloat32Number* Output,
                      cmsUInt32Number len,
                      cmsUInt32Number Stride)
{
    
    cmsFloat32Number        r, g, b;
    cmsFloat32Number        px, py, pz;
    int                     x0, y0, z0;
    int                     X0, Y0, Z0, X1, Y1, Z1;
    cmsFloat32Number        rx, ry, rz;
    cmsFloat32Number        c0, c1 = 0, c2 = 0, c3 = 0;

    cmsUInt32Number         OutChan;
    FloatCLUTData*          p8 = (FloatCLUTData*) _cmsGetTransformUserData(CMMcargo);

    const cmsInterpParams*  p = p8 ->p;
    cmsUInt32Number        TotalOut = p -> nOutputs;
    const cmsFloat32Number* LutTable = (const cmsFloat32Number*)p->Table;
    cmsUInt32Number        ii;
    const cmsUInt8Number*  rin;
    const cmsUInt8Number*  gin;
    const cmsUInt8Number*  bin;

    cmsUInt8Number* out[cmsMAXCHANNELS];
    cmsUInt32Number SourceStartingOrder[cmsMAXCHANNELS];
    cmsUInt32Number SourceIncrements[cmsMAXCHANNELS];
    cmsUInt32Number DestStartingOrder[cmsMAXCHANNELS];
    cmsUInt32Number DestIncrements[cmsMAXCHANNELS];

    cmsUInt32Number InputFormat  = cmsGetTransformInputFormat((cmsHTRANSFORM) CMMcargo);
    cmsUInt32Number OutputFormat = cmsGetTransformOutputFormat((cmsHTRANSFORM) CMMcargo);

    cmsUInt32Number nchans, nalpha;

    _cmsComputeComponentIncrements(InputFormat, Stride, &nchans, &nalpha, SourceStartingOrder, SourceIncrements);
    _cmsComputeComponentIncrements(OutputFormat, Stride, &nchans, &nalpha, DestStartingOrder, DestIncrements);

    // SeparateRGB(InputFormat, Stride,  SourceStartingOrder, SourceIncrements);
    // SeparateRGB(OutputFormat, Stride, DestStartingOrder, DestIncrements);

    rin = (const cmsUInt8Number*)Input + SourceStartingOrder[0];
    gin = (const cmsUInt8Number*)Input + SourceStartingOrder[1];
    bin = (const cmsUInt8Number*)Input + SourceStartingOrder[2];

    for (ii=0; ii < TotalOut; ii++) 
           out[ii] = (cmsUInt8Number*) Output + DestStartingOrder[ii];

    for (ii=0; ii < len; ii++) {
            
           r = fclamp(*(cmsFloat32Number*)rin);
           g = fclamp(*(cmsFloat32Number*)gin);
           b = fclamp(*(cmsFloat32Number*)bin);

        rin += SourceIncrements[0];
        gin += SourceIncrements[1];
        bin += SourceIncrements[2];
       
        px = r * p->Domain[0];
        py = g * p->Domain[1];
        pz = b * p->Domain[2];

        
        x0 = (int)_cmsQuickFloor(px); rx = (px - (cmsFloat32Number)x0);
        y0 = (int)_cmsQuickFloor(py); ry = (py - (cmsFloat32Number)y0);
        z0 = (int)_cmsQuickFloor(pz); rz = (pz - (cmsFloat32Number)z0);
   
        X0 = p->opta[2] * x0;
        X1 = X0 + (r >= 1.0 ? 0 : p->opta[2]);

        Y0 = p->opta[1] * y0;
        Y1 = Y0 + (g >= 1.0 ? 0 : p->opta[1]);

        Z0 = p->opta[0] * z0;
        Z1 = Z0 + (b >= 1.0 ? 0 : p->opta[0]);
      
        for (OutChan = 0; OutChan < TotalOut; OutChan++) {

               // These are the 6 Tetrahedral

               c0 = DENS(X0, Y0, Z0);

               if (rx >= ry && ry >= rz) {

                      c1 = DENS(X1, Y0, Z0) - c0;
                      c2 = DENS(X1, Y1, Z0) - DENS(X1, Y0, Z0);
                      c3 = DENS(X1, Y1, Z1) - DENS(X1, Y1, Z0);

               }
               else
                      if (rx >= rz && rz >= ry) {

                             c1 = DENS(X1, Y0, Z0) - c0;
                             c2 = DENS(X1, Y1, Z1) - DENS(X1, Y0, Z1);
                             c3 = DENS(X1, Y0, Z1) - DENS(X1, Y0, Z0);

                      }
                      else
                             if (rz >= rx && rx >= ry) {

                                    c1 = DENS(X1, Y0, Z1) - DENS(X0, Y0, Z1);
                                    c2 = DENS(X1, Y1, Z1) - DENS(X1, Y0, Z1);
                                    c3 = DENS(X0, Y0, Z1) - c0;

                             }
                             else
                                    if (ry >= rx && rx >= rz) {

                                           c1 = DENS(X1, Y1, Z0) - DENS(X0, Y1, Z0);
                                           c2 = DENS(X0, Y1, Z0) - c0;
                                           c3 = DENS(X1, Y1, Z1) - DENS(X1, Y1, Z0);

                                    }
                                    else
                                           if (ry >= rz && rz >= rx) {

                                                  c1 = DENS(X1, Y1, Z1) - DENS(X0, Y1, Z1);
                                                  c2 = DENS(X0, Y1, Z0) - c0;
                                                  c3 = DENS(X0, Y1, Z1) - DENS(X0, Y1, Z0);

                                           }
                                           else
                                                  if (rz >= ry && ry >= rx) {

                                                         c1 = DENS(X1, Y1, Z1) - DENS(X0, Y1, Z1);
                                                         c2 = DENS(X0, Y1, Z1) - DENS(X0, Y0, Z1);
                                                         c3 = DENS(X0, Y0, Z1) - c0;

                                                  }
                                                  else  {
                                                         c1 = c2 = c3 = 0;
                                                  }

                                                  *(cmsFloat32Number*) (out[OutChan]) = c0 + c1 * rx + c2 * ry + c3 * rz;
                                                 
                                                  out[OutChan] += DestIncrements[OutChan];

        }


    }
}

#undef DENS



// --------------------------------------------------------------------------------------------------------------

cmsBool OptimizeCLUTRGBTransform(_cmsTransformFn* TransformFn,
                                  void** UserData,
                                  _cmsFreeUserDataFn* FreeDataFn,
                                  cmsPipeline** Lut, 
                                  cmsUInt32Number* InputFormat, 
                                  cmsUInt32Number* OutputFormat, 
                                  cmsUInt32Number* dwFlags)      
{
    cmsPipeline* OriginalLut;
    int nGridPoints;    
    cmsPipeline* OptimizedLUT = NULL;    
    cmsStage* OptimizedCLUTmpe;
    cmsColorSpaceSignature OutputColorSpace;    
    cmsStage* mpe;
    FloatCLUTData* p8;
    cmsContext ContextID;
    _cmsStageCLutData* data;

    // For empty transforms, do nothing
    if (*Lut == NULL) return FALSE;

    // This is a loosy optimization! does not apply in floating-point cases
    if (!T_FLOAT(*InputFormat) || !T_FLOAT(*OutputFormat)) return FALSE;

    // Only on 8-bit
    if (T_BYTES(*InputFormat) != 4 || T_BYTES(*OutputFormat) != 4) return FALSE;

    // Only on RGB
    if (T_COLORSPACE(*InputFormat)  != PT_RGB) return FALSE;
    if (T_COLORSPACE(*OutputFormat) != PT_RGB) return FALSE;
  
    OriginalLut = *Lut;

   // Named color pipelines cannot be optimized either
   for (mpe = cmsPipelineGetPtrToFirstStage(OriginalLut);
         mpe != NULL;
         mpe = cmsStageNext(mpe)) {
            if (cmsStageType(mpe) == cmsSigNamedColorElemType) return FALSE;
    }

    ContextID = cmsGetPipelineContextID(OriginalLut);
    OutputColorSpace = _cmsICCcolorSpace(T_COLORSPACE(*OutputFormat));
    nGridPoints      = _cmsReasonableGridpointsByColorspace(cmsSigRgbData, *dwFlags);
             
    // Create the result LUT
    OptimizedLUT = cmsPipelineAlloc(cmsGetPipelineContextID(OriginalLut), 3, cmsPipelineOutputChannels(OriginalLut));
    if (OptimizedLUT == NULL) goto Error;

    
    // Allocate the CLUT for result
    OptimizedCLUTmpe = cmsStageAllocCLutFloat(ContextID, nGridPoints, 3, cmsPipelineOutputChannels(OriginalLut), NULL);

    // Add the CLUT to the destination LUT
    cmsPipelineInsertStage(OptimizedLUT, cmsAT_BEGIN, OptimizedCLUTmpe);

    // Resample the LUT
    if (!cmsStageSampleCLutFloat(OptimizedCLUTmpe, XFormSampler, (void*)OriginalLut, 0)) goto Error;

    // Set the evaluator, copy parameters   
    data = (_cmsStageCLutData*) cmsStageData(OptimizedCLUTmpe);

    p8 = FloatCLUTAlloc(ContextID, data ->Params);
    if (p8 == NULL) return FALSE;   

    // And return the obtained LUT
    cmsPipelineFree(OriginalLut);

    *Lut = OptimizedLUT;
    *TransformFn = (_cmsTransformFn) FloatCLUTEval;
    *UserData   = p8;
    *FreeDataFn = _cmsFree;
    *dwFlags &= ~cmsFLAGS_CAN_CHANGE_FORMATTER;
    return TRUE;

Error:
      
    if (OptimizedLUT != NULL) cmsPipelineFree(OptimizedLUT);

    return FALSE;    
}


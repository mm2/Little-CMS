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

} FloatCMYKData;



// Precomputes tables on input devicelink. 
static
FloatCMYKData* FloatCMYKAlloc(cmsContext ContextID, const cmsInterpParams* p)
{
    FloatCMYKData* fd;

    fd = (FloatCMYKData*) _cmsMallocZero(ContextID, sizeof(FloatCMYKData));
    if (fd == NULL) return NULL;
    
    fd ->ContextID = ContextID;
    fd ->p = p;
    return fd;
}


static
int XFormSampler(register const cmsFloat32Number In[], register cmsFloat32Number Out[], register void* Cargo)
{
    // Evaluate in 16 bits
    cmsPipelineEvalFloat(In, Out, (cmsPipeline*) Cargo);

    // Always succeed
    return TRUE;
}

cmsINLINE cmsFloat32Number LinearInterpInt(cmsFloat32Number a, cmsFloat32Number l, cmsFloat32Number h)
{
       return (h - l) * a + l;       
}

// To prevent out of bounds indexing
cmsINLINE cmsFloat32Number fclamp100(cmsFloat32Number v)
{
       return v < 0.0f ? 0.0f : (v > 100.0f ? 100.0f : v);
}


// A optimized interpolation for 8-bit input.
#define DENS(i,j,k) (LutTable[(i)+(j)+(k)+OutChan])

static
void FloatCMYKCLUTEval(struct _cmstransform_struct *CMMcargo,
                      const cmsFloat32Number* Input,
                      cmsFloat32Number* Output,
                      cmsUInt32Number len,
                      cmsUInt32Number Stride)
{
    
    cmsFloat32Number        c, m, y, k;
    cmsFloat32Number        px, py, pz, pk;
    int                     x0, y0, z0, k0;
    int                     X0, Y0, Z0, K0, X1, Y1, Z1, K1;
    cmsFloat32Number        rx, ry, rz, rk;
    cmsFloat32Number        c0, c1 = 0, c2 = 0, c3 = 0;

    cmsUInt32Number         OutChan;
    FloatCMYKData*          p8 = (FloatCMYKData*) _cmsGetTransformUserData(CMMcargo);

    const cmsInterpParams*  p = p8 ->p;
    cmsUInt32Number        TotalOut = p -> nOutputs;
    const cmsFloat32Number* LutTable = (const cmsFloat32Number*)p->Table;
    cmsUInt32Number        ii;
    const cmsUInt8Number*  cin;
    const cmsUInt8Number*  min;
    const cmsUInt8Number*  yin;
    const cmsUInt8Number*  kin;

    cmsFloat32Number        Tmp1[cmsMAXCHANNELS], Tmp2[cmsMAXCHANNELS];

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

    // SeparateCMYK(InputFormat, Stride,  SourceStartingOrder, SourceIncrements);
    // SeparateCMYK(OutputFormat, Stride, DestStartingOrder, DestIncrements);

    cin = (const cmsUInt8Number*)Input + SourceStartingOrder[0];
    min = (const cmsUInt8Number*)Input + SourceStartingOrder[1];
    yin = (const cmsUInt8Number*)Input + SourceStartingOrder[2];
    kin = (const cmsUInt8Number*)Input + SourceStartingOrder[3];

    for (ii=0; ii < TotalOut; ii++) 
           out[ii] = (cmsUInt8Number*)Output + DestStartingOrder[ii];

    for (ii=0; ii < len; ii++) {
            
           c = fclamp100(*(cmsFloat32Number*)cin) / 100.0f;
           m = fclamp100(*(cmsFloat32Number*)min) / 100.0f;
           y = fclamp100(*(cmsFloat32Number*)yin) / 100.0f;
           k = fclamp100(*(cmsFloat32Number*)kin) / 100.0f;

           cin += SourceIncrements[0];
           min += SourceIncrements[1];
           yin += SourceIncrements[2];
           kin += SourceIncrements[3];

           pk = c * p->Domain[0];  // C
           px = m * p->Domain[1];  // M 
           py = y * p->Domain[2];  // Y 
           pz = k * p->Domain[3];  // K


           k0 = (int)_cmsQuickFloor(pk); rk = (pk - (cmsFloat32Number)k0);
           x0 = (int)_cmsQuickFloor(px); rx = (px - (cmsFloat32Number)x0);
           y0 = (int)_cmsQuickFloor(py); ry = (py - (cmsFloat32Number)y0);
           z0 = (int)_cmsQuickFloor(pz); rz = (pz - (cmsFloat32Number)z0);


           K0 = p->opta[3] * k0;
           K1 = K0 + (c >= 1.0 ? 0 : p->opta[3]);

           X0 = p->opta[2] * x0;
           X1 = X0 + (m >= 1.0 ? 0 : p->opta[2]);

           Y0 = p->opta[1] * y0;
           Y1 = Y0 + (y >= 1.0 ? 0 : p->opta[1]);

           Z0 = p->opta[0] * z0;
           Z1 = Z0 + (k >= 1.0 ? 0 : p->opta[0]);

           for (OutChan = 0; OutChan < TotalOut; OutChan++) {
               
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


             Tmp1[OutChan] = c0 + c1 * rx + c2 * ry + c3 * rz;
        
        }


        LutTable = (cmsFloat32Number*)p->Table;
        LutTable += K1;

        for (OutChan = 0; OutChan < p->nOutputs; OutChan++) {

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

                                                  Tmp2[OutChan] = c0 + c1 * rx + c2 * ry + c3 * rz;
        }


        for (OutChan = 0; OutChan < p->nOutputs; OutChan++) {

               *(cmsFloat32Number*)(out[OutChan]) = LinearInterpInt(rk, Tmp1[OutChan], Tmp2[OutChan]);
               out[OutChan] += DestIncrements[OutChan];
        }


    }
}

#undef DENS



// --------------------------------------------------------------------------------------------------------------

cmsBool OptimizeCLUTCMYKTransform(_cmsTransformFn* TransformFn,
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
    FloatCMYKData* p8;
    cmsContext ContextID;
    _cmsStageCLutData* data;

    // For empty transforms, do nothing
    if (*Lut == NULL) return FALSE;

    // This is a loosy optimization! does not apply in floating-point cases
    if (!T_FLOAT(*InputFormat) || !T_FLOAT(*OutputFormat)) return FALSE;

    // Only on 8-bit
    if (T_BYTES(*InputFormat) != 4 || T_BYTES(*OutputFormat) != 4) return FALSE;

    // Only on CMYK
    if (T_COLORSPACE(*InputFormat)  != PT_CMYK) return FALSE;
   
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
    OptimizedLUT = cmsPipelineAlloc(cmsGetPipelineContextID(OriginalLut), 4, cmsPipelineOutputChannels(OriginalLut));
    if (OptimizedLUT == NULL) goto Error;

    
    // Allocate the CLUT for result
    OptimizedCLUTmpe = cmsStageAllocCLutFloat(ContextID, nGridPoints, 4, cmsPipelineOutputChannels(OriginalLut), NULL);

    // Add the CLUT to the destination LUT
    cmsPipelineInsertStage(OptimizedLUT, cmsAT_BEGIN, OptimizedCLUTmpe);

    // Resample the LUT
    if (!cmsStageSampleCLutFloat(OptimizedCLUTmpe, XFormSampler, (void*)OriginalLut, 0)) goto Error;

    // Set the evaluator, copy parameters   
    data = (_cmsStageCLutData*) cmsStageData(OptimizedCLUTmpe);

    p8 = FloatCMYKAlloc(ContextID, data ->Params);
    if (p8 == NULL) return FALSE;   

    // And return the obtained LUT
    cmsPipelineFree(OriginalLut);

    *Lut = OptimizedLUT;
    *TransformFn = (_cmsTransformFn) FloatCMYKCLUTEval;
    *UserData   = p8;
    *FreeDataFn = _cmsFree;
    *dwFlags &= ~cmsFLAGS_CAN_CHANGE_FORMATTER;
    return TRUE;

Error:
      
    if (OptimizedLUT != NULL) cmsPipelineFree(OptimizedLUT);

    return FALSE;    
}


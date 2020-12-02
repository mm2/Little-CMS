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

#include <stdlib.h>
#include <memory.h>


// Some pixel representations
typedef struct { cmsUInt8Number  r, g, b;    }  Scanline_rgb8bits;
typedef struct { cmsUInt8Number  r, g, b, a; }  Scanline_rgba8bits;
typedef struct { cmsUInt8Number  c, m, y, k; }  Scanline_cmyk8bits;
typedef struct { cmsUInt16Number r, g, b;    }  Scanline_rgb16bits;
typedef struct { cmsUInt16Number r, g, b, a; }  Scanline_rgba16bits;
typedef struct { cmsUInt16Number c, m, y, k; }  Scanline_cmyk16bits;
typedef struct { cmsUInt16Number r, g, b;    }  Scanline_rgb15bits;
typedef struct { cmsUInt16Number r, g, b, a; }  Scanline_rgba15bits;
typedef struct { cmsUInt16Number r, g, b, a; }  Scanline_cmyk15bits;
typedef struct { cmsFloat32Number r, g, b;    }  Scanline_rgbFloat;
typedef struct { cmsFloat32Number r, g, b, a; }  Scanline_rgbaFloat;
typedef struct { cmsFloat32Number c, m, y, k; }  Scanline_cmykFloat;
typedef struct { cmsFloat32Number L, a, b; }     Scanline_LabFloat;

// 15 bit mode. <=> 8 bits mode
#define FROM_8_TO_15(x8) (cmsUInt16Number) ((((cmsUInt64Number)x8 << 15)) / 0xFF)
#define FROM_15_TO_8(x15) (cmsUInt8Number) (((cmsUInt64Number) x15 * 0xFF + 0x4000) >> 15)


// Floating point acuracy for tests 
#define EPSILON_FLOAT_TESTS 0.005


// The callback function used by cmsSetLogErrorHandler()
static
void FatalErrorQuit(cmsContext ContextID, cmsUInt32Number ErrorCode, const char *Text)
{
       UNUSED_PARAMETER(ContextID);
       UNUSED_PARAMETER(ErrorCode);

       printf("** Fatal error: %s\n", Text);
       exit(1);
}

// Rise an error and exit
static
void Fail(const char* frm, ...)
{
       char ReasonToFailBuffer[1024];
       va_list args;

       va_start(args, frm);
       vsprintf(ReasonToFailBuffer, frm, args);
       FatalErrorQuit(0, 0, ReasonToFailBuffer);

      // unreacheable va_end(args);
}


// Creates a fake profile that only has a curve. Used in several places
static
cmsHPROFILE CreateCurves(void)
{
       cmsToneCurve* Gamma = cmsBuildGamma(0, 1.1);
       cmsToneCurve* Transfer[3];
       cmsHPROFILE h;

       Transfer[0] = Transfer[1] = Transfer[2] = Gamma;
       h = cmsCreateLinearizationDeviceLink(cmsSigRgbData, Transfer);

       cmsFreeToneCurve(Gamma);

       return h;
}


// Check for a single 15 bit Photoshop-like formatter
static
void CheckSingleFormatter15(cmsContext id, cmsUInt32Number Type, const char* Text)
{
       cmsUInt16Number Values[cmsMAXCHANNELS];
       cmsUInt8Number Buffer[1024];
       cmsFormatter f, b;
       cmsInt32Number i, j, nChannels, bytes;
       _xform_head info;

       UNUSED_PARAMETER(id);

       memset(&info, 0, sizeof(info));
       info.OutputFormat = info.InputFormat = Type;

       // Get functions to go forth and back
       f = Formatter_15Bit_Factory(Type, cmsFormatterInput, CMS_PACK_FLAGS_16BITS);
       b = Formatter_15Bit_Factory(Type, cmsFormatterOutput, CMS_PACK_FLAGS_16BITS);

       if (f.Fmt16 == NULL || b.Fmt16 == NULL) {

              Fail("no formatter for %s", Text);
              return;
       }

       nChannels = T_CHANNELS(Type);
       bytes = T_BYTES(Type);

       for (j = 0; j < 5; j++) {

              for (i = 0; i < nChannels; i++) {

                     Values[i] = (cmsUInt16Number)(i + j) << 1;
              }

              b.Fmt16((struct _cmstransform_struct*) &info, Values, Buffer, 1);
              memset(Values, 0, sizeof(Values));
              f.Fmt16((struct _cmstransform_struct*) &info, Values, Buffer, 1);

              for (i = 0; i < nChannels; i++) {

                     if (Values[i] != ((i + j) << 1)) {

                            Fail("%s failed", Text);
                            return;
                     }
              }
       }
}

#define C(a) CheckSingleFormatter15(0, a, #a)

// Check for all 15 bits formatters
static
void CheckFormatters15(void)
{
       C(TYPE_GRAY_15);
       C(TYPE_GRAY_15_REV);
       C(TYPE_GRAY_15_SE);
       C(TYPE_GRAYA_15);
       C(TYPE_GRAYA_15_SE);
       C(TYPE_GRAYA_15_PLANAR);
       C(TYPE_RGB_15);
       C(TYPE_RGB_15_PLANAR);
       C(TYPE_RGB_15_SE);
       C(TYPE_BGR_15);
       C(TYPE_BGR_15_PLANAR);
       C(TYPE_BGR_15_SE);
       C(TYPE_RGBA_15);
       C(TYPE_RGBA_15_PLANAR);
       C(TYPE_RGBA_15_SE);
       C(TYPE_ARGB_15);
       C(TYPE_ABGR_15);
       C(TYPE_ABGR_15_PLANAR);
       C(TYPE_ABGR_15_SE);
       C(TYPE_BGRA_15);
       C(TYPE_BGRA_15_SE);
       C(TYPE_YMC_15);
       C(TYPE_CMY_15);
       C(TYPE_CMY_15_PLANAR);
       C(TYPE_CMY_15_SE);
       C(TYPE_CMYK_15);
       C(TYPE_CMYK_15_REV);
       C(TYPE_CMYK_15_PLANAR);
       C(TYPE_CMYK_15_SE);
       C(TYPE_KYMC_15);
       C(TYPE_KYMC_15_SE);
       C(TYPE_KCMY_15);
       C(TYPE_KCMY_15_REV);
       C(TYPE_KCMY_15_SE);
}
#undef C


static
cmsInt32Number checkSingleComputeIncrements(cmsUInt32Number Format, cmsUInt32Number planeStride, cmsUInt32Number ExpectedChannels, cmsUInt32Number ExpectedAlpha, ...)
{
       cmsUInt32Number nChannels, nAlpha, nTotal, i, rc = 0 ;
       cmsUInt32Number ComponentStartingOrder[cmsMAXCHANNELS], ComponentPointerIncrements[cmsMAXCHANNELS];
       va_list args;

  
       va_start(args, ExpectedAlpha);

       _cmsComputeComponentIncrements(Format, planeStride, &nChannels, &nAlpha, ComponentStartingOrder, ComponentPointerIncrements);

       if (nChannels != ExpectedChannels)
              return 0;

       if (nAlpha != ExpectedAlpha)
              return 0;

       nTotal = nAlpha + nChannels;
     
       for (i = 0; i < nTotal; i++)
       {
              cmsUInt32Number so = va_arg(args, cmsUInt32Number);
              if (so != ComponentStartingOrder[i])
                     goto Error;                     
       }

       for (i = 0; i < nTotal; i++)
       {
              cmsUInt32Number so = va_arg(args, cmsUInt32Number);
              if (so != ComponentPointerIncrements[i])
                     goto Error;
       }

       // Success
       rc = 1;

Error:
       va_end(args);

       return rc;
}

#define CHECK(frm, plane, chans, alpha, ...) if (!checkSingleComputeIncrements(frm, plane, chans, alpha, __VA_ARGS__)) { printf("Format failed!\n"); return 0; }



// Validate the compute increments function
cmsInt32Number CheckComputeIncrements(void)
{      
       CHECK(TYPE_GRAY_8,    0, 1, 0, /**/ 0,    /**/ 1);
       CHECK(TYPE_GRAYA_8,   0, 1, 1, /**/ 0, 1, /**/ 2, 2);
       CHECK(TYPE_AGRAY_8,   0, 1, 1, /**/ 1, 0, /**/ 2, 2);
       CHECK(TYPE_GRAY_16,   0, 1, 0, /**/ 0,    /**/ 2);
       CHECK(TYPE_GRAYA_16,  0, 1, 1, /**/ 0, 2, /**/ 4, 4);
       CHECK(TYPE_AGRAY_16,  0, 1, 1, /**/ 2, 0, /**/ 4, 4);

       CHECK(TYPE_GRAY_FLT,  0, 1, 0, /**/ 0,    /**/ 4);
       CHECK(TYPE_GRAYA_FLT, 0, 1, 1, /**/ 0, 4, /**/ 8, 8);
       CHECK(TYPE_AGRAY_FLT, 0, 1, 1, /**/ 4, 0, /**/ 8, 8);

       CHECK(TYPE_GRAY_DBL,  0, 1, 0, /**/ 0,      /**/ 8);
       CHECK(TYPE_AGRAY_DBL, 0, 1, 1, /**/ 8, 0,   /**/ 16, 16);

       CHECK(TYPE_RGB_8,    0, 3, 0, /**/ 0, 1, 2,     /**/ 3, 3, 3);
       CHECK(TYPE_RGBA_8,   0, 3, 1, /**/ 0, 1, 2, 3,  /**/ 4, 4, 4, 4);
       CHECK(TYPE_ARGB_8,   0, 3, 1, /**/ 1, 2, 3, 0,  /**/ 4, 4, 4, 4);

       CHECK(TYPE_RGB_16,  0, 3, 0, /**/ 0, 2, 4,     /**/ 6, 6, 6);
       CHECK(TYPE_RGBA_16, 0, 3, 1, /**/ 0, 2, 4, 6,  /**/ 8, 8, 8, 8);
       CHECK(TYPE_ARGB_16, 0, 3, 1, /**/ 2, 4, 6, 0,  /**/ 8, 8, 8, 8);

       CHECK(TYPE_RGB_FLT,  0, 3, 0, /**/ 0, 4, 8,     /**/ 12, 12, 12);
       CHECK(TYPE_RGBA_FLT, 0, 3, 1, /**/ 0, 4, 8, 12,  /**/ 16, 16, 16, 16);
       CHECK(TYPE_ARGB_FLT, 0, 3, 1, /**/ 4, 8, 12, 0,  /**/ 16, 16, 16, 16);

       CHECK(TYPE_BGR_8,  0, 3, 0, /**/ 2, 1, 0,     /**/ 3, 3, 3);
       CHECK(TYPE_BGRA_8, 0, 3, 1, /**/ 2, 1, 0, 3,  /**/ 4, 4, 4, 4);
       CHECK(TYPE_ABGR_8, 0, 3, 1, /**/ 3, 2, 1, 0,  /**/ 4, 4, 4, 4);

       CHECK(TYPE_BGR_16,  0, 3, 0, /**/ 4, 2, 0,     /**/ 6, 6, 6);
       CHECK(TYPE_BGRA_16, 0, 3, 1, /**/ 4, 2, 0, 6,  /**/ 8, 8, 8, 8);
       CHECK(TYPE_ABGR_16, 0, 3, 1, /**/ 6, 4, 2, 0,  /**/ 8, 8, 8, 8);

       CHECK(TYPE_BGR_FLT, 0, 3, 0,  /**/ 8, 4, 0,     /**/  12, 12, 12);
       CHECK(TYPE_BGRA_FLT, 0, 3, 1, /**/ 8, 4, 0, 12,  /**/ 16, 16, 16, 16);
       CHECK(TYPE_ABGR_FLT, 0, 3, 1, /**/ 12, 8, 4, 0,  /**/ 16, 16, 16, 16);


       CHECK(TYPE_CMYK_8,  0, 4, 0, /**/ 0, 1, 2, 3,     /**/ 4, 4, 4, 4);
       CHECK(TYPE_CMYKA_8, 0, 4, 1, /**/ 0, 1, 2, 3, 4,  /**/ 5, 5, 5, 5, 5);
       CHECK(TYPE_ACMYK_8, 0, 4, 1, /**/ 1, 2, 3, 4, 0,  /**/ 5, 5, 5, 5, 5);

       CHECK(TYPE_KYMC_8,  0, 4, 0, /**/ 3, 2, 1, 0,     /**/ 4, 4, 4, 4);
       CHECK(TYPE_KYMCA_8, 0, 4, 1, /**/ 3, 2, 1, 0, 4,  /**/ 5, 5, 5, 5, 5);
       CHECK(TYPE_AKYMC_8, 0, 4, 1, /**/ 4, 3, 2, 1, 0,  /**/ 5, 5, 5, 5, 5);

       CHECK(TYPE_KCMY_8,  0, 4, 0, /**/ 1, 2, 3, 0,      /**/ 4, 4, 4, 4);
       
       CHECK(TYPE_CMYK_16, 0, 4, 0, /**/ 0, 2, 4, 6,      /**/ 8, 8, 8, 8);
       CHECK(TYPE_CMYKA_16, 0, 4, 1, /**/ 0, 2, 4, 6, 8,  /**/ 10, 10, 10, 10, 10);
       CHECK(TYPE_ACMYK_16, 0, 4, 1, /**/ 2, 4, 6, 8, 0,  /**/ 10, 10, 10, 10, 10);

       CHECK(TYPE_KYMC_16, 0, 4, 0,  /**/ 6, 4, 2, 0,     /**/ 8, 8, 8, 8);
       CHECK(TYPE_KYMCA_16, 0, 4, 1, /**/ 6, 4, 2, 0, 8,  /**/ 10, 10, 10, 10, 10);
       CHECK(TYPE_AKYMC_16, 0, 4, 1, /**/ 8, 6, 4, 2, 0,  /**/ 10, 10, 10, 10, 10);

       CHECK(TYPE_KCMY_16, 0, 4, 0, /**/ 2, 4, 6, 0,      /**/ 8, 8, 8, 8);

       // Planar

       CHECK(TYPE_GRAYA_8_PLANAR, 100, 1, 1, /**/ 0, 100,  /**/ 1, 1);
       CHECK(TYPE_AGRAY_8_PLANAR, 100, 1, 1, /**/ 100, 0,  /**/ 1, 1);

       CHECK(TYPE_GRAYA_16_PLANAR, 100, 1, 1, /**/ 0, 100,   /**/ 2, 2);
       CHECK(TYPE_AGRAY_16_PLANAR, 100, 1, 1, /**/ 100, 0,   /**/ 2, 2);

       CHECK(TYPE_GRAYA_FLT_PLANAR, 100, 1, 1, /**/ 0, 100,   /**/ 4, 4);
       CHECK(TYPE_AGRAY_FLT_PLANAR, 100, 1, 1, /**/ 100, 0,   /**/ 4, 4);

       CHECK(TYPE_GRAYA_DBL_PLANAR, 100, 1, 1, /**/ 0, 100,   /**/ 8, 8);
       CHECK(TYPE_AGRAY_DBL_PLANAR, 100, 1, 1, /**/ 100, 0,   /**/ 8, 8);

       CHECK(TYPE_RGB_8_PLANAR,  100, 3, 0, /**/ 0, 100, 200,      /**/ 1, 1, 1);
       CHECK(TYPE_RGBA_8_PLANAR, 100, 3, 1, /**/ 0, 100, 200, 300, /**/ 1, 1, 1, 1);
       CHECK(TYPE_ARGB_8_PLANAR, 100, 3, 1, /**/ 100, 200, 300, 0,  /**/ 1, 1, 1, 1);

       CHECK(TYPE_BGR_8_PLANAR,  100, 3, 0, /**/ 200, 100, 0,       /**/ 1, 1, 1);
       CHECK(TYPE_BGRA_8_PLANAR, 100, 3, 1, /**/ 200, 100, 0, 300,  /**/ 1, 1, 1, 1);
       CHECK(TYPE_ABGR_8_PLANAR, 100, 3, 1, /**/ 300, 200, 100, 0,  /**/ 1, 1, 1, 1);

       CHECK(TYPE_RGB_16_PLANAR, 100, 3, 0, /**/ 0, 100, 200,      /**/ 2, 2, 2);
       CHECK(TYPE_RGBA_16_PLANAR, 100, 3, 1, /**/ 0, 100, 200, 300, /**/ 2, 2, 2, 2);
       CHECK(TYPE_ARGB_16_PLANAR, 100, 3, 1, /**/ 100, 200, 300, 0,  /**/ 2, 2, 2, 2);

       CHECK(TYPE_BGR_16_PLANAR, 100, 3, 0, /**/ 200, 100, 0,       /**/ 2, 2, 2);
       CHECK(TYPE_BGRA_16_PLANAR, 100, 3, 1, /**/ 200, 100, 0, 300,  /**/ 2, 2, 2, 2);
       CHECK(TYPE_ABGR_16_PLANAR, 100, 3, 1, /**/ 300, 200, 100, 0,  /**/ 2, 2, 2, 2);

       return 1;
}



// Check 15 bit mode accuracy
static
cmsBool Valid15(cmsUInt16Number a, cmsUInt8Number b)
{
       return abs(FROM_15_TO_8(a) - b) <= 2;
}

// Check the test macros itselves
static
void Check15bitMacros(void)
{
       int i;

       printf("Checking 15 bit <=> 8 bit macros...");

       for (i = 0; i < 256; i++)
       {
              cmsUInt16Number n = FROM_8_TO_15(i);
              cmsUInt8Number m = FROM_15_TO_8(n);

              if (m != i)
                     Fail("Failed on %d (->%d->%d)", i, n, m);
       }
       printf("ok\n");
}

// Do an in-depth test by checking all RGB cube of 8 bits, going from profilein to profileout. 
// Results should be same except for 2 contone levels allowed for roundoff. Note 15 bits is more 
// precise than 8 bits and this is a source of discrepancies. Cache is disabled
static 
void TryAllValues15(cmsHPROFILE hlcmsProfileIn, cmsHPROFILE hlcmsProfileOut, cmsInt32Number Intent)
{
       Scanline_rgb8bits* buffer8in;
       Scanline_rgb15bits* buffer15in;
       Scanline_rgb8bits* buffer8out;
       Scanline_rgb15bits* buffer15out;
       int r, g, b, j;
       cmsUInt32Number npixels = 256 * 256 * 256;  // All RGB cube in 8 bits

       cmsHTRANSFORM xform15 = cmsCreateTransformTHR(0, hlcmsProfileIn, TYPE_RGB_15, hlcmsProfileOut, TYPE_RGB_15, Intent, cmsFLAGS_NOCACHE);
       cmsHTRANSFORM xform8 = cmsCreateTransformTHR(0, hlcmsProfileIn, TYPE_RGB_8, hlcmsProfileOut, TYPE_RGB_8, Intent, cmsFLAGS_NOCACHE);       // Transforms already created
       
       cmsCloseProfile(hlcmsProfileIn);
       cmsCloseProfile(hlcmsProfileOut);

       if (xform15 == NULL || xform8 == NULL) {

              Fail("NULL transforms on check for 15 bit conversions");
       }

       // Since this is just a test, I will not check memory allocation...
       buffer8in = (Scanline_rgb8bits*)malloc(npixels * sizeof(Scanline_rgb8bits));
       buffer15in = (Scanline_rgb15bits*)malloc(npixels * sizeof(Scanline_rgb15bits));
       buffer8out = (Scanline_rgb8bits*)malloc(npixels * sizeof(Scanline_rgb8bits));
       buffer15out = (Scanline_rgb15bits*)malloc(npixels * sizeof(Scanline_rgb15bits));

       // Fill input values for 8 and 15 bits
       j = 0;
       for (r = 0; r < 256; r++)
              for (g = 0; g < 256; g++)
                     for (b = 0; b < 256; b++) {

              buffer8in[j].r = (cmsUInt8Number)r;
              buffer8in[j].g = (cmsUInt8Number)g;
              buffer8in[j].b = (cmsUInt8Number)b;

              buffer15in[j].r = FROM_8_TO_15(r);
              buffer15in[j].g = FROM_8_TO_15(g);
              buffer15in[j].b = FROM_8_TO_15(b);

              j++;
       }
       
       cmsDoTransform(xform15, buffer15in, buffer15out, npixels);
       cmsDoTransform(xform8,  buffer8in, buffer8out,  npixels);

       j = 0;
       for (r = 0; r < 256; r++)
              for (g = 0; g < 256; g++)
                     for (b = 0; b < 256; b++) {

              // Check the results
              if (!Valid15(buffer15out[j].r, buffer8out[j].r) ||
                  !Valid15(buffer15out[j].g, buffer8out[j].g) ||
                  !Valid15(buffer15out[j].b, buffer8out[j].b))
                  Fail("Conversion failed at (%d %d %d) != (%d %d %d)", buffer8out[j].r, buffer8out[j].g, buffer8out[j].b,
                  FROM_15_TO_8(buffer15out[j].r), FROM_15_TO_8(buffer15out[j].g), FROM_15_TO_8(buffer15out[j].b));

              j++;
       }

       free(buffer8in); free(buffer15in);
       free(buffer8out); free(buffer15out);
       cmsDeleteTransform(xform15);
       cmsDeleteTransform(xform8);   
}

// Convert some known values
static
void Check15bitsConversions(void)
{
       Check15bitMacros();

       printf("Checking accuracy of 15 bits on CLUT...");
       TryAllValues15(cmsOpenProfileFromFile("test5.icc", "r"), cmsOpenProfileFromFile("test3.icc", "r"), INTENT_PERCEPTUAL);
       printf("Ok\n");

       printf("Checking accuracy of 15 bits on same profile ...");
       TryAllValues15(cmsOpenProfileFromFile("test0.icc", "r"), cmsOpenProfileFromFile("test0.icc", "r"), INTENT_PERCEPTUAL);
       printf("Ok\n");

       printf("Checking accuracy of 15 bits on Matrix...");
       TryAllValues15(cmsOpenProfileFromFile("test5.icc", "r"), cmsOpenProfileFromFile("test0.icc", "r"), INTENT_PERCEPTUAL);
       printf("Ok\n");

       printf("All 15 bits tests passed OK\n\n");
}

// Next test checks results of optimized 16 bits versus raw 16 bits. 
static
void TryAllValues16bits(cmsHPROFILE hlcmsProfileIn, cmsHPROFILE hlcmsProfileOut, cmsInt32Number Intent)
{
    cmsContext Raw = cmsCreateContext(NULL, NULL);
    cmsContext Plugin = cmsCreateContext(cmsFastFloatExtensions(), NULL);

    Scanline_rgba16bits* bufferIn;
    Scanline_rgba16bits* bufferRawOut;
    Scanline_rgba16bits* bufferPluginOut;
    int r, g, b;

    int j;
    cmsUInt32Number npixels = 256 * 256 * 256;

    cmsHTRANSFORM xformRaw = cmsCreateTransformTHR(Raw, hlcmsProfileIn, TYPE_RGBA_16, hlcmsProfileOut, TYPE_RGBA_16, Intent, cmsFLAGS_NOCACHE| cmsFLAGS_COPY_ALPHA);
    cmsHTRANSFORM xformPlugin = cmsCreateTransformTHR(Plugin, hlcmsProfileIn, TYPE_RGBA_16, hlcmsProfileOut, TYPE_RGBA_16, Intent, cmsFLAGS_NOCACHE| cmsFLAGS_COPY_ALPHA);

    cmsCloseProfile(hlcmsProfileIn);
    cmsCloseProfile(hlcmsProfileOut);

    if (xformRaw == NULL || xformPlugin == NULL) {

        Fail("NULL transforms on check float conversions");
    }

    // Again, no checking on mem alloc because this is just a test
    bufferIn = (Scanline_rgba16bits*)malloc(npixels * sizeof(Scanline_rgba16bits));
    bufferRawOut = (Scanline_rgba16bits*)malloc(npixels * sizeof(Scanline_rgba16bits));
    bufferPluginOut = (Scanline_rgba16bits*)malloc(npixels * sizeof(Scanline_rgba16bits));

    // Same input to both transforms
    j = 0;
    for (r = 0; r < 256; r++)
        for (g = 0; g < 256; g++)
            for (b = 0; b < 256; b++) {

                bufferIn[j].r = FROM_8_TO_16(0xf8);
                bufferIn[j].g = FROM_8_TO_16(0xf8);
                bufferIn[j].b = FROM_8_TO_16(0xf8);
                bufferIn[j].a = 0xffff;

                j++;
            }

    // Different transforms, different output buffers
    cmsDoTransform(xformRaw, bufferIn, bufferRawOut, npixels);
    cmsDoTransform(xformPlugin, bufferIn, bufferPluginOut, npixels);

    // Lets compare results
    j = 0;
    for (r = 0; r < 256; r++)
        for (g = 0; g < 256; g++)
            for (b = 0; b < 256; b++) {

                if (bufferRawOut[j].r != bufferPluginOut[j].r ||
                    bufferRawOut[j].g != bufferPluginOut[j].g ||
                    bufferRawOut[j].b != bufferPluginOut[j].b ||
                    bufferRawOut[j].a != bufferPluginOut[j].a)
                    Fail(
                    "Conversion failed at [%x %x %x %x] (%x %x %x %x) != (%x %x %x %x)", 
                        bufferIn[j].r, bufferIn[j].g, bufferIn[j].b, bufferIn[j].a,
                        bufferRawOut[j].r, bufferRawOut[j].g, bufferRawOut[j].b, bufferRawOut[j].a,
                        bufferPluginOut[j].r, bufferPluginOut[j].g, bufferPluginOut[j].b, bufferPluginOut[j].a);

                j++;
            }

    free(bufferIn); free(bufferRawOut);
    free(bufferPluginOut);

    cmsDeleteTransform(xformRaw);
    cmsDeleteTransform(xformPlugin);

    cmsDeleteContext(Plugin);
    cmsDeleteContext(Raw);
}

static
void CheckAccuracy16Bits(void)
{
    // CLUT should be as 16 bits or better
    printf("Checking accuracy of 16 bits CLUT...");
    TryAllValues16bits(cmsOpenProfileFromFile("test5.icc", "r"), cmsOpenProfileFromFile("test3.icc", "r"), INTENT_PERCEPTUAL);
    printf("All 16 bits tests passed OK\n\n");
}

// --------------------------------------------------------------------------------------------------
// A C C U R A C Y   C H E C K S
// --------------------------------------------------------------------------------------------------


// Check result accuracy
static
cmsBool ValidFloat(cmsFloat32Number a, cmsFloat32Number b)
{
       return fabsf(a-b) < EPSILON_FLOAT_TESTS;
}

// Do an in-depth test by checking all RGB cube of 8 bits, going from profilein to profileout. 
// Values with and without optimization are checked (different contexts, one with the plugin and another without)
// Results should be same except for EPSILON_FLOAT_TESTS allowed for accuracy/speed tradeoff. Cache is disabled
static
void TryAllValuesFloat(cmsHPROFILE hlcmsProfileIn, cmsHPROFILE hlcmsProfileOut, cmsInt32Number Intent)
{
       cmsContext Raw     = cmsCreateContext(NULL, NULL);
       cmsContext Plugin  = cmsCreateContext(cmsFastFloatExtensions(), NULL);

       Scanline_rgbFloat* bufferIn;
       Scanline_rgbFloat* bufferRawOut;
       Scanline_rgbFloat* bufferPluginOut;
       int r, g, b;
       
       int j;
       cmsUInt32Number npixels = 256 * 256 * 256;

       cmsHTRANSFORM xformRaw = cmsCreateTransformTHR(Raw, hlcmsProfileIn, TYPE_RGB_FLT, hlcmsProfileOut, TYPE_RGB_FLT, Intent, cmsFLAGS_NOCACHE);
       cmsHTRANSFORM xformPlugin = cmsCreateTransformTHR(Plugin, hlcmsProfileIn, TYPE_RGB_FLT, hlcmsProfileOut, TYPE_RGB_FLT, Intent, cmsFLAGS_NOCACHE);

       cmsCloseProfile(hlcmsProfileIn);
       cmsCloseProfile(hlcmsProfileOut);

       if (xformRaw == NULL || xformPlugin == NULL) {

              Fail("NULL transforms on check float conversions");
       }

       // Again, no checking on mem alloc because this is just a test
       bufferIn = (Scanline_rgbFloat*)malloc(npixels * sizeof(Scanline_rgbFloat));
       bufferRawOut = (Scanline_rgbFloat*)malloc(npixels * sizeof(Scanline_rgbFloat));
       bufferPluginOut = (Scanline_rgbFloat*)malloc(npixels * sizeof(Scanline_rgbFloat));

       // Same input to both transforms
       j = 0;
       for (r = 0; r < 256; r++)
              for (g = 0; g < 256; g++)
                     for (b = 0; b < 256; b++) {

              bufferIn[j].r = (cmsFloat32Number)r / 255.0f;
              bufferIn[j].g = (cmsFloat32Number)g / 255.0f;
              bufferIn[j].b = (cmsFloat32Number)b / 255.0f;
              
              j++;
              }
       
       // Different transforms, different output buffers
       cmsDoTransform(xformRaw,    bufferIn, bufferRawOut, npixels);
       cmsDoTransform(xformPlugin, bufferIn, bufferPluginOut, npixels);

       // Lets compare results
       j = 0;
       for (r = 0; r < 256; r++)
              for (g = 0; g < 256; g++)
                     for (b = 0; b < 256; b++) {

              if (!ValidFloat(bufferRawOut[j].r, bufferPluginOut[j].r) ||
                     !ValidFloat(bufferRawOut[j].g, bufferPluginOut[j].g) ||
                     !ValidFloat(bufferRawOut[j].b, bufferPluginOut[j].b))
                     Fail("Conversion failed at (%f %f %f) != (%f %f %f)", bufferRawOut[j].r, bufferRawOut[j].g, bufferRawOut[j].b,
                     bufferPluginOut[j].r, bufferPluginOut[j].g, bufferPluginOut[j].b);

              j++;
              }

       free(bufferIn); free(bufferRawOut);
       free(bufferPluginOut);

       cmsDeleteTransform(xformRaw);
       cmsDeleteTransform(xformPlugin);

       cmsDeleteContext(Plugin);
       cmsDeleteContext(Raw);
}

static
void TryAllValuesFloatAlpha(cmsHPROFILE hlcmsProfileIn, cmsHPROFILE hlcmsProfileOut, cmsInt32Number Intent, cmsBool copyAlpha)
{
       cmsContext Raw     = cmsCreateContext(NULL, NULL);
       cmsContext Plugin  = cmsCreateContext(cmsFastFloatExtensions(), NULL);

       Scanline_rgbaFloat* bufferIn;
       Scanline_rgbaFloat* bufferRawOut;
       Scanline_rgbaFloat* bufferPluginOut;
       int r, g, b;
       
       int j;
       cmsUInt32Number npixels = 256 * 256 * 256;

       cmsUInt32Number flags = cmsFLAGS_NOCACHE | ( copyAlpha? cmsFLAGS_COPY_ALPHA : 0);

       cmsHTRANSFORM xformRaw = cmsCreateTransformTHR(Raw, hlcmsProfileIn, TYPE_RGBA_FLT, hlcmsProfileOut, TYPE_RGBA_FLT, Intent, flags);
       cmsHTRANSFORM xformPlugin = cmsCreateTransformTHR(Plugin, hlcmsProfileIn, TYPE_RGBA_FLT, hlcmsProfileOut, TYPE_RGBA_FLT, Intent, flags);

       cmsCloseProfile(hlcmsProfileIn);
       cmsCloseProfile(hlcmsProfileOut);

       if (xformRaw == NULL || xformPlugin == NULL) {

              Fail("NULL transforms on check float conversions");
       }

       // Again, no checking on mem alloc because this is just a test
       bufferIn = (Scanline_rgbaFloat*)malloc(npixels * sizeof(Scanline_rgbaFloat));
       bufferRawOut = (Scanline_rgbaFloat*)malloc(npixels * sizeof(Scanline_rgbaFloat));
       bufferPluginOut = (Scanline_rgbaFloat*)malloc(npixels * sizeof(Scanline_rgbaFloat));

       memset(bufferRawOut, 0, npixels * sizeof(Scanline_rgbaFloat));
       memset(bufferPluginOut, 0, npixels * sizeof(Scanline_rgbaFloat));

       // Same input to both transforms
       j = 0;
       for (r = 0; r < 256; r++)
              for (g = 0; g < 256; g++)
                     for (b = 0; b < 256; b++) {

              bufferIn[j].r = (cmsFloat32Number)r / 255.0f;
              bufferIn[j].g = (cmsFloat32Number)g / 255.0f;
              bufferIn[j].b = (cmsFloat32Number)b / 255.0f;
              bufferIn[j].a = (cmsFloat32Number) 1.0f;
              
              j++;
              }
       
       // Different transforms, different output buffers
       cmsDoTransform(xformRaw,    bufferIn, bufferRawOut, npixels);
       cmsDoTransform(xformPlugin, bufferIn, bufferPluginOut, npixels);

       // Lets compare results
       j = 0;
       for (r = 0; r < 256; r++)
              for (g = 0; g < 256; g++)
                     for (b = 0; b < 256; b++) {

              if (!ValidFloat(bufferRawOut[j].r, bufferPluginOut[j].r) ||
                  !ValidFloat(bufferRawOut[j].g, bufferPluginOut[j].g) ||
                  !ValidFloat(bufferRawOut[j].b, bufferPluginOut[j].b) ||
                  !ValidFloat(bufferRawOut[j].a, bufferPluginOut[j].a))
                    Fail("Conversion failed at (%f %f %f %f) != (%f %f %f %f)", bufferRawOut[j].r, bufferRawOut[j].g, bufferRawOut[j].b, bufferRawOut[j].a,
                     bufferPluginOut[j].r, bufferPluginOut[j].g, bufferPluginOut[j].b, bufferPluginOut[j].a);

              j++;
              }

       free(bufferIn); free(bufferRawOut);
       free(bufferPluginOut);

       cmsDeleteTransform(xformRaw);
       cmsDeleteTransform(xformPlugin);

       cmsDeleteContext(Plugin);
       cmsDeleteContext(Raw);
}



// Next test checks results of optimized floating point versus 16 bits. That is, converting the float to 16 bits, operating 
// in 16 bits and back to float. Results again should be in range of epsilon
static
cmsBool Valid16Float(cmsUInt16Number a, cmsFloat32Number b)
{
       return fabs(((cmsFloat32Number)a / (cmsFloat32Number) 0xFFFF) - b) < EPSILON_FLOAT_TESTS;
}

// Do an in-depth test by checking all RGB cube of 8 bits, going from profilein to profileout. 16 bits temporary is used as reference
static
void TryAllValuesFloatVs16(cmsHPROFILE hlcmsProfileIn, cmsHPROFILE hlcmsProfileOut, cmsInt32Number Intent)
{          
       Scanline_rgbFloat* bufferIn;
       Scanline_rgb16bits* bufferIn16;

       Scanline_rgbFloat* bufferFloatOut;
       Scanline_rgb16bits* buffer16Out;
       int r, g, b;

       int j;
       cmsUInt32Number npixels = 256 * 256 * 256;

       cmsHTRANSFORM xformRaw = cmsCreateTransform(hlcmsProfileIn, TYPE_RGB_16, hlcmsProfileOut, TYPE_RGB_16, Intent, cmsFLAGS_NOCACHE);
       cmsHTRANSFORM xformPlugin = cmsCreateTransform(hlcmsProfileIn, TYPE_RGB_FLT, hlcmsProfileOut, TYPE_RGB_FLT, Intent, cmsFLAGS_NOCACHE);

       cmsCloseProfile(hlcmsProfileIn);
       cmsCloseProfile(hlcmsProfileOut);

       if (xformRaw == NULL || xformPlugin == NULL) {

              Fail("NULL transforms on check float vs 16 conversions");
       }

       // Again, no checking on mem alloc because this is just a test
       bufferIn = (Scanline_rgbFloat*)malloc(npixels * sizeof(Scanline_rgbFloat));
       bufferIn16 = (Scanline_rgb16bits*)malloc(npixels * sizeof(Scanline_rgb16bits));
       bufferFloatOut = (Scanline_rgbFloat*)malloc(npixels * sizeof(Scanline_rgbFloat));
       buffer16Out = (Scanline_rgb16bits*)malloc(npixels * sizeof(Scanline_rgb16bits));


       // Fill two equivalent input buffers
       j = 0;
       for (r = 0; r < 256; r++)
              for (g = 0; g < 256; g++)
                     for (b = 0; b < 256; b++) {

              bufferIn[j].r = (cmsFloat32Number)r / 255.0f;
              bufferIn[j].g = (cmsFloat32Number)g / 255.0f;
              bufferIn[j].b = (cmsFloat32Number)b / 255.0f;

              bufferIn16[j].r = FROM_8_TO_16(r);
              bufferIn16[j].g = FROM_8_TO_16(g);
              bufferIn16[j].b = FROM_8_TO_16(b);

              j++;
       }

       // Convert 
       cmsDoTransform(xformRaw, bufferIn16, buffer16Out, npixels);
       cmsDoTransform(xformPlugin, bufferIn, bufferFloatOut, npixels);


       j = 0;
       for (r = 0; r < 256; r++)
              for (g = 0; g < 256; g++)
                     for (b = 0; b < 256; b++) {

              // Check for same values
              if (!Valid16Float(buffer16Out[j].r, bufferFloatOut[j].r) ||
                     !Valid16Float(buffer16Out[j].g, bufferFloatOut[j].g) ||
                     !Valid16Float(buffer16Out[j].b, bufferFloatOut[j].b))
                     Fail("Conversion failed at (%f %f %f) != (%f %f %f)", buffer16Out[j].r / 65535.0, buffer16Out[j].g / 65535.0, buffer16Out[j].b / 65535.0,
                     bufferFloatOut[j].r, bufferFloatOut[j].g, bufferFloatOut[j].b);

              j++;
       }

       free(bufferIn16); free(buffer16Out);
       free(bufferIn); free(bufferFloatOut);
       cmsDeleteTransform(xformRaw);
       cmsDeleteTransform(xformPlugin);
}


// Check change format feature
static
void CheckChangeFormat(void)
{
    cmsHPROFILE hsRGB, hLab;
    cmsHTRANSFORM xform;
    cmsUInt8Number rgb8[3]  = { 10, 120, 40 };
    cmsUInt16Number rgb16[3] = { 10* 257, 120*257, 40*257 };
    cmsUInt16Number lab16_1[3], lab16_2[3];

    printf("Checking change format feature...");

    hsRGB = cmsCreate_sRGBProfile();
    hLab = cmsCreateLab4Profile(NULL);

    xform = cmsCreateTransform(hsRGB, TYPE_RGB_16, hLab, TYPE_Lab_16, INTENT_PERCEPTUAL, 0);

    cmsCloseProfile(hsRGB);
    cmsCloseProfile(hLab);

    cmsDoTransform(xform, rgb16, lab16_1, 1);

    cmsChangeBuffersFormat(xform, TYPE_RGB_8, TYPE_Lab_16);

    cmsDoTransform(xform, rgb8, lab16_2, 1);
    cmsDeleteTransform(xform);

    if (memcmp(lab16_1, lab16_2, sizeof(lab16_1)) != 0)
        Fail("Change format failed!");

    printf("Ok\n");

}

static
cmsBool ValidInt(cmsUInt16Number a, cmsUInt16Number b)
{
    return abs(a - b) <= 32;
}

static
void CheckLab2Roundtrip(void)
{
    cmsHPROFILE hsRGB, hLab;
    cmsHTRANSFORM xform, xform2;
    cmsInt8Number* lab;
    cmsInt32Number Mb, j;
    cmsInt32Number r, g, b;
    Scanline_rgb8bits* In;
    Scanline_rgb8bits* Out;

    printf("Checking lab2 roundtrip...");

    hsRGB = cmsCreate_sRGBProfile();
    hLab = cmsCreateLab2Profile(NULL);


    xform = cmsCreateTransform(hsRGB, TYPE_RGB_8, hLab, TYPE_Lab_8, INTENT_RELATIVE_COLORIMETRIC, cmsFLAGS_NOOPTIMIZE|cmsFLAGS_BLACKPOINTCOMPENSATION);
    xform2 = cmsCreateTransform(hLab, TYPE_Lab_8, hsRGB, TYPE_RGB_8, INTENT_RELATIVE_COLORIMETRIC, cmsFLAGS_NOOPTIMIZE | cmsFLAGS_BLACKPOINTCOMPENSATION);

    cmsCloseProfile(hsRGB);
    cmsCloseProfile(hLab);


    Mb = 256 * 256 * 256 * sizeof(Scanline_rgb8bits);
    In = (Scanline_rgb8bits*)malloc(Mb);
    Out = (Scanline_rgb8bits*)malloc(Mb);
    lab = (cmsInt8Number*)malloc(256 * 256 * 256 * 3 * sizeof(cmsInt8Number));

    j = 0;
    for (r = 0; r < 256; r++)
        for (g = 0; g < 256; g++)
            for (b = 0; b < 256; b++)
            {

                In[j].r = (cmsUInt8Number)r;
                In[j].g = (cmsUInt8Number)g;
                In[j].b = (cmsUInt8Number)b;
                j++;
            }


    cmsDoTransform(xform, In, lab, 256 * 256 * 256);
    cmsDoTransform(xform2, lab, Out, 256 * 256 * 256);

    cmsDeleteTransform(xform);
    cmsDeleteTransform(xform2);


    j = 0;
    for (r = 0; r < 256; r++)
        for (g = 0; g < 256; g++)
            for (b = 0; b < 256; b++) {

                // Check for same values
                if (!ValidInt(In[j].r, Out[j].r) ||
                    !ValidInt(In[j].g, Out[j].g) ||
                    !ValidInt(In[j].b, Out[j].b))
                    Fail("Conversion failed at (%d %d %d) != (%d %d %d)", In[j].r, In[j].g, In[j].b,
                        Out[j].r, Out[j].g, Out[j].b);

                j++;
            }


    free(In);
    free(Out);
    free(lab);
    printf("Ok\n");

}

// Convert some known values
static
void CheckConversionFloat(void)
{
       printf("Crash test.");
       TryAllValuesFloatAlpha(cmsOpenProfileFromFile("test5.icc", "r"), cmsOpenProfileFromFile("test0.icc", "r"), INTENT_PERCEPTUAL, FALSE);
       printf("..");
       TryAllValuesFloatAlpha(cmsOpenProfileFromFile("test5.icc", "r"), cmsOpenProfileFromFile("test0.icc", "r"), INTENT_PERCEPTUAL, TRUE);
       printf("Ok\n");


       printf("Crash (II) test.");
       TryAllValuesFloatAlpha(cmsOpenProfileFromFile("test0.icc", "r"), cmsOpenProfileFromFile("test0.icc", "r"), INTENT_PERCEPTUAL, FALSE);
       printf("..");
       TryAllValuesFloatAlpha(cmsOpenProfileFromFile("test0.icc", "r"), cmsOpenProfileFromFile("test0.icc", "r"), INTENT_PERCEPTUAL, TRUE);
       printf("Ok\n");

       // Matrix-shaper should be accurate 
       printf("Checking accuracy on Matrix-shaper...");
       TryAllValuesFloat(cmsOpenProfileFromFile("test5.icc", "r"), cmsOpenProfileFromFile("test0.icc", "r"), INTENT_PERCEPTUAL);
       printf("Ok\n");

       // CLUT should be as 16 bits or better
       printf("Checking accuracy of CLUT...");
       TryAllValuesFloatVs16(cmsOpenProfileFromFile("test5.icc", "r"), cmsOpenProfileFromFile("test3.icc", "r"), INTENT_PERCEPTUAL);
       printf("Ok\n");

       // Same profile should give same values (we test both methods)
       printf("Checking accuracy on same profile ...");
       TryAllValuesFloatVs16(cmsOpenProfileFromFile("test0.icc", "r"), cmsOpenProfileFromFile("test0.icc", "r"), INTENT_PERCEPTUAL);
       TryAllValuesFloat(cmsOpenProfileFromFile("test0.icc", "r"), cmsOpenProfileFromFile("test0.icc", "r"), INTENT_PERCEPTUAL);
       printf("Ok\n");
}


static
cmsBool ValidFloat2(cmsFloat32Number a, cmsFloat32Number b)
{
    return fabsf(a - b) < 0.007;
}


static
cmsFloat32Number distance(cmsFloat32Number rgb1[], cmsFloat32Number rgb2[])
{
    cmsFloat32Number dr = rgb2[0] - rgb1[0];
    cmsFloat32Number dg = rgb2[1] - rgb1[1];
    cmsFloat32Number db = rgb2[2] - rgb1[2];

    return dr * dr + dg * dg + db * db;
}

static
void CheckLab2RGB(void)
{
    cmsHPROFILE hLab = cmsCreateLab4Profile(NULL);
    cmsHPROFILE hRGB = cmsOpenProfileFromFile("test3.icc", "r");
    cmsContext noPlugin = cmsCreateContext(0, 0);

    cmsHTRANSFORM hXformNoPlugin = cmsCreateTransformTHR(noPlugin, hLab, TYPE_Lab_FLT, hRGB, TYPE_RGB_FLT, INTENT_RELATIVE_COLORIMETRIC, cmsFLAGS_NOCACHE);
    cmsHTRANSFORM hXformPlugin = cmsCreateTransformTHR(0, hLab, TYPE_Lab_FLT, hRGB, TYPE_RGB_FLT, INTENT_RELATIVE_COLORIMETRIC, cmsFLAGS_NOCACHE);

    cmsFloat32Number Lab[3], RGB[3], RGB2[3];

    cmsFloat32Number maxInside = 0, maxOutside = 0, L, a, b;

    printf("Checking Lab -> RGB...");
    for (L = 4; L <= 100; L++)
    {
        for (a = -30; a < +30; a++)
            for (b = -30; b < +30; b++)
            {
                cmsFloat32Number d;

                Lab[0] = L; Lab[1] = a; Lab[2] = b;
                cmsDoTransform(hXformNoPlugin, Lab, RGB, 1);
                cmsDoTransform(hXformPlugin, Lab, RGB2, 1);

                d = distance(RGB, RGB2);
                if (d > maxInside)
                    maxInside = d;                   
            }
    }


    for (L = 1; L <= 100; L += 5)
    {
        for (a = -100; a < +100; a += 5)
            for (b = -100; b < +100; b += 5)
            {
                cmsFloat32Number d;

                Lab[0] = L; Lab[1] = a; Lab[2] = b;
                cmsDoTransform(hXformNoPlugin, Lab, RGB, 1);
                cmsDoTransform(hXformPlugin, Lab, RGB2, 1);

                d = distance(RGB, RGB2);
                if (d > maxOutside)
                    maxOutside = d;
            }

    }


    printf("Max distance: Inside gamut %f, Outside gamut %f\n", sqrtf(maxInside), sqrtf(maxOutside));

    cmsDeleteTransform(hXformNoPlugin);
    cmsDeleteTransform(hXformPlugin);

    cmsDeleteContext(noPlugin);    
}




// --------------------------------------------------------------------------------------------------
// P E R F O R M A N C E   C H E C K S
// --------------------------------------------------------------------------------------------------


static 
cmsFloat64Number MPixSec(cmsFloat64Number diff)
{
       cmsFloat64Number seconds = (cmsFloat64Number)diff / (cmsFloat64Number)CLOCKS_PER_SEC;
       return (256.0 * 256.0 * 256.0) / (1024.0*1024.0*seconds);
}

typedef cmsFloat64Number(*perf_fn)(cmsContext ct, cmsHPROFILE hlcmsProfileIn, cmsHPROFILE hlcmsProfileOut);


static
void PerformanceHeader(void)
{
       printf("                                  MPixel/sec.   MByte/sec.\n");
}


static
cmsHPROFILE loadProfile(const char* name)
{
    if (*name == '*')
    {
        if (strcmp(name, "*lab") == 0)
        {
            return cmsCreateLab4Profile(NULL);
        }
        else
            if (strcmp(name, "*xyz") == 0)
            {
                return cmsCreateXYZProfile();
            }
            else
                if (strcmp(name, "*curves") == 0)
                {
                    return CreateCurves();
                }
                else
                    Fail("Unknown builtin '%s'", name);

    }
    
    return cmsOpenProfileFromFile(name, "r");
}


static
cmsFloat64Number Performance(const char* Title, perf_fn fn, cmsContext ct, const char* inICC, const char* outICC, size_t sz, cmsFloat64Number prev)
{
       cmsHPROFILE hlcmsProfileIn = loadProfile(inICC);
       cmsHPROFILE hlcmsProfileOut = loadProfile(outICC);
                 
       cmsFloat64Number n = fn(ct, hlcmsProfileIn, hlcmsProfileOut);

       printf("%-30s: ", Title); fflush(stdout);
       printf("%-12.2f %-12.2f", n, n * sz);

       if (prev > 0.0) {

              cmsFloat64Number imp = n / prev;
              if (imp > 1)
                   printf(" (x %-2.1f)",  imp);
       }

       printf("\n"); fflush(stdout);
       return n;
}


static
void ComparativeCt(cmsContext ct1, cmsContext ct2, const char* Title, perf_fn fn1, perf_fn fn2, const char* inICC, const char* outICC)
{
       cmsHPROFILE hlcmsProfileIn;
       cmsHPROFILE hlcmsProfileOut;

       if (inICC == NULL)
              hlcmsProfileIn = CreateCurves();
       else
              hlcmsProfileIn = cmsOpenProfileFromFile(inICC, "r");

       if (outICC == NULL)
              hlcmsProfileOut = CreateCurves();
       else
              hlcmsProfileOut = cmsOpenProfileFromFile(outICC, "r");


       cmsFloat64Number n1 = fn1(ct1, hlcmsProfileIn, hlcmsProfileOut);

       if (inICC == NULL)
              hlcmsProfileIn = CreateCurves();
       else
              hlcmsProfileIn = cmsOpenProfileFromFile(inICC, "r");

       if (outICC == NULL)
              hlcmsProfileOut = CreateCurves();
       else
              hlcmsProfileOut = cmsOpenProfileFromFile(outICC, "r");

       cmsFloat64Number n2 = fn2(ct2, hlcmsProfileIn, hlcmsProfileOut);


       printf("%-30s: ", Title); fflush(stdout);
       printf("%-12.2f %-12.2f\n", n1, n2);
}

static
void Comparative(const char* Title, perf_fn fn1, perf_fn fn2, const char* inICC, const char* outICC)
{
       ComparativeCt(0, 0, Title, fn1, fn2, inICC, outICC);
}

// The worst case is used, no cache and all rgb combinations
static
cmsFloat64Number SpeedTest8bitsRGB(cmsContext ct, cmsHPROFILE hlcmsProfileIn, cmsHPROFILE hlcmsProfileOut)
{
       cmsInt32Number r, g, b, j;
       clock_t atime;
       cmsFloat64Number diff;
       cmsHTRANSFORM hlcmsxform;
       Scanline_rgb8bits *In;
       cmsUInt32Number Mb;

       if (hlcmsProfileIn == NULL || hlcmsProfileOut == NULL)
              Fail("Unable to open profiles");

       hlcmsxform = cmsCreateTransformTHR(ct, hlcmsProfileIn, TYPE_RGB_8, hlcmsProfileOut, TYPE_RGB_8, INTENT_PERCEPTUAL, cmsFLAGS_NOCACHE);
       cmsCloseProfile(hlcmsProfileIn);
       cmsCloseProfile(hlcmsProfileOut);

       Mb = 256 * 256 * 256 * sizeof(Scanline_rgb8bits);
       In = (Scanline_rgb8bits*)malloc(Mb);

       j = 0;
       for (r = 0; r < 256; r++)
              for (g = 0; g < 256; g++)
                     for (b = 0; b < 256; b++) {

                            In[j].r = (cmsUInt8Number)r;
                            In[j].g = (cmsUInt8Number)g;
                            In[j].b = (cmsUInt8Number)b;

                            j++;
                     }

       atime = clock();

       cmsDoTransform(hlcmsxform, In, In, 256 * 256 * 256);

       diff = clock() - atime;
       free(In);

       cmsDeleteTransform(hlcmsxform);

       return MPixSec(diff);
}

static
cmsFloat64Number SpeedTest8bitsRGBA(cmsContext ct, cmsHPROFILE hlcmsProfileIn, cmsHPROFILE hlcmsProfileOut)
{
       cmsInt32Number r, g, b, j;
       clock_t atime;
       cmsFloat64Number diff;
       cmsHTRANSFORM hlcmsxform;
       Scanline_rgba8bits *In;
       cmsUInt32Number Mb;

       if (hlcmsProfileIn == NULL || hlcmsProfileOut == NULL)
              Fail("Unable to open profiles");

       hlcmsxform = cmsCreateTransformTHR(ct, hlcmsProfileIn, TYPE_RGBA_8, hlcmsProfileOut, TYPE_RGBA_8, INTENT_PERCEPTUAL, cmsFLAGS_NOCACHE);
       cmsCloseProfile(hlcmsProfileIn);
       cmsCloseProfile(hlcmsProfileOut);

       Mb = 256 * 256 * 256 * sizeof(Scanline_rgba8bits);
       In = (Scanline_rgba8bits*)malloc(Mb);

       j = 0;
       for (r = 0; r < 256; r++)
              for (g = 0; g < 256; g++)
                     for (b = 0; b < 256; b++) {

                            In[j].r = (cmsUInt8Number)r;
                            In[j].g = (cmsUInt8Number)g;
                            In[j].b = (cmsUInt8Number)b;
                            In[j].a = 0;

                            j++;
                     }

       atime = clock();

       cmsDoTransform(hlcmsxform, In, In, 256 * 256 * 256);

       diff = clock() - atime;
       free(In);

       cmsDeleteTransform(hlcmsxform);
       return MPixSec(diff);

}


// The worst case is used, no cache and all rgb combinations
static
cmsFloat64Number SpeedTest15bitsRGB(cmsContext ct, cmsHPROFILE hlcmsProfileIn, cmsHPROFILE hlcmsProfileOut)
{
       cmsInt32Number r, g, b, j;
       clock_t atime;
       cmsFloat64Number diff;
       cmsHTRANSFORM hlcmsxform;
       Scanline_rgb15bits *In;
       cmsUInt32Number Mb;

       if (hlcmsProfileIn == NULL || hlcmsProfileOut == NULL)
              Fail("Unable to open profiles");

       hlcmsxform = cmsCreateTransformTHR(ct, hlcmsProfileIn, TYPE_RGB_15, hlcmsProfileOut, TYPE_RGB_15, INTENT_PERCEPTUAL, cmsFLAGS_NOCACHE);
       cmsCloseProfile(hlcmsProfileIn);
       cmsCloseProfile(hlcmsProfileOut);

       Mb = 256 * 256 * 256 * sizeof(Scanline_rgb15bits);
       In = (Scanline_rgb15bits*)malloc(Mb);

       j = 0;
       for (r = 0; r < 256; r++)
              for (g = 0; g < 256; g++)
                     for (b = 0; b < 256; b++) {

              In[j].r = (cmsUInt16Number)r;
              In[j].g = (cmsUInt16Number)g;
              In[j].b = (cmsUInt16Number)b;

              j++;
      }
      
       atime = clock();

       cmsDoTransform(hlcmsxform, In, In, 256 * 256 * 256);

       diff = clock() - atime;
       free(In);
       
       cmsDeleteTransform(hlcmsxform);

       return MPixSec(diff);
}

static
cmsFloat64Number SpeedTest15bitsRGBA(cmsContext ct, cmsHPROFILE hlcmsProfileIn, cmsHPROFILE hlcmsProfileOut)
{
       cmsInt32Number r, g, b, j;
       clock_t atime;
       cmsFloat64Number diff;
       cmsHTRANSFORM hlcmsxform;
       Scanline_rgba15bits *In;
       cmsUInt32Number Mb;

       if (hlcmsProfileIn == NULL || hlcmsProfileOut == NULL)
              Fail("Unable to open profiles");

       hlcmsxform = cmsCreateTransformTHR(ct, hlcmsProfileIn, TYPE_RGBA_15, hlcmsProfileOut, TYPE_RGBA_15, INTENT_PERCEPTUAL, cmsFLAGS_NOCACHE);
       cmsCloseProfile(hlcmsProfileIn);
       cmsCloseProfile(hlcmsProfileOut);

       Mb = 256 * 256 * 256 * sizeof(Scanline_rgba15bits);
       In = (Scanline_rgba15bits*)malloc(Mb);

       j = 0;
       for (r = 0; r < 256; r++)
              for (g = 0; g < 256; g++)
                     for (b = 0; b < 256; b++) {

              In[j].r = (cmsUInt16Number)r;
              In[j].g = (cmsUInt16Number)g;
              In[j].b = (cmsUInt16Number)b;
              In[j].a = 0;

              j++;
       }      

       atime = clock();

       cmsDoTransform(hlcmsxform, In, In, 256 * 256 * 256);

       diff = clock() - atime;
       free(In);

       cmsDeleteTransform(hlcmsxform);
       return MPixSec(diff);

}

static
cmsFloat64Number SpeedTest15bitsCMYK(cmsContext ct, cmsHPROFILE hlcmsProfileIn, cmsHPROFILE hlcmsProfileOut)
{

       cmsInt32Number r, g, b, j;
       clock_t atime;
       cmsFloat64Number diff;
       cmsHTRANSFORM hlcmsxform;
       Scanline_cmyk15bits *In;
       cmsUInt32Number Mb;

       if (hlcmsProfileIn == NULL || hlcmsProfileOut == NULL)
              Fail("Unable to open profiles");

       hlcmsxform = cmsCreateTransformTHR(ct, hlcmsProfileIn, TYPE_CMYK_15, hlcmsProfileOut, TYPE_CMYK_15, INTENT_PERCEPTUAL, cmsFLAGS_NOCACHE);
       cmsCloseProfile(hlcmsProfileIn);
       cmsCloseProfile(hlcmsProfileOut);

       Mb = 256 * 256 * 256 * sizeof(Scanline_cmyk15bits);
       In = (Scanline_cmyk15bits*)malloc(Mb);

       j = 0;
       for (r = 0; r < 256; r++)
              for (g = 0; g < 256; g++)
                     for (b = 0; b < 256; b++) {

              In[j].r = (cmsUInt16Number)r;
              In[j].g = (cmsUInt16Number)g;
              In[j].b = (cmsUInt16Number)b;
              In[j].a = (cmsUInt16Number)0;

              j++;
       }

       atime = clock();

       cmsDoTransform(hlcmsxform, In, In, 256 * 256 * 256);

       diff = clock() - atime;
       free(In);

       cmsDeleteTransform(hlcmsxform);
       return MPixSec(diff);
}

// The worst case is used, no cache and all rgb combinations
static
cmsFloat64Number SpeedTest16bitsRGB(cmsContext ct, cmsHPROFILE hlcmsProfileIn, cmsHPROFILE hlcmsProfileOut)
{
    cmsInt32Number r, g, b, j;
    clock_t atime;
    cmsFloat64Number diff;
    cmsHTRANSFORM hlcmsxform;
    Scanline_rgb16bits *In;
    cmsUInt32Number Mb;

    if (hlcmsProfileIn == NULL || hlcmsProfileOut == NULL)
        Fail("Unable to open profiles");

    hlcmsxform = cmsCreateTransformTHR(ct, hlcmsProfileIn, TYPE_RGB_16, hlcmsProfileOut, TYPE_RGB_16, INTENT_PERCEPTUAL, cmsFLAGS_NOCACHE);
    cmsCloseProfile(hlcmsProfileIn);
    cmsCloseProfile(hlcmsProfileOut);

    Mb = 256 * 256 * 256 * sizeof(Scanline_rgb16bits);
    In = (Scanline_rgb16bits*)malloc(Mb);

    j = 0;
    for (r = 0; r < 256; r++)
        for (g = 0; g < 256; g++)
            for (b = 0; b < 256; b++) {

                In[j].r = (cmsUInt16Number)FROM_8_TO_16(r);
                In[j].g = (cmsUInt16Number)FROM_8_TO_16(g);
                In[j].b = (cmsUInt16Number)FROM_8_TO_16(b);

                j++;
            }

    atime = clock();

    cmsDoTransform(hlcmsxform, In, In, 256 * 256 * 256);

    diff = clock() - atime;
    free(In);

    cmsDeleteTransform(hlcmsxform);

    return MPixSec(diff);
}

static
cmsFloat64Number SpeedTest16bitsCMYK(cmsContext ct, cmsHPROFILE hlcmsProfileIn, cmsHPROFILE hlcmsProfileOut)
{

    cmsInt32Number r, g, b, j;
    clock_t atime;
    cmsFloat64Number diff;
    cmsHTRANSFORM hlcmsxform;
    Scanline_cmyk16bits* In;
    cmsUInt32Number Mb;

    if (hlcmsProfileIn == NULL || hlcmsProfileOut == NULL)
        Fail("Unable to open profiles");

    hlcmsxform = cmsCreateTransformTHR(ct, hlcmsProfileIn, TYPE_CMYK_16, hlcmsProfileOut, TYPE_CMYK_16, INTENT_PERCEPTUAL, cmsFLAGS_NOCACHE);
    cmsCloseProfile(hlcmsProfileIn);
    cmsCloseProfile(hlcmsProfileOut);

    Mb = 256 * 256 * 256 * sizeof(Scanline_cmyk16bits);
    In = (Scanline_cmyk16bits*)malloc(Mb);

    j = 0;
    for (r = 0; r < 256; r++)
        for (g = 0; g < 256; g++)
            for (b = 0; b < 256; b++) {

                In[j].c = (cmsUInt16Number)r;
                In[j].m = (cmsUInt16Number)g;
                In[j].y = (cmsUInt16Number)b;
                In[j].k = (cmsUInt16Number)r;

                j++;
            }

    atime = clock();

    cmsDoTransform(hlcmsxform, In, In, 256 * 256 * 256);

    diff = clock() - atime;
    free(In);

    cmsDeleteTransform(hlcmsxform);
    return MPixSec(diff);
}



static
void SpeedTest8(void)
{
    cmsContext noPlugin = cmsCreateContext(0, 0);

    cmsFloat64Number t[10];

    printf("\n\n");
    printf("P E R F O R M A N C E   T E S T S   8 B I T S  (D E F A U L T)\n");
    printf("==============================================================\n\n");
    fflush(stdout);

    PerformanceHeader();
    t[0] = Performance("8 bits on CLUT profiles  ", SpeedTest8bitsRGB, noPlugin, "test5.icc", "test3.icc", sizeof(Scanline_rgb8bits), 0);
    t[1] = Performance("8 bits on Matrix-Shaper  ", SpeedTest8bitsRGB, noPlugin, "test5.icc", "test0.icc", sizeof(Scanline_rgb8bits), 0);
    t[2] = Performance("8 bits on same MatrixSh  ", SpeedTest8bitsRGB, noPlugin, "test0.icc", "test0.icc", sizeof(Scanline_rgb8bits), 0);
    t[3] = Performance("8 bits on curves         ", SpeedTest8bitsRGB, noPlugin, "*curves",   "*curves",   sizeof(Scanline_rgb8bits), 0);

    // Note that context 0 has the plug-in installed

    printf("\n\n");
    printf("P E R F O R M A N C E   T E S T S  8 B I T S  (P L U G I N)\n");
    printf("===========================================================\n\n");
    fflush(stdout);

    PerformanceHeader();
    Performance("8 bits on CLUT profiles  ", SpeedTest8bitsRGB, 0, "test5.icc", "test3.icc", sizeof(Scanline_rgb8bits), t[0]);
    Performance("8 bits on Matrix-Shaper  ", SpeedTest8bitsRGB, 0, "test5.icc", "test0.icc", sizeof(Scanline_rgb8bits), t[1]);
    Performance("8 bits on same MatrixSh  ", SpeedTest8bitsRGB, 0, "test0.icc", "test0.icc", sizeof(Scanline_rgb8bits), t[2]);
    Performance("8 bits on curves         ", SpeedTest8bitsRGB, 0, "*curves",   "*curves",   sizeof(Scanline_rgb8bits), t[3]);

    cmsDeleteContext(noPlugin);
}

static
void SpeedTest15(void)
{
       printf("\n\nP E R F O R M A N C E   T E S T S   1 5  B I T S  (P L U G I N)\n");
       printf(    "===============================================================\n\n");
       
       PerformanceHeader();
       Performance("15 bits on CLUT profiles         ", SpeedTest15bitsRGB, 0, "test5.icc", "test3.icc",  sizeof(Scanline_rgb15bits), 0);
       Performance("15 bits on Matrix-Shaper profiles", SpeedTest15bitsRGB, 0, "test5.icc", "test0.icc",  sizeof(Scanline_rgb15bits), 0);
       Performance("15 bits on same Matrix-Shaper    ", SpeedTest15bitsRGB, 0, "test0.icc", "test0.icc",  sizeof(Scanline_rgb15bits), 0);
       Performance("15 bits on curves                ", SpeedTest15bitsRGB, 0, "*curves",   "*curves",    sizeof(Scanline_rgb15bits), 0);
       Performance("15 bits on CMYK CLUT profiles    ", SpeedTest15bitsCMYK, 0, "test1.icc", "test2.icc", sizeof(Scanline_rgba15bits), 0);
}

static
void SpeedTest16(void)
{
    cmsContext noPlugin = cmsCreateContext(0, 0);


    printf("\n\n");
    printf("P E R F O R M A N C E   T E S T S   1 6  B I T S  (D E F A U L T)\n");
    printf("=================================================================\n\n");
    
    PerformanceHeader();
    Performance("16 bits on CLUT profiles         ", SpeedTest16bitsRGB,  noPlugin, "test5.icc", "test3.icc",  sizeof(Scanline_rgb16bits), 0);
    Performance("16 bits on Matrix-Shaper profiles", SpeedTest16bitsRGB,  noPlugin, "test5.icc", "test0.icc",  sizeof(Scanline_rgb16bits), 0);
    Performance("16 bits on same Matrix-Shaper    ", SpeedTest16bitsRGB,  noPlugin, "test0.icc", "test0.icc",  sizeof(Scanline_rgb16bits), 0);
    Performance("16 bits on curves                ", SpeedTest16bitsRGB,  noPlugin, "*curves",   "*curves",    sizeof(Scanline_rgb16bits), 0);
    Performance("16 bits on CMYK CLUT profiles    ", SpeedTest16bitsCMYK, noPlugin, "test1.icc", "test2.icc",  sizeof(Scanline_cmyk16bits), 0);
    
    printf("\n\n");
    printf("P E R F O R M A N C E   T E S T S   1 6  B I T S  (P L U G I N)\n");
    printf("===============================================================\n\n");

    PerformanceHeader();
    Performance("16 bits on CLUT profiles         ", SpeedTest16bitsRGB,  0, "test5.icc", "test3.icc", sizeof(Scanline_rgb16bits), 0);
    Performance("16 bits on Matrix-Shaper profiles", SpeedTest16bitsRGB,  0, "test5.icc", "test0.icc", sizeof(Scanline_rgb16bits), 0);
    Performance("16 bits on same Matrix-Shaper    ", SpeedTest16bitsRGB,  0, "test0.icc", "test0.icc", sizeof(Scanline_rgb16bits), 0);
    Performance("16 bits on curves                ", SpeedTest16bitsRGB,  0, "*curves",   "*curves",   sizeof(Scanline_rgb16bits), 0);
    Performance("16 bits on CMYK CLUT profiles    ", SpeedTest16bitsCMYK, 0, "test1.icc", "test2.icc", sizeof(Scanline_cmyk16bits), 0);
}

// The worst case is used, no cache and all rgb combinations
static
cmsFloat64Number SpeedTestFloatRGB(cmsContext ct, cmsHPROFILE hlcmsProfileIn, cmsHPROFILE hlcmsProfileOut)
{
       cmsInt32Number j;
       clock_t atime;
       cmsFloat64Number diff;
       cmsHTRANSFORM hlcmsxform;
       void *In;
       cmsUInt32Number size, Mb;
       cmsUInt32Number inFormatter=0, outFormatter=0;
       cmsFloat64Number seconds;

       if (hlcmsProfileIn == NULL || hlcmsProfileOut == NULL)
           Fail("Unable to open profiles");


       switch (cmsGetColorSpace(hlcmsProfileIn))
       {
       case cmsSigRgbData: inFormatter = TYPE_RGB_FLT; break;
       case cmsSigLabData: inFormatter = TYPE_Lab_FLT; break;
       
       default:
           Fail("Invalid colorspace");
       }

       switch (cmsGetColorSpace(hlcmsProfileOut))
       {
       case cmsSigRgbData:  outFormatter = TYPE_RGB_FLT; break;
       case cmsSigLabData:  outFormatter = TYPE_Lab_FLT; break;
       case cmsSigXYZData:  outFormatter = TYPE_XYZ_FLT; break;

       default:
           Fail("Invalid colorspace");
       }

       hlcmsxform = cmsCreateTransformTHR(ct, hlcmsProfileIn, inFormatter, hlcmsProfileOut, outFormatter, INTENT_PERCEPTUAL, cmsFLAGS_NOCACHE);
       cmsCloseProfile(hlcmsProfileIn);
       cmsCloseProfile(hlcmsProfileOut);

 

       j = 0;

       if (inFormatter == TYPE_RGB_FLT)
       {
           cmsInt32Number r, g, b;
           Scanline_rgbFloat* fill;
           
           size = 256 * 256 * 256;
           Mb = size * sizeof(Scanline_rgbFloat);
           In = malloc(Mb);
           fill = (Scanline_rgbFloat*)In;

           for (r = 0; r < 256; r++)
               for (g = 0; g < 256; g++)
                   for (b = 0; b < 256; b++) {

                       fill[j].r = (cmsFloat32Number)r / 255.0f;
                       fill[j].g = (cmsFloat32Number)g / 255.0f;
                       fill[j].b = (cmsFloat32Number)b / 255.0f;

                       j++;
                   }

       }
       else
       {
           cmsFloat32Number L, a, b;
           Scanline_LabFloat* fill;

           size = 100 * 256 * 256;
           Mb =  size * sizeof(Scanline_LabFloat);
           In = malloc(Mb);
           fill = (Scanline_LabFloat*)In;

           for (L = 0; L < 100; L++)
               for (a = -127.0; a < 127.0; a++)
                   for (b = -127.0; b < +127.0; b++) {

                       fill[j].L = L;
                       fill[j].a = a;
                       fill[j].b = b;

                       j++;
                   }
       }

       atime = clock();

       cmsDoTransform(hlcmsxform, In, In, size);

       diff = clock() - atime;
       free(In);

       cmsDeleteTransform(hlcmsxform);
       
       seconds = (cmsFloat64Number)diff / (cmsFloat64Number)CLOCKS_PER_SEC;
       return ((cmsFloat64Number)size) / (1024.0 * 1024.0 * seconds);       
}


static
cmsFloat64Number SpeedTestFloatCMYK(cmsContext ct, cmsHPROFILE hlcmsProfileIn, cmsHPROFILE hlcmsProfileOut)
{
    cmsInt32Number c, m, y, k, j;
    clock_t atime;
    cmsFloat64Number diff;
    cmsHTRANSFORM hlcmsxform;
    Scanline_cmykFloat* In;
    cmsUInt32Number Mb;
    
    if (hlcmsProfileIn == NULL || hlcmsProfileOut == NULL)
        Fail("Unable to open profiles");

    
    hlcmsxform = cmsCreateTransformTHR(ct, hlcmsProfileIn, TYPE_CMYK_FLT, hlcmsProfileOut, TYPE_CMYK_FLT, INTENT_PERCEPTUAL, cmsFLAGS_NOCACHE);
    cmsCloseProfile(hlcmsProfileIn);
    cmsCloseProfile(hlcmsProfileOut);

    Mb = 64 * 64 * 64 * 64 * sizeof(Scanline_cmykFloat);
    In = (Scanline_cmykFloat*)malloc(Mb);

    j = 0;
    for (c = 0; c < 256; c += 4)
        for (m = 0; m < 256; m += 4)
            for (y = 0; y < 256; y += 4)
                for (k = 0; k < 256; k += 4) {

                In[j].c = (cmsFloat32Number)c / 255.0f;
                In[j].m = (cmsFloat32Number)m / 255.0f;
                In[j].y = (cmsFloat32Number)y / 255.0f;
                In[j].k = (cmsFloat32Number)k / 255.0f;

                j++;
            }

    atime = clock();

    cmsDoTransform(hlcmsxform, In, In, 64 * 64 * 64 * 64);

    diff = clock() - atime;
    free(In);

    cmsDeleteTransform(hlcmsxform);
    return MPixSec(diff);
}


static
cmsFloat64Number SpeedTestFloatLab(cmsContext ct, cmsHPROFILE hlcmsProfileIn, cmsHPROFILE hlcmsProfileOut)
{
    cmsInt32Number j;
    clock_t atime;
    cmsFloat64Number diff;
    cmsHTRANSFORM hlcmsxform;
    void* In;
    cmsUInt32Number size, Mb;
    cmsUInt32Number  outFormatter = 0;
    cmsFloat64Number seconds;
    cmsFloat32Number L, a, b;
    Scanline_LabFloat* fill;


    if (hlcmsProfileIn == NULL || hlcmsProfileOut == NULL)
        Fail("Unable to open profiles");


    if (cmsGetColorSpace(hlcmsProfileIn) != cmsSigLabData)
    {
        Fail("Invalid colorspace");
    }

    switch (cmsGetColorSpace(hlcmsProfileOut))
    {
    case cmsSigRgbData:  outFormatter = TYPE_RGB_FLT; break;
    case cmsSigLabData:  outFormatter = TYPE_Lab_FLT; break;
    case cmsSigXYZData:  outFormatter = TYPE_XYZ_FLT; break;

    default:
        Fail("Invalid colorspace");
    }

    hlcmsxform = cmsCreateTransformTHR(ct, hlcmsProfileIn, TYPE_Lab_FLT, hlcmsProfileOut, outFormatter, INTENT_PERCEPTUAL, cmsFLAGS_NOCACHE);
    cmsCloseProfile(hlcmsProfileIn);
    cmsCloseProfile(hlcmsProfileOut);

    j = 0;

    size = 100 * 256 * 256;
    Mb = size * sizeof(Scanline_LabFloat);
    In = malloc(Mb);
    fill = (Scanline_LabFloat*)In;

    for (L = 0; L < 100; L++)
        for (a = -127.0; a < 127.0; a++)
            for (b = -127.0; b < +127.0; b++) {

                fill[j].L = L;
                fill[j].a = a;
                fill[j].b = b;

                j++;
            }
    

    atime = clock();

    cmsDoTransform(hlcmsxform, In, In, size);

    diff = clock() - atime;
    free(In);

    cmsDeleteTransform(hlcmsxform);

    seconds = (cmsFloat64Number)diff / (cmsFloat64Number)CLOCKS_PER_SEC;
    return ((cmsFloat64Number)size) / (1024.0 * 1024.0 * seconds);
}



static
void SpeedTestFloat(void)
{
       cmsContext noPlugin = cmsCreateContext(0, 0);
       
       cmsFloat64Number t[10] = { 0 };

       printf("\n\n");
       printf("P E R F O R M A N C E   T E S T S   F L O A T  (D E F A U L T)\n");
       printf("==============================================================\n\n");
       fflush(stdout);

       PerformanceHeader();
       t[0] = Performance("Floating point on CLUT profiles  ", SpeedTestFloatRGB, noPlugin, "test5.icc", "test3.icc", sizeof(Scanline_rgbFloat), 0);
       t[1] = Performance("Floating point on Matrix-Shaper  ", SpeedTestFloatRGB, noPlugin, "test5.icc", "test0.icc", sizeof(Scanline_rgbFloat), 0);
       t[2] = Performance("Floating point on same MatrixSh  ", SpeedTestFloatRGB, noPlugin, "test0.icc", "test0.icc", sizeof(Scanline_rgbFloat), 0);
       t[3] = Performance("Floating point on curves         ", SpeedTestFloatRGB, noPlugin, "*curves", "*curves",     sizeof(Scanline_rgbFloat), 0);
       t[4] = Performance("Floating point on RGB->Lab       ", SpeedTestFloatRGB, noPlugin, "test5.icc", "*lab",      sizeof(Scanline_rgbFloat), 0);
       t[5] = Performance("Floating point on RGB->XYZ       ", SpeedTestFloatRGB, noPlugin, "test3.icc", "*xyz",      sizeof(Scanline_rgbFloat), 0);
       t[6] = Performance("Floating point on CMYK->CMYK     ", SpeedTestFloatCMYK, noPlugin, "test1.icc", "test2.icc",sizeof(Scanline_cmykFloat), 0);
       t[7] = Performance("Floating point on Lab->RGB       ", SpeedTestFloatLab,  noPlugin, "*lab",     "test3.icc", sizeof(Scanline_LabFloat), 0);


       // Note that context 0 has the plug-in installed

       printf("\n\n");
       printf("P E R F O R M A N C E   T E S T S  F L O A T  (P L U G I N)\n");
       printf("===========================================================\n\n");
       fflush(stdout);

       PerformanceHeader();                   
       Performance("Floating point on CLUT profiles  ", SpeedTestFloatRGB, 0, "test5.icc", "test3.icc", sizeof(Scanline_rgbFloat), t[0]);
       Performance("Floating point on Matrix-Shaper  ", SpeedTestFloatRGB, 0, "test5.icc", "test0.icc", sizeof(Scanline_rgbFloat), t[1]);
       Performance("Floating point on same MatrixSh  ", SpeedTestFloatRGB, 0, "test0.icc", "test0.icc", sizeof(Scanline_rgbFloat), t[2]);
       Performance("Floating point on curves         ", SpeedTestFloatRGB, 0, "*curves", "*curves",     sizeof(Scanline_rgbFloat), t[3]);
       Performance("Floating point on RGB->Lab       ", SpeedTestFloatRGB, 0, "test5.icc", "*lab",      sizeof(Scanline_rgbFloat), t[4]);
       Performance("Floating point on RGB->XYZ       ", SpeedTestFloatRGB, 0, "test3.icc", "*xyz",      sizeof(Scanline_rgbFloat), t[5]);
       Performance("Floating point on CMYK->CMYK     ", SpeedTestFloatCMYK, 0, "test1.icc", "test2.icc", sizeof(Scanline_cmykFloat), t[6]);
       Performance("Floating point on Lab->RGB       ", SpeedTestFloatLab,  0, "*lab",      "test3.icc", sizeof(Scanline_LabFloat), t[7]);

       cmsDeleteContext(noPlugin);
}


static
cmsFloat64Number SpeedTestFloatByUsing16BitsRGB(cmsContext ct, cmsHPROFILE hlcmsProfileIn, cmsHPROFILE hlcmsProfileOut)
{
       cmsInt32Number r, g, b, j;
       clock_t atime;
       cmsFloat64Number diff;
       cmsHTRANSFORM xform16;
       Scanline_rgbFloat *In;
       Scanline_rgb16bits *tmp16;       
       cmsUInt32Number MbFloat, Mb16;

       UNUSED_PARAMETER(ct);

       if (hlcmsProfileIn == NULL || hlcmsProfileOut == NULL)
              Fail("Unable to open profiles");

       xform16    = cmsCreateTransformTHR(0, hlcmsProfileIn, TYPE_RGB_16, hlcmsProfileOut, TYPE_RGB_16, INTENT_PERCEPTUAL, cmsFLAGS_NOCACHE);

       cmsCloseProfile(hlcmsProfileIn);
       cmsCloseProfile(hlcmsProfileOut);

       MbFloat = 256 * 256 * 256 * sizeof(Scanline_rgbFloat);
       Mb16    = 256 * 256 * 256 * sizeof(Scanline_rgb16bits);

       In    = (Scanline_rgbFloat*)malloc(MbFloat);       
       tmp16 = (Scanline_rgb16bits*)malloc(Mb16);
       

       j = 0;
       for (r = 0; r < 256; r++)
              for (g = 0; g < 256; g++)
                     for (b = 0; b < 256; b++) {

              In[j].r = (cmsFloat32Number)r / 255.0f;
              In[j].g = (cmsFloat32Number)g / 255.0f;
              In[j].b = (cmsFloat32Number)b / 255.0f;

              j++;
       }
      

       atime = clock();

       
       for (j = 0; j < 256 * 256 * 256; j++) {

              tmp16[j].r = (cmsUInt16Number)floor(In[j].r * 65535.0 + 0.5);
              tmp16[j].g = (cmsUInt16Number)floor(In[j].g * 65535.0 + 0.5);
              tmp16[j].b = (cmsUInt16Number)floor(In[j].b * 65535.0 + 0.5);

              j++;
       }

       cmsDoTransform(xform16, tmp16, tmp16, 256 * 256 * 256);

       for (j = 0; j < 256 * 256 * 256; j++) {

              In[j].r = (cmsFloat32Number) (tmp16[j].r / 65535.0 );
              In[j].g = (cmsFloat32Number) (tmp16[j].g / 65535.0);
              In[j].b = (cmsFloat32Number) (tmp16[j].b / 65535.0);

              j++;
       }

       diff = clock() - atime;
       free(In);
      
       cmsDeleteTransform(xform16);
       return MPixSec(diff);
}





static
void ComparativeFloatVs16bits(void)
{
       printf("\n\n");
       printf("C O M P A R A T I V E  converting to 16 bit vs. using float plug-in.\n");
       printf("                              values given in MegaPixels per second.\n");
       printf("====================================================================\n");       
       printf("                                  16 bits tmp.  Float plugin\n");
       fflush(stdout);

       Comparative("Floating point on CLUT profiles  ", SpeedTestFloatByUsing16BitsRGB, SpeedTestFloatRGB,  "test5.icc", "test3.icc");
       Comparative("Floating point on Matrix-Shaper  ", SpeedTestFloatByUsing16BitsRGB, SpeedTestFloatRGB,  "test5.icc", "test0.icc");
       Comparative("Floating point on same MatrixSh  ", SpeedTestFloatByUsing16BitsRGB, SpeedTestFloatRGB,  "test0.icc", "test0.icc");
       Comparative("Floating point on curves         ", SpeedTestFloatByUsing16BitsRGB, SpeedTestFloatRGB,  NULL, NULL);
}







typedef struct
{
       Scanline_rgba8bits pixels[256][256];
       cmsUInt8Number     padding[4];

} padded_line;

typedef struct
{
       padded_line line[256];
} big_bitmap;


static
cmsFloat64Number SpeedTest8bitDoTransform(cmsContext ct, cmsHPROFILE hlcmsProfileIn, cmsHPROFILE hlcmsProfileOut)
{
       cmsInt32Number r, g, b, j;
       clock_t atime;
       cmsFloat64Number diff;
       cmsHTRANSFORM hlcmsxform;
       big_bitmap* In;
       big_bitmap* Out;
       cmsUInt32Number Mb;

       if (hlcmsProfileIn == NULL || hlcmsProfileOut == NULL)
              Fail("Unable to open profiles");

       hlcmsxform = cmsCreateTransformTHR(ct, hlcmsProfileIn, TYPE_RGBA_8, hlcmsProfileOut, TYPE_RGBA_8, INTENT_PERCEPTUAL, cmsFLAGS_NOCACHE);
       cmsCloseProfile(hlcmsProfileIn);
       cmsCloseProfile(hlcmsProfileOut);


       // Our test bitmap is 256 x 256 padded lines
       Mb = sizeof(big_bitmap);

       In = (big_bitmap*)malloc(Mb);
       Out = (big_bitmap*)malloc(Mb);

       for (r = 0; r < 256; r++)
              for (g = 0; g < 256; g++)
                     for (b = 0; b < 256; b++) {

                            In->line[r].pixels[g][b].r = (cmsUInt8Number)r;
                            In->line[r].pixels[g][b].g = (cmsUInt8Number)g;
                            In->line[r].pixels[g][b].b = (cmsUInt8Number)b;
                            In->line[r].pixels[g][b].a = 0;
                     }

       atime = clock();

       for (j = 0; j < 256; j++) {

              cmsDoTransform(hlcmsxform, In->line[j].pixels, Out->line[j].pixels, 256 * 256);
       }

       diff = clock() - atime;
       free(In); free(Out);

       cmsDeleteTransform(hlcmsxform);
       return MPixSec(diff);

}


static
cmsFloat64Number SpeedTest8bitLineStride(cmsContext ct, cmsHPROFILE hlcmsProfileIn, cmsHPROFILE hlcmsProfileOut)
{
       cmsInt32Number r, g, b;
       clock_t atime;
       cmsFloat64Number diff;
       cmsHTRANSFORM hlcmsxform;
       big_bitmap* In;
       big_bitmap* Out;
       cmsUInt32Number Mb;

       if (hlcmsProfileIn == NULL || hlcmsProfileOut == NULL)
              Fail("Unable to open profiles");

       hlcmsxform = cmsCreateTransformTHR(ct, hlcmsProfileIn, TYPE_RGBA_8, hlcmsProfileOut, TYPE_RGBA_8, INTENT_PERCEPTUAL, cmsFLAGS_NOCACHE);
       cmsCloseProfile(hlcmsProfileIn);
       cmsCloseProfile(hlcmsProfileOut);


       // Our test bitmap is 256 x 256 padded lines
       Mb = sizeof(big_bitmap);

       In = (big_bitmap*)malloc(Mb);
       Out = (big_bitmap*)malloc(Mb);

       for (r = 0; r < 256; r++)
              for (g = 0; g < 256; g++)
                     for (b = 0; b < 256; b++) {

                            In->line[r].pixels[g][b].r = (cmsUInt8Number)r;
                            In->line[r].pixels[g][b].g = (cmsUInt8Number)g;
                            In->line[r].pixels[g][b].b = (cmsUInt8Number)b;
                            In->line[r].pixels[g][b].a = 0;
                     }

       atime = clock();
       
       cmsDoTransformLineStride(hlcmsxform, In, Out, 256*256, 256, sizeof(padded_line), sizeof(padded_line), 0, 0);
       
       diff = clock() - atime;
       free(In); free(Out);

       cmsDeleteTransform(hlcmsxform);
       return MPixSec(diff);

}

static
void ComparativeLineStride8bits(void)
{
       cmsContext NoPlugin, Plugin;

       printf("\n\n");
       printf("C O M P A R A T I V E cmsDoTransform() vs. cmsDoTransformLineStride()\n");
       printf("                              values given in MegaPixels per second.\n");
       printf("====================================================================\n");

       fflush(stdout);

       NoPlugin = cmsCreateContext(NULL, NULL);
       Plugin = cmsCreateContext(cmsFastFloatExtensions(), NULL);

       ComparativeCt(NoPlugin, Plugin, "CLUT profiles  ", SpeedTest8bitDoTransform, SpeedTest8bitLineStride, "test5.icc", "test3.icc");
       ComparativeCt(NoPlugin, Plugin, "CLUT 16 bits   ", SpeedTest16bitsRGB,       SpeedTest16bitsRGB, "test5.icc", "test3.icc");
       ComparativeCt(NoPlugin, Plugin, "Matrix-Shaper  ", SpeedTest8bitDoTransform, SpeedTest8bitLineStride, "test5.icc", "test0.icc");
       ComparativeCt(NoPlugin, Plugin, "same MatrixSh  ", SpeedTest8bitDoTransform, SpeedTest8bitLineStride, "test0.icc", "test0.icc");
       ComparativeCt(NoPlugin, Plugin, "curves         ", SpeedTest8bitDoTransform, SpeedTest8bitLineStride, NULL, NULL);

       cmsDeleteContext(Plugin);
       cmsDeleteContext(NoPlugin);
}



static
void TestGrayTransformPerformance()
{
       cmsInt32Number j;
       clock_t atime;
       cmsFloat64Number diff;
       cmsHTRANSFORM hlcmsxform;
       float *In;

       cmsInt32Number pixels;
       cmsUInt32Number Mb;
       cmsToneCurve* gamma18;
       cmsToneCurve* gamma22;

       cmsHPROFILE hlcmsProfileIn;
       cmsHPROFILE hlcmsProfileOut;


       gamma18 = cmsBuildGamma(0, 1.8);
       gamma22 = cmsBuildGamma(0, 2.2);

       hlcmsProfileIn = cmsCreateGrayProfile(NULL, gamma18);
       hlcmsProfileOut = cmsCreateGrayProfile(NULL, gamma22);


       cmsFreeToneCurve(gamma18);
       cmsFreeToneCurve(gamma22);

       hlcmsxform = cmsCreateTransform(hlcmsProfileIn, TYPE_GRAY_FLT | EXTRA_SH(1), hlcmsProfileOut, TYPE_GRAY_FLT|EXTRA_SH(1), INTENT_PERCEPTUAL, 0);
       cmsCloseProfile(hlcmsProfileIn);
       cmsCloseProfile(hlcmsProfileOut);

       pixels = 256 * 256 * 256;
       Mb = pixels* 2*sizeof(float);
       In = (float*) malloc(Mb);

       for (j = 0; j < pixels*2; j++)
              In[j] = (j % 256) / 255.0f;

       atime = clock();

       cmsDoTransform(hlcmsxform, In, In, pixels);

       diff = clock() - atime;
       free(In);

       cmsDeleteTransform(hlcmsxform);
       printf("Gray conversion using two gray profiles\t %-12.2f MPixels/Sec.\n", MPixSec(diff));
}

static
void TestGrayTransformPerformance1()
{
       cmsInt32Number j;
       clock_t atime;
       cmsFloat64Number diff;
       cmsHTRANSFORM hlcmsxform;
       float *In;

       cmsInt32Number pixels;
       cmsUInt32Number Mb;
       cmsToneCurve* gamma18;
       cmsToneCurve* gamma22;

       cmsHPROFILE hlcmsProfileIn;
       cmsHPROFILE hlcmsProfileOut;


       gamma18 = cmsBuildGamma(0, 1.8);
       gamma22 = cmsBuildGamma(0, 1./2.2);

       hlcmsProfileIn = cmsCreateLinearizationDeviceLink(cmsSigGrayData, &gamma18);
       hlcmsProfileOut = cmsCreateLinearizationDeviceLink(cmsSigGrayData, &gamma22);


       cmsFreeToneCurve(gamma18);
       cmsFreeToneCurve(gamma22);

       hlcmsxform = cmsCreateTransform(hlcmsProfileIn, TYPE_GRAY_FLT, hlcmsProfileOut, TYPE_GRAY_FLT, INTENT_PERCEPTUAL, 0);
       cmsCloseProfile(hlcmsProfileIn);
       cmsCloseProfile(hlcmsProfileOut);

       pixels = 256 * 256 * 256;
       Mb = pixels* sizeof(float);
       In = (float*) malloc(Mb);

       for (j = 0; j < pixels; j++)
              In[j] = (j % 256) / 255.0f;

       atime = clock();

       cmsDoTransform(hlcmsxform, In, In, pixels);

       diff = clock() - atime;
       free(In);

       cmsDeleteTransform(hlcmsxform);
       printf("Gray conversion using two devicelinks\t %-12.2f MPixels/Sec.\n", MPixSec(diff));
}


// The harness test
int main()
{
       printf("FastFloating point extensions testbed - 1.3\n");
       printf("Copyright (c) 1998-2020 Marti Maria Saguer, all rights reserved\n");
       
       printf("\nInstalling error logger ... ");
       cmsSetLogErrorHandler(FatalErrorQuit);
       printf("done.\n");

       printf("Installing plug-in ... ");
       cmsPlugin(cmsFastFloatExtensions());
       printf("done.\n\n");
       
       
       CheckComputeIncrements();

       // 15 bit functionality
       CheckFormatters15();
       Check15bitsConversions();    
 
       // 16 bits functionality
       CheckAccuracy16Bits();

       // Lab to whatever
       CheckLab2RGB();

       // Change format
       CheckChangeFormat();

       // Floating point functionality
       CheckConversionFloat();  
       printf("All floating point tests passed OK\n");
                 
       SpeedTest8();
       SpeedTest16();
       SpeedTest15();
       SpeedTestFloat();
     
       ComparativeFloatVs16bits();
       ComparativeLineStride8bits();

       // Test gray performance
       printf("\n\n");
       printf("F L O A T   G R A Y   conversions performance.\n");
       printf("====================================================================\n");
       TestGrayTransformPerformance();
       TestGrayTransformPerformance1();
       
       printf("\nAll tests passed OK\n");


       return 0;
}




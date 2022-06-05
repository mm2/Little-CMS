//---------------------------------------------------------------------------------
//
//  Little Color Management System, multithreaded extensions
//  Copyright (c) 1998-2022 Marti Maria Saguer, all rights reserved
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

#include "threaded_internal.h"

#include <stdlib.h>
#include <memory.h>

// A fast way to convert from/to 16 <-> 8 bits
#define FROM_8_TO_16(rgb) (cmsUInt16Number) ((((cmsUInt16Number) (rgb)) << 8)|(rgb)) 
#define FROM_16_TO_8(rgb) (cmsUInt8Number) ((((rgb) * 65281 + 8388608) >> 24) & 0xFF)

// Some pixel representations
typedef struct { cmsUInt8Number  r, g, b;    }  Scanline_rgb8bits;
typedef struct { cmsUInt8Number  r, g, b, a; }  Scanline_rgba8bits;
typedef struct { cmsUInt8Number  c, m, y, k; }  Scanline_cmyk8bits;
typedef struct { cmsUInt16Number r, g, b;    }  Scanline_rgb16bits;
typedef struct { cmsUInt16Number r, g, b, a; }  Scanline_rgba16bits;
typedef struct { cmsUInt16Number c, m, y, k; }  Scanline_cmyk16bits;


// A flushed printf
static 
void trace(const char* frm, ...)
{
    va_list args;

    va_start(args, frm);
    vfprintf(stderr, frm, args);
    fflush(stderr);
    va_end(args);
}


// The callback function used by cmsSetLogErrorHandler()
static
void FatalErrorQuit(cmsContext ContextID, cmsUInt32Number ErrorCode, const char *Text)
{
       UNUSED_PARAMETER(ContextID);
       UNUSED_PARAMETER(ErrorCode);

       trace("** Fatal error: %s\n", Text);
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

      // unreachable va_end(args);
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


// --------------------------------------------------------------------------------------------------
// A C C U R A C Y   C H E C K S
// --------------------------------------------------------------------------------------------------



// Check change format feature
static
void CheckChangeFormat(void)
{
    cmsHPROFILE hsRGB, hLab;
    cmsHTRANSFORM xform;
    cmsUInt8Number rgb8[3]  = { 10, 120, 40 };
    cmsUInt16Number rgb16[3] = { 10* 257, 120*257, 40*257 };
    cmsUInt16Number lab16_1[3], lab16_2[3];

    trace("Checking change format feature...");

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

    trace("Ok\n");

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
       trace("                                  MPixel/sec.   MByte/sec.\n");
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

       trace("%-30s: ", Title); fflush(stdout);
       trace("%-12.2f %-12.2f", n, n * sz);

       if (prev > 0.0) {

              cmsFloat64Number imp = n / prev;
              if (imp > 1)
                   trace(" (x %-2.1f)",  imp);
       }

       trace("\n"); fflush(stdout);
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


       trace("%-30s: ", Title); fflush(stdout);
       trace("%-12.2f %-12.2f\n", n1, n2);
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

    trace("\n\n");
    trace("P E R F O R M A N C E   T E S T S   8 B I T S  (D E F A U L T)\n");
    trace("==============================================================\n\n");
    fflush(stdout);

    PerformanceHeader();
    t[0] = Performance("8 bits on CLUT profiles  ", SpeedTest8bitsRGB, noPlugin, "test5.icc", "test3.icc", sizeof(Scanline_rgb8bits), 0);
    t[1] = Performance("8 bits on Matrix-Shaper  ", SpeedTest8bitsRGB, noPlugin, "test5.icc", "test0.icc", sizeof(Scanline_rgb8bits), 0);
    t[2] = Performance("8 bits on same MatrixSh  ", SpeedTest8bitsRGB, noPlugin, "test0.icc", "test0.icc", sizeof(Scanline_rgb8bits), 0);
    t[3] = Performance("8 bits on curves         ", SpeedTest8bitsRGB, noPlugin, "*curves",   "*curves",   sizeof(Scanline_rgb8bits), 0);

    // Note that context 0 has the plug-in installed

    trace("\n\n");
    trace("P E R F O R M A N C E   T E S T S  8 B I T S  (P L U G I N)\n");
    trace("===========================================================\n\n");
    fflush(stdout);

    PerformanceHeader();
    Performance("8 bits on CLUT profiles  ", SpeedTest8bitsRGB, 0, "test5.icc", "test3.icc", sizeof(Scanline_rgb8bits), t[0]);
    Performance("8 bits on Matrix-Shaper  ", SpeedTest8bitsRGB, 0, "test5.icc", "test0.icc", sizeof(Scanline_rgb8bits), t[1]);
    Performance("8 bits on same MatrixSh  ", SpeedTest8bitsRGB, 0, "test0.icc", "test0.icc", sizeof(Scanline_rgb8bits), t[2]);
    Performance("8 bits on curves         ", SpeedTest8bitsRGB, 0, "*curves",   "*curves",   sizeof(Scanline_rgb8bits), t[3]);

    cmsDeleteContext(noPlugin);
}

static
void SpeedTest16(void)
{
    cmsContext noPlugin = cmsCreateContext(0, 0);

    cmsFloat64Number t[10];

    trace("\n\n");
    trace("P E R F O R M A N C E   T E S T S   1 6  B I T S  (D E F A U L T)\n");
    trace("=================================================================\n\n");
    
    PerformanceHeader();
    t[0] = Performance("16 bits on CLUT profiles         ", SpeedTest16bitsRGB,  noPlugin, "test5.icc", "test3.icc",  sizeof(Scanline_rgb16bits), 0);
    t[1] = Performance("16 bits on Matrix-Shaper profiles", SpeedTest16bitsRGB,  noPlugin, "test5.icc", "test0.icc",  sizeof(Scanline_rgb16bits), 0);
    t[2] = Performance("16 bits on same Matrix-Shaper    ", SpeedTest16bitsRGB,  noPlugin, "test0.icc", "test0.icc",  sizeof(Scanline_rgb16bits), 0);
    t[3] = Performance("16 bits on curves                ", SpeedTest16bitsRGB,  noPlugin, "*curves",   "*curves",    sizeof(Scanline_rgb16bits), 0);
    t[4] = Performance("16 bits on CMYK CLUT profiles    ", SpeedTest16bitsCMYK, noPlugin, "test1.icc", "test2.icc",  sizeof(Scanline_cmyk16bits), 0);
    
    trace("\n\n");
    trace("P E R F O R M A N C E   T E S T S   1 6  B I T S  (P L U G I N)\n");
    trace("===============================================================\n\n");

    PerformanceHeader();
    Performance("16 bits on CLUT profiles         ", SpeedTest16bitsRGB,  0, "test5.icc", "test3.icc", sizeof(Scanline_rgb16bits), t[0]);
    Performance("16 bits on Matrix-Shaper profiles", SpeedTest16bitsRGB,  0, "test5.icc", "test0.icc", sizeof(Scanline_rgb16bits), t[1]);
    Performance("16 bits on same Matrix-Shaper    ", SpeedTest16bitsRGB,  0, "test0.icc", "test0.icc", sizeof(Scanline_rgb16bits), t[2]);
    Performance("16 bits on curves                ", SpeedTest16bitsRGB,  0, "*curves",   "*curves",   sizeof(Scanline_rgb16bits), t[3]);
    Performance("16 bits on CMYK CLUT profiles    ", SpeedTest16bitsCMYK, 0, "test1.icc", "test2.icc", sizeof(Scanline_cmyk16bits), t[4]);
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

       trace("\n\n");
       trace("C O M P A R A T I V E cmsDoTransform() vs. cmsDoTransformLineStride()\n");
       trace("                              values given in MegaPixels per second.\n");
       trace("====================================================================\n");

       fflush(stdout);

       NoPlugin = cmsCreateContext(NULL, NULL);
       Plugin = cmsCreateContext(cmsThreadedExtensions(CMS_THREADED_GUESS_MAX_THREADS, 0), NULL);

       ComparativeCt(NoPlugin, Plugin, "CLUT profiles  ", SpeedTest8bitDoTransform, SpeedTest8bitLineStride, "test5.icc", "test3.icc");
       ComparativeCt(NoPlugin, Plugin, "CLUT 16 bits   ", SpeedTest16bitsRGB,       SpeedTest16bitsRGB, "test5.icc", "test3.icc");
       ComparativeCt(NoPlugin, Plugin, "Matrix-Shaper  ", SpeedTest8bitDoTransform, SpeedTest8bitLineStride, "test5.icc", "test0.icc");
       ComparativeCt(NoPlugin, Plugin, "same MatrixSh  ", SpeedTest8bitDoTransform, SpeedTest8bitLineStride, "test0.icc", "test0.icc");
       ComparativeCt(NoPlugin, Plugin, "curves         ", SpeedTest8bitDoTransform, SpeedTest8bitLineStride, NULL, NULL);

       cmsDeleteContext(Plugin);
       cmsDeleteContext(NoPlugin);
}



// The harness test
int main()
{
    trace("Multithreaded extensions testbed - 1.0\n");
    trace("Copyright (c) 1998-2022 Marti Maria Saguer, all rights reserved\n");

    trace("\nInstalling error logger ... ");
    cmsSetLogErrorHandler(FatalErrorQuit);
    trace("done.\n");

    trace("Installing plug-in ... ");
    cmsPlugin(cmsThreadedExtensions(CMS_THREADED_GUESS_MAX_THREADS, 0));
    trace("done.\n\n");

    // Change format
    CheckChangeFormat();

    // Check speed
    SpeedTest8();
    SpeedTest16();
    ComparativeLineStride8bits();

    trace("\nAll tests passed OK\n");
    return 0;
}




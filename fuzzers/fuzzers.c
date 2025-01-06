//---------------------------------------------------------------------------------
//
//  Little Color Management System
//  Copyright (c) 1998-2024 Marti Maria Saguer
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software
// is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//---------------------------------------------------------------------------------
//
#include <stdint.h>
#include "lcms2_plugin.h"

CMSAPI cmsPipeline* CMSEXPORT _cmsReadInputLUT(cmsHPROFILE hProfile, cmsUInt32Number Intent);
CMSAPI cmsPipeline* CMSEXPORT _cmsReadOutputLUT(cmsHPROFILE hProfile, cmsUInt32Number Intent);
CMSAPI cmsPipeline* CMSEXPORT _cmsReadDevicelinkLUT(cmsHPROFILE hProfile, cmsUInt32Number Intent);

// Read and decode all tags on a profile 
static
void ReadAllTags(cmsHPROFILE h)
{
    cmsInt32Number i, n;
    cmsTagSignature sig;

    n = cmsGetTagCount(h);
    for (i = 0; i < n; i++) {

        sig = cmsGetTagSignature(h, i);
        if (cmsReadTag(h, sig) == NULL) return;
    }
}


// Read all raw tags on a profile
static
void ReadAllRAWTags(cmsHPROFILE h)
{
    cmsInt32Number i, n;
    cmsTagSignature sig;
    cmsInt32Number len;

    n = cmsGetTagCount(h);
    for (i = 0; i < n; i++) {

        sig = cmsGetTagSignature(h, i);
        len = cmsReadRawTag(h, sig, NULL, 0);
    }
}

// read and decode information 
static
void FetchInfo(cmsHPROFILE h, cmsInfoType Info)
{
    wchar_t* text;
    cmsInt32Number len;
    cmsContext id = 0;

    len = cmsGetProfileInfo(h, Info, "en", "US", NULL, 0);
    if (len == 0) return;

    text = (wchar_t*)_cmsMalloc(id, len);
    if (text != NULL) {
        cmsGetProfileInfo(h, Info, "en", "US", text, len);
        _cmsFree(id, text);
    }
}

// Get all information 
static
void FetchAllInfos(cmsHPROFILE h)
{
    FetchInfo(h, cmsInfoDescription);
    FetchInfo(h, cmsInfoManufacturer);
    FetchInfo(h, cmsInfoModel);
    FetchInfo(h, cmsInfoCopyright);
}

// Read all LUTs, which some may be missing
static
void ReadAllLUTS(cmsHPROFILE h)
{
    cmsPipeline* a;
    cmsCIEXYZ Black;

    a = _cmsReadInputLUT(h, INTENT_PERCEPTUAL);
    if (a) cmsPipelineFree(a);

    a = _cmsReadInputLUT(h, INTENT_RELATIVE_COLORIMETRIC);
    if (a) cmsPipelineFree(a);

    a = _cmsReadInputLUT(h, INTENT_SATURATION);
    if (a) cmsPipelineFree(a);

    a = _cmsReadInputLUT(h, INTENT_ABSOLUTE_COLORIMETRIC);
    if (a) cmsPipelineFree(a);


    a = _cmsReadOutputLUT(h, INTENT_PERCEPTUAL);
    if (a) cmsPipelineFree(a);

    a = _cmsReadOutputLUT(h, INTENT_RELATIVE_COLORIMETRIC);
    if (a) cmsPipelineFree(a);

    a = _cmsReadOutputLUT(h, INTENT_SATURATION);
    if (a) cmsPipelineFree(a);

    a = _cmsReadOutputLUT(h, INTENT_ABSOLUTE_COLORIMETRIC);
    if (a) cmsPipelineFree(a);


    a = _cmsReadDevicelinkLUT(h, INTENT_PERCEPTUAL);
    if (a) cmsPipelineFree(a);

    a = _cmsReadDevicelinkLUT(h, INTENT_RELATIVE_COLORIMETRIC);
    if (a) cmsPipelineFree(a);

    a = _cmsReadDevicelinkLUT(h, INTENT_SATURATION);
    if (a) cmsPipelineFree(a);

    a = _cmsReadDevicelinkLUT(h, INTENT_ABSOLUTE_COLORIMETRIC);
    if (a) cmsPipelineFree(a);


    cmsDetectDestinationBlackPoint(&Black, h, INTENT_PERCEPTUAL, 0);
    cmsDetectDestinationBlackPoint(&Black, h, INTENT_RELATIVE_COLORIMETRIC, 0);
    cmsDetectDestinationBlackPoint(&Black, h, INTENT_SATURATION, 0);
    cmsDetectDestinationBlackPoint(&Black, h, INTENT_ABSOLUTE_COLORIMETRIC, 0);
    cmsDetectTAC(h);
}


// Create PostScript resources

static
void GenerateCSA(cmsHPROFILE hProfile)
{
    cmsUInt32Number n;
    char* Buffer;

    n = cmsGetPostScriptCSA(0, hProfile, 0, 0, NULL, 0);
    if (n == 0) return;

    Buffer = (char*)_cmsMalloc(0, n);
    cmsGetPostScriptCSA(0, hProfile, 0, 0, Buffer, n);
    _cmsFree(0, Buffer);
}


static
void GenerateCRD(cmsHPROFILE hProfile)
{
    cmsUInt32Number n;
    char* Buffer;
    cmsUInt32Number dwFlags = 0;

    n = cmsGetPostScriptCRD(0, hProfile, 0, dwFlags, NULL, 0);
    if (n == 0) return;

    Buffer = (char*)_cmsMalloc(0, n);
    cmsGetPostScriptCRD(0, hProfile, 0, dwFlags, Buffer, n);
    _cmsFree(0, Buffer);
}


// The fuzzer entry
int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t size)
{
    cmsHPROFILE hProfile = cmsOpenProfileFromMem(Data, (cmsUInt32Number)size);
    if (hProfile == NULL)
        return 0;
    
    ReadAllTags(hProfile);
    ReadAllRAWTags(hProfile);
    FetchAllInfos(hProfile);
    ReadAllLUTS(hProfile);
    GenerateCSA(hProfile);
    GenerateCRD(hProfile);

    cmsCloseProfile(hProfile);

    return 0;
}


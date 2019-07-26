/**
* Little Color Management System
* 
* Copyright (C) 1998-2020 Marti Maria Saguer
* 
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _lcms3_H
#define _lcms3_H

#include <cstdint>
#include <fstream>
#include <exception>

namespace lcms3
{
    class Exception
    {
    public:

        enum class ErrorCode : public std::exception
        {
            ERROR_UNDEFINED,
            ERROR_FILE,
            ERROR_RANGE,
            ERROR_INTERNAL,
            ERROR_NULL,
            ERROR_READ,
            ERROR_SEEK,
            ERROR_WRITE,
            ERROR_UNKNOWN_EXTENSION,
            ERROR_COLORSPACE_CHECK,
            ERROR_ALREADY_DEFINED,
            ERROR_BAD_SIGNATURE,
            ERROR_CORRUPTION_DETECTED,
            ERROR_NOT_SUITABLE
        };

        Exception(ErrorCode e);
        Exception(ErrorCode e, const char* frm, ...);
       
        virtual ~Exception();

        const char* what();
    };


    class Context
    {
    public:
                
        Context();
        Context(const Context& cpy);

        virtual ~Context();
     
        void unregisterPlugIns();

        void plug(const void* in);
  
        virtual errorLogger(Exception& e);

    };

    class IIOhandler
    {
    public:

        static IIOhandler* forFile(Context* ctx, const wchar_t* fileName, const char* access);
        static IIOhandler* forStream(Context* ctx, std::istream& strm);
        static IIOhandler* forStream(Context* ctx, std::ostream& strm);
        static IIOhandler* forStream(Context* ctx, std::fstream& strm);
        static IIOhandler* forMem(Context* ctx, const uint8_t* ptr, size_t size);
        static IIOhandler* forNULL(Context* ctx);

        virtual ~IIOhandler() = 0;

        virtual int32_t  read(uint8_t* buffer, uint32_t size) = 0;
        virtual void     write(const uint8_t* buffer, uint32_t size) = 0;
        virtual bool     seek(uint64_t offset) = 0;

        virtual uint64_t tell() = 0;
        virtual uint64_t size() = 0;

    protected:

        IIOhandler()
        {
        }
    };

      
enum class TagSignature : uint32_t
{
    sigAToB0Tag                          = 0x41324230,  // 'A2B0'  
    sigAToB1Tag                          = 0x41324231,  // 'A2B1' 
    sigAToB2Tag                          = 0x41324232,  // 'A2B2'  
    sigAToB3Tag                          = 0x41324233,  // 'A2B3' 
    sigAToM0Tag                          = 0x41324d30,  // 'A2M0' 
    sigBlueColorantTag                   = 0x6258595A,  // 'bXYZ' 
    sigBlueMatrixColumnTag               = 0x6258595A,  // 'bXYZ' 
    sigBlueTRCTag                        = 0x62545243,  // 'bTRC' 
    sigBrdfColorimetricParameter0Tag     = 0x62637030,  // 'bcp0' 
    sigBrdfColorimetricParameter1Tag     = 0x62637031,  // 'bcp1' 
    sigBrdfColorimetricParameter2Tag     = 0x62637032,  // 'bcp2' 
    sigBrdfColorimetricParameter3Tag     = 0x62637033,  // 'bcp3' 
    sigBrdfSpectralParameter0Tag         = 0x62737030,  // 'bsp0' 
    sigBrdfSpectralParameter1Tag         = 0x62737031,  // 'bsp1' 
    sigBrdfSpectralParameter2Tag         = 0x62737032,  // 'bsp2' 
    sigBrdfSpectralParameter3Tag         = 0x62737033,  // 'bsp3' 
    sigBRDFAToB0Tag                      = 0x62414230,  // 'bAB0' 
    sigBRDFAToB1Tag                      = 0x62414231,  // 'bAB1' 
    sigBRDFAToB2Tag                      = 0x62414232,  // 'bAB2' 
    sigBRDFAToB3Tag                      = 0x62414233,  // 'bAB3' 
    sigBRDFDToB0Tag                      = 0x62444230,  // 'bDB0' 
    sigBRDFDToB1Tag                      = 0x62444231,  // 'bDB1' 
    sigBRDFDToB2Tag                      = 0x62444232,  // 'bDB2' 
    sigBRDFDToB3Tag                      = 0x62444233,  // 'bDB3' 
    sigBRDFMToB0Tag                      = 0x624D4230,  // 'bMB0' 
    sigBRDFMToB1Tag                      = 0x624D4231,  // 'bMB1' 
    sigBRDFMToB2Tag                      = 0x624D4232,  // 'bMB2' 
    sigBRDFMToB3Tag                      = 0x624D4233,  // 'bMB3' 
    sigBRDFMToS0Tag                      = 0x624D5330,  // 'bMS0' 
    sigBRDFMToS1Tag                      = 0x624D5331,  // 'bMS1' 
    sigBRDFMToS2Tag                      = 0x624D5332,  // 'bMS2' 
    sigBRDFMToS3Tag                      = 0x624D5333,  // 'bMS3' 
    sigBToA0Tag                          = 0x42324130,  // 'B2A0' 
    sigBToA1Tag                          = 0x42324131,  // 'B2A1' 
    sigBToA2Tag                          = 0x42324132,  // 'B2A2' 
    sigBToA3Tag                          = 0x42324133,  // 'B2A3' 
    sigCalibrationDateTimeTag            = 0x63616C74,  // 'calt' 
    sigCharTargetTag                     = 0x74617267,  // 'targ'  
    sigChromaticAdaptationTag            = 0x63686164,  // 'chad' 
    sigChromaticityTag                   = 0x6368726D,  // 'chrm' 
    sigColorEncodingParamsTag            = 0x63657074,  // 'cept' 
    sigColorSpaceNameTag                 = 0x63736e6d,  // 'csnm' 
    sigColorantInfoTag                   = 0x636c696e,  // 'clin' 
    sigColorantInfoOutTag                = 0x636c696f,  // 'clio' 
    sigColorantOrderTag                  = 0x636C726F,  // 'clro' 
    sigColorantOrderOutTag               = 0x636c6f6f,  // 'cloo' 
    sigColorantTableTag                  = 0x636C7274,  // 'clrt' 
    sigColorantTableOutTag               = 0x636C6F74,  // 'clot' 
    sigColorimetricIntentImageStateTag   = 0x63696973,  // 'ciis' 
    sigCopyrightTag                      = 0x63707274,  // 'cprt' 
    sigCrdInfoTag                        = 0x63726469,  // 'crdi' deprecated 
    sigCustomToStandardPcsTag            = 0x63327370,  // 'c2sp' 
    sigCxFTag                            = 0x43784620,  // 'CxF ' 
    sigDataTag                           = 0x64617461,  // 'data' deprecated 
    sigDateTimeTag                       = 0x6474696D,  // 'dtim' deprecated 
    sigDeviceMediaWhitePointTag          = 0x646d7770,  // 'dmwp' 
    sigDeviceMfgDescTag                  = 0x646D6E64,  // 'dmnd' 
    sigDeviceModelDescTag                = 0x646D6464,  // 'dmdd' 
    sigDeviceSettingsTag                 = 0x64657673,  // 'devs' deprecated 
    sigDToB0Tag                          = 0x44324230,  // 'D2B0' 
    sigDToB1Tag                          = 0x44324231,  // 'D2B1' 
    sigDToB2Tag                          = 0x44324232,  // 'D2B2' 
    sigDToB3Tag                          = 0x44324233,  // 'D2B3' 
    sigBToD0Tag                          = 0x42324430,  // 'B2D0' 
    sigBToD1Tag                          = 0x42324431,  // 'B2D1' 
    sigBToD2Tag                          = 0x42324432,  // 'B2D2' 
    sigBToD3Tag                          = 0x42324433,  // 'B2D3' 
    sigGamutTag                          = 0x67616D74,  // 'gamt' 
    sigGamutBoundaryDescription0Tag      = 0x67626430,  // 'gbd0' 
    sigGamutBoundaryDescription1Tag      = 0x67626431,  // 'gbd1' 
    sigGamutBoundaryDescription2Tag      = 0x67626432,  // 'gbd2' 
    sigGamutBoundaryDescription3Tag      = 0x67626433,  // 'gbd3' 
    sigGrayTRCTag                        = 0x6b545243,  // 'kTRC' 
    sigGreenColorantTag                  = 0x6758595A,  // 'gXYZ' 
    sigGreenMatrixColumnTag              = 0x6758595A,  // 'gXYZ' 
    sigGreenTRCTag                       = 0x67545243,  // 'gTRC' 
    sigLuminanceTag                      = 0x6C756d69,  // 'lumi' 
    sigMaterialDefaultValuesTag          = 0x6D647620,  // 'mdv ' 
    sigMaterialTypeArrayTag              = 0x6d637461,  // 'mcta' 
    sigMToA0Tag                          = 0x4d324130,  // 'M2A0' 
    sigMToB0Tag                          = 0x4d324230,  // 'M2B0' 
    sigMToB1Tag                          = 0x4d324231,  // 'M2B1' 
    sigMToB2Tag                          = 0x4d324232,  // 'M2B2' 
    sigMToB3Tag                          = 0x4d324233,  // 'M2B3' 
    sigMToS0Tag                          = 0x4d325330,  // 'M2S0' 
    sigMToS1Tag                          = 0x4d325331,  // 'M2S1' 
    sigMToS2Tag                          = 0x4d325332,  // 'M2S2' 
    sigMToS3Tag                          = 0x4d325333,  // 'M2S3' 
    sigMeasurementTag                    = 0x6D656173,  // 'meas' 
    sigMediaBlackPointTag                = 0x626B7074,  // 'bkpt' 
    sigMediaWhitePointTag                = 0x77747074,  // 'wtpt' 
    sigMetaDataTag                       = 0x6D657461,  // 'meta' 
    sigNamedColorTag                     = 0x6e6d636C,  // 'nmcl' use for V5 
    sigNamedColor2Tag                    = 0x6E636C32,  // 'ncl2' 
    sigOutputResponseTag                 = 0x72657370,  // 'resp' 
    sigPerceptualRenderingIntentGamutTag = 0x72696730,  // 'rig0' 
    sigPreview0Tag                       = 0x70726530,  // 'pre0' 
    sigPreview1Tag                       = 0x70726531,  // 'pre1' 
    sigPreview2Tag                       = 0x70726532,  // 'pre2' 
    sigPrintConditionTag                 = 0x7074636e,  // 'ptcn' 
    sigProfileDescriptionTag             = 0x64657363,  // 'desc' 
    sigProfileSequenceDescTag            = 0x70736571,  // 'pseq' 
    sigProfileSequceIdTag                = 0x70736964,  // 'psid' 
    sigPs2CRD0Tag                        = 0x70736430,  // 'psd0' deprecated 
    sigPs2CRD1Tag                        = 0x70736431,  // 'psd1' deprecated 
    sigPs2CRD2Tag                        = 0x70736432,  // 'psd2' deprecated 
    sigPs2CRD3Tag                        = 0x70736433,  // 'psd3' deprecated 
    sigPs2CSATag                         = 0x70733273,  // 'ps2s' deprecated 
    sigPs2RenderingIntentTag             = 0x70733269,  // 'ps2i' deprecated 
    sigRedColorantTag                    = 0x7258595A,  // 'rXYZ' 
    sigRedMatrixColumnTag                = 0x7258595A,  // 'rXYZ' 
    sigRedTRCTag                         = 0x72545243,  // 'rTRC' 
    sigReferenceNameTag                  = 0x72666e6d,  // 'rfnm' 
    sigSaturationRenderingIntentGamutTag = 0x72696732,  // 'rig2' 
    sigScreeningDescTag                  = 0x73637264,  // 'scrd' deprecated 
    sigScreeningTag                      = 0x7363726E,  // 'scrn' deprecated 
    sigSpectralDataInfoTag               = 0x7364696e,  // 'sdin' 
    sigSpectralMediaWhitePointTag        = 0x736d7770,  // 'smwp' 
    sigSpectralViewingConditionsTag      = 0x7376636e,  // 'svcn' 
    sigStandardToCustomPcsTag            = 0x73326370,  // 's2cp' 
    sigTechnologyTag                     = 0x74656368,  // 'tech' 
    sigUcrBgTag                          = 0x62666420,  // 'bfd ' deprecated 
    sigViewingCondDescTag                = 0x76756564,  // 'vued' 
    sigViewingConditionsTag              = 0x76696577,  // 'view' 

    sigVcgtTag                           = 0x76636774,  // 'vcgt'
    sigMetaTag                           = 0x6D657461,  // 'meta'
    sigArgyllArtsTag                     = 0x61727473   // 'arts'
    
};

enum class TechnologySignature : uint32_t
{
    sigDigitalCamera                  = 0x6463616D,  // 'dcam' 
    sigFilmScanner                    = 0x6673636E,  // 'fscn' 
    sigReflectiveScanner              = 0x7273636E,  // 'rscn' 
    sigInkJetPrinter                  = 0x696A6574,  // 'ijet' 
    sigThermalWaxPrinter              = 0x74776178,  // 'twax' 
    sigElectrophotographicPrinter     = 0x6570686F,  // 'epho' 
    sigElectrostaticPrinter           = 0x65737461,  // 'esta' 
    sigDyeSublimationPrinter          = 0x64737562,  // 'dsub' 
    sigPhotographicPaperPrinter       = 0x7270686F,  // 'rpho' 
    sigFilmWriter                     = 0x6670726E,  // 'fprn' 
    sigVideoMonitor                   = 0x7669646D,  // 'vidm' 
    sigVideoCamera                    = 0x76696463,  // 'vidc' 
    sigProjectionTelevision           = 0x706A7476,  // 'pjtv' 
    sigCRTDisplay                     = 0x43525420,  // 'CRT ' 
    sigPMDisplay                      = 0x504D4420,  // 'PMD ' 
    sigAMDisplay                      = 0x414D4420,  // 'AMD ' 
    sigPhotoCD                        = 0x4B504344,  // 'KPCD' 
    sigPhotoImageSetter               = 0x696D6773,  // 'imgs' 
    sigGravure                        = 0x67726176,  // 'grav' 
    sigOffsetLithography              = 0x6F666673,  // 'offs' 
    sigSilkscreen                     = 0x73696C6B,  // 'silk' 
    sigFlexography                    = 0x666C6578,  // 'flex' 
    sigMotionPictureFilmScanner       = 0x6D706673,  // 'mpfs' 
    sigMotionPictureFilmRecorder      = 0x6D706672,  // 'mpfr' 
    sigDigitalMotionPictureCamera     = 0x646D7063,  // 'dmpc' 
    sigDigitalCinemaProjector         = 0x64636A70   // 'dcpj'     
};

enum class TagTypeSignature : uint32_t
{
    sigChromaticityType               = 0x6368726D,  // 'chrm' 
    sigColorantOrderType              = 0x636C726F,  // 'clro' 
    sigColorantTableType              = 0x636C7274,  // 'clrt' 
    sigCrdInfoType                    = 0x63726469,  // 'crdi' deprecated 
    sigCurveType                      = 0x63757276,  // 'curv' 
    sigDataType                       = 0x64617461,  // 'data' 
    sigDictType                       = 0x64696374,  // 'dict' 
    sigDateTimeType                   = 0x6474696D,  // 'dtim' 
    sigDeviceSettingsType             = 0x64657673,  // 'devs' deprecated 
    sigFloat16ArrayType               = 0x666c3136,  // 'fl16' 
    sigFloat32ArrayType               = 0x666c3332,  // 'fl32' 
    sigFloat64ArrayType               = 0x666c3634,  // 'fl64' 
    sigGamutBoundaryDescType	      = 0x67626420,  // 'gbd ' 
    sigLut16Type                      = 0x6d667432,  // 'mft2' 
    sigLut8Type                       = 0x6d667431,  // 'mft1' 
    sigLutAtoBType                    = 0x6d414220,  // 'mAB ' 
    sigLutBtoAType                    = 0x6d424120,  // 'mBA ' 
    sigMeasurementType                = 0x6D656173,  // 'meas' 
    sigMultiLocalizedUnicodeType      = 0x6D6C7563,  // 'mluc' 
    sigMultiProcessElementType        = 0x6D706574,  // 'mpet' 
    sigNamedColor2Type                = 0x6E636C32,  // 'ncl2' 
    sigParametricCurveType            = 0x70617261,  // 'para' 
    sigProfileSequenceDescType        = 0x70736571,  // 'pseq' 
    sigProfileSequceIdType            = 0x70736964,  // 'psid' 
    sigResponseCurveSet16Type         = 0x72637332,  // 'rcs2' 
    sigS15Fixed16ArrayType            = 0x73663332,  // 'sf32' 
    sigScreeningType                  = 0x7363726E,  // 'scrn' deprecated 
    sigSegmentedCurveType             = 0x63757266,  // 'curf' 
    sigSignatureType                  = 0x73696720,  // 'sig ' 
    sigSparseMatrixArrayType          = 0x736D6174,  // 'smat' 
    sigSpectralViewingConditionsType  = 0x7376636e,  // 'svcn' 
    sigSpectralDataInfoType           = 0x7364696e,  // 'sdin' 
    sigTagArrayType                   = 0x74617279,  // 'tary' 
    sigTagStructType                  = 0x74737472,  // 'tstr' 
    sigTextType                       = 0x74657874,  // 'text' 
    sigTextDescriptionType            = 0x64657363,  // 'desc' deprecated 
    sigU16Fixed16ArrayType            = 0x75663332,  // 'uf32' 
    sigUcrBgType                      = 0x62666420,  // 'bfd ' deprecated 
    sigUInt16ArrayType                = 0x75693136,  // 'ui16' 
    sigUInt32ArrayType                = 0x75693332,  // 'ui32' 
    sigUInt64ArrayType                = 0x75693634,  // 'ui64' 
    sigUInt8ArrayType                 = 0x75693038,  // 'ui08' 
    sigViewingConditionsType          = 0x76696577,  // 'view' 
    sigUtf8TextType                   = 0x75746638,  // 'utf8' 
    sigUtf16TextType                  = 0x75743136,  // 'ut16' 
    sigXYZType                        = 0x58595A20,  // 'XYZ ' 
    sigXYZArrayType                   = 0x58595A20,  // 'XYZ ' 
    sigZipUtf8TextType                = 0x7a757438,  // 'zut8' 
    sigZipXmlType                     = 0x5a584d4c   // 'ZXML'     
};


enum class TagStructSignature : uint32_t
{
    sigBRDFStruct                     = 0x62726466,  // 'brdf' 
    sigColorantInfoStruct             = 0x63696e66,  // 'cinf' 
    sigColorEncodingParamsSruct       = 0x63657074,  // 'cept' 
    sigMeasurementInfoStruct          = 0x6d656173,  // 'meas' 
    sigNamedColorStruct               = 0x6e6d636c,  // 'nmcl' 
    sigProfileInfoStruct              = 0x70696e66,  // 'pinf' 
    sigTintZeroStruct                 = 0x746e7430   // 'tnt0'        
};


enum class ArrayTypeSignature : uint32_t
{
    sigNamedColorArray             = 0x6e6d636c,  // 'nmcl' 
    sigColorantInfoArray           = 0x63696e66,  // 'cinf' 
    sigUtf8TextTypeArray           = 0x75746638   // 'utf8'     
};

enum class CMMenvVar : uint32_t
{
    sigTrueVar                      = 0x74727565,  // 'true' 
    sigNotDefVar                    = 0x6e646566   // 'ndef'         
};


enum class ElemTypeSignature : uint32_t
{
    sigCurveSetElemType             = 0x63767374,  // 'cvst' 
    sigMatrixElemType               = 0x6D617466,  // 'matf' 
    sigCLutElemType                 = 0x636C7574,  // 'clut' 
    sigBAcsElemType                 = 0x62414353,  // 'bACS' 
    sigEAcsElemType                 = 0x65414353,  // 'eACS' 
    sigCalculatorElemType           = 0x63616c63,  // 'calc' 
    sigExtCLutElemType              = 0x78636c74,  // 'xclt' 
    sigXYZToJabElemType             = 0x58746f4a,  // 'XtoJ' 
    sigJabToXYZElemType             = 0x4a746f58,  // 'JtoX' 
    sigSparseMatrixElemType         = 0x736d6574,  // 'smet' 
    sigTintArrayElemType            = 0x74696e74,  // 'tint' 
    sigEmissionMatrixElemType       = 0x656d7478,  // 'emtx' 
    sigInvEmissionMatrixElemType    = 0x69656d78,  // 'iemx' 
    sigEmissionCLUTElemType         = 0x65636c74,  // 'eclt' 
    sigReflectanceCLUTElemType      = 0x72636c74,  // 'rclt' 
    sigEmissionObserverElemType     = 0x656f6273,  // 'eobs' 
    sigReflectanceObserverElemType  = 0x726f6273   // 'robs'     
};


class enum BrdfMemberSignature : uint32_t
{
    sigBrdfTypeMbr                 = 0x74797065,  // 'type'
    sigBrdfFunctionMbr             = 0x66756e63,  // 'func'
    sigBrdfParamsPerChannelMbr     = 0x6e756d70,  // 'nump'
    sigBrdfTransformMbr            = 0x7866726d,  // 'xfrm'
    sigBrdfLightTransformMbr       = 0x6c747866,  // 'ltxf'
    sigBrdfOutputTransformMbr      = 0x6f757478   // 'outx'
};

enum class BrdfTypeSignature : uint32_t
{

    sigBrdfTypeBlinnPhongMono         = 0x42506830,    // 'BPh0' *
    sigBrdfTypeBlinnPhongColor        = 0x42506831,    // 'BPh1' *
    sigBrdfTypeCookTorranceMono1Lobe  = 0x43543130,    // 'CT10' *
    sigBrdfTypeCookTorranceMono2Lobe  = 0x43543230,    // 'CT20' *
    sigBrdfTypeCookTorranceMono3Lobe  = 0x43543330,    // 'CT30' *
    sigBrdfTypeCookTorranceColor1Lobe = 0x43543131,    // 'CT11' *
    sigBrdfTypeCookTorranceColor2Lobe = 0x43543231,    // 'CT21' *
    sigBrdfTypeCookTorranceColor3Lobe = 0x43543331,    // 'CT31' *
    sigBrdfTypeWardMono               = 0x57617230,    // 'War0' *
    sigBrdfTypeWardColor              = 0x57617230,    // 'War1' *
    sigBrdfTypeLafortuneMono1Lobe     = 0x4c613130,    // 'La10' *
    sigBrdfTypeLafortuneMono2Lobe     = 0x4c613230,    // 'La20' *
    sigBrdfTypeLafortuneMono3Lobe     = 0x4c613330,    // 'La30' *
    sigBrdfTypeLafortuneColor1Lobe    = 0x4c613131,    // 'La11' *
    sigBrdfTypeLafortuneColor2Lobe    = 0x4c613231,    // 'La21' *
    sigBrdfTypeLafortuneColor3Lobe    = 0x4c613331     // 'La31' *

};


enum class ColorantInfoStructure : uint32_t 
{
    sigCinfNameMbr                = 0x6e616d65, /* 'name' */
    sigCinfLocalizedNameMbr       = 0x6c636e6d, /* 'lcnm' */
    sigCinfPcsDataMbr             = 0x70637320, /* 'pcs ' */
    sigCinfSpectralDataMbr        = 0x73706563  /* 'spec' */
};

enum class ColorEncodingParamsMemberSignature : uint32_t
{
    sigCeptBluePrimaryXYZMbr                = 0x6258595a,  // 'bXYZ'
    sigCeptGreenPrimaryXYZMbr	              = 0x6758595a,  // 'gXYZ'
    sigCeptRedPrimaryXYZMbr                 = 0x7258595a,  // 'rXYZ'
    sigCeptTransferFunctionMbr              = 0x66756e63,  // 'func' 
    sigCeptInverseTransferFunctionMbr       = 0x69666e63,  // 'ifnc' 
    sigCeptLumaChromaMatrixMbr              = 0x6c6d6174,  // 'lmat'
    sigCeptWhitePointLuminanceMbr           = 0x776c756d,  // 'wlum'
    sigCeptWhitePointChromaticityMbr        = 0x7758595a,  // 'wXYZ'
    sigCeptEncodingRangeMbr                 = 0x65526e67,  // 'eRng'
    sigCeptBitDepthMbr                      = 0x62697473,  // 'bits'
    sigCeptImageStateMbr                    = 0x696d7374,  // 'imst'
    sigCeptImageBackgroundMbr               = 0x69626b67,  // 'ibkg'
    sigCeptViewingSurroundMbr               = 0x73726e64,  // 'srnd'
    sigCeptAmbientIlluminanceMbr            = 0x61696c6d,  // 'ailm'
    sigCeptAmbientWhitePointLuminanceMbr    = 0x61776c6d,  // 'awlm'
    sigCeptAmbientWhitePointChromaticityMbr = 0x61777063,  // 'awpc'
    sigCeptViewingFlareMbr                  = 0x666c6172,  // 'flar'
    sigCeptValidRelativeLuminanceRangeMbr   = 0x6c726e67,  // 'lrng'
    sigCeptMediumWhitePointLuminanceMbr     = 0x6d77706c,  // 'mwpl'
    sigCeptMediumWhitePointChromaticityMbr  = 0x6d777063,  // 'mwpc'
    sigCeptMediumBlackPointLuminanceMbr     = 0x6d62706c,  // 'mbpl'
    sigCeptMediumBlackPointChromaticityMbr  = 0x6d627063   // 'mbpc'

};


enum class MeasurementInfoMemberSignature : uint32_t 
{
    sigMeasBackingMbr                      = 0x6d62616b, // 'mbak'
    sigMeasFlareMbr                        = 0x6d666c72, // 'mflr'
    sigMeasGeometryMbr                     = 0x6d67656f, // 'mgeo'
    sigMeasIlluminantMbr                   = 0x6d696c6c, // 'mill'
    sigMeasIlluminantRangeMbr              = 0x6d697772, // 'miwr'
    sigMeasModeMbr                         = 0x6d6d6f64  // 'mmod'
    
};


enum class NamedColorMemberSignature : uint32_t 
{

    sigNmclBrdfColorimetricMbr       = 0x62636f6c,  // 'bcol' 
    sigNmclBrdfColorimetricParamsMbr = 0x62636f6c,  // 'bcpr' 
    sigNmclBrdfSpectralMbr           = 0x62737063,  // 'bspc' 
    sigNmclBrdfSpectralParamsMbr     = 0x62737072,  // 'bspr' 
    sigNmclDeviceDataMbr             = 0x64657620,  // 'dev ' 
    sigNmclLocalizedNameMbr          = 0x6c636e6d,  // 'lcnm' 
    sigNmclNameMbr                   = 0x6e616d65,  // 'name' 
    sigNmclNormalMapMbr              = 0x6e6d6170,  // 'nmap' 
    sigNmclPcsDataMbr                = 0x70637320,  // 'pcs ' 
    sigNmclSpectralDataMbr           = 0x73706563,  // 'spec' 
    sigNmclSpectralOverBlackMbr      = 0x73706362,  // 'spcb' 
    sigNmclSpectralOverGrayMbr       = 0x73706367,  // 'spcg' 
    sigNmclTintMbr                   = 0x74696e74,  // 'tint' 

};

enum class ProfileInfoMemberSignature : uint32_t
{
    sigPinfAttributesMbr       = 0x61747472, // 'attr' 
    sigPinfProfileDescMbr      = 0x70647363, // 'pdsc' 
    sigPinfProfileIDMbr        = 0x70696420, // 'pid ' 
    sigPinfManufacturerDescMbr = 0x646d6e64, // 'dmnd' 
    sigPinfManufacturerSigMbr  = 0x646d6e73, // 'dmns' 
    sigPinfModelDescMbr        = 0x646d6464, // 'dmdd' 
    sigPinfModelSigMbr         = 0x6d6f6420, // 'mod ' 
    sigPinfRenderTransformMbr  = 0x7274726e, // 'rtrn' 
    sigPinfTechnologyMbr       = 0x74656368, // 'tech' 
 
};


enum class TintZeroMemberSignature : uint32_t
{
    sigTnt0DeviceDataMbr        = 0x64657620,  // 'dev ' 
    sigTnt0PcsDataMbr           = 0x70637320,  // 'pcs ' 
    sigTnt0SpectralDataMbr      = 0x73706563,  // 'spec' 
    sigTnt0SpectralOverBlackMbr = 0x73706362,  // 'spcb' 
    sigTnt0SpectralOverGrayMbr  = 0x73706367,  // 'spcg' 
  
};



enum class ColorSpaceSignature : uint32_t
{    
    sigXYZData                        = 0x58595A20,  // 'XYZ ' 
    sigLabData                        = 0x4C616220,  // 'Lab ' 
    sigLuvData                        = 0x4C757620,  // 'Luv ' 
    sigYCbCrData                      = 0x59436272,  // 'YCbr' 
    sigYxyData                        = 0x59787920,  // 'Yxy ' 
    sigRgbData                        = 0x52474220,  // 'RGB ' 
    sigGrayData                       = 0x47524159,  // 'GRAY' 
    sigHsvData                        = 0x48535620,  // 'HSV ' 
    sigHlsData                        = 0x484C5320,  // 'HLS ' 
    sigCmykData                       = 0x434D594B,  // 'CMYK' 
    sigCmyData                        = 0x434D5920,  // 'CMY '     
    sig1colorData                     = 0x31434C52,  // '1CLR' 
    sig2colorData                     = 0x32434C52,  // '2CLR' 
    sig3colorData                     = 0x33434C52,  // '3CLR' 
    sig4colorData                     = 0x34434C52,  // '4CLR' 
    sig5colorData                     = 0x35434C52,  // '5CLR' 
    sig6colorData                     = 0x36434C52,  // '6CLR' 
    sig7colorData                     = 0x37434C52,  // '7CLR' 
    sig8colorData                     = 0x38434C52,  // '8CLR' 
    sig9colorData                     = 0x39434C52,  // '9CLR' 
    sig10colorData                    = 0x41434C52,  // 'ACLR' 
    sig11colorData                    = 0x42434C52,  // 'BCLR' 
    sig12colorData                    = 0x43434C52,  // 'CCLR' 
    sig13colorData                    = 0x44434C52,  // 'DCLR' 
    sig14colorData                    = 0x45434C52,  // 'ECLR' 
    sig15colorData                    = 0x46434C52,  // 'FCLR' 
    sigNamedData                      = 0x6e6d636c,  // 'nmcl' 
    sigMCH1Data                       = 0x31434C52,  // '1CLR' 
    sigMCH2Data                       = 0x32434C52,  // '2CLR'         
    sigMCH3Data                       = 0x33434C52,  // '3CLR'         
    sigMCH4Data                       = 0x34434C52,  // '4CLR'         
    sigMCH5Data                       = 0x35434C52,  // '5CLR' 
    sigMCH6Data                       = 0x36434C52,  // '6CLR' 
    sigMCH7Data                       = 0x37434C52,  // '7CLR' 
    sigMCH8Data                       = 0x38434C52,  // '8CLR' 
    sigMCH9Data                       = 0x39434C52,  // '9CLR' 
    sigMCHAData                       = 0x41434C52,  // 'ACLR' 
    sigMCHBData                       = 0x42434C52,  // 'BCLR' 
    sigMCHCData                       = 0x43434C52,  // 'CCLR' 
    sigMCHDData                       = 0x44434C52,  // 'DCLR' 
    sigMCHEData                       = 0x45434C52,  // 'ECLR' 
    sigMCHFData                       = 0x46434C52,  // 'FCLR' 
    sigNChannelData                   = 0x6e630000,  // 'nc??' 
    sigSrcMCSChannelData              = 0x6d630000   // 'mc??'     
};



enum class ProfileClassSignature : uint32_t
{
    sigInputClass                     = 0x73636E72,  // 'scnr' 
    sigDisplayClass                   = 0x6D6E7472,  // 'mntr' 
    sigOutputClass                    = 0x70727472,  // 'prtr' 
    sigLinkClass                      = 0x6C696E6B,  // 'link' 
    sigAbstractClass                  = 0x61627374,  // 'abst' 
    sigColorSpaceClass                = 0x73706163,  // 'spac' 
    sigNamedColorClass                = 0x6e6d636c,  // 'nmcl' 
    sigColorEncodingClass             = 0x63656e63,  // 'cenc' 
    sigMaterialIdentificationClass    = 0x6D696420,  // 'mid ' 
    sigMaterialLinkClass              = 0x6d6c6e6b,  // 'mlnk' 
    sigMaterialVisualizationClass     = 0x6d766973   // 'mvis' 
};


enum class PlatformSignature : uint32_t
{   
    sigMacintosh                      = 0x4150504C,  // 'APPL' 
    sigMicrosoft                      = 0x4D534654,  // 'MSFT' 
    sigSolaris                        = 0x53554E57,  // 'SUNW' 
    sigSGI                            = 0x53474920,  // 'SGI ' 
    sigTaligent                       = 0x54474E54   // 'TGNT' 
};

enum class CmmSignature : uint32_t
{                                                    
    sigAdobe                          = 0x41444245,  // 'ADBE' 
    sigAgfa                           = 0x41434D53,  // 'ACMS' 
    sigApple                          = 0x6170706C,  // 'appl' 
    sigColorGear                      = 0x43434D53,  // 'CCMS' 
    sigColorGearLite                  = 0x5543434D,  // 'UCCM' 
    sigColorGearC                     = 0x55434D53,  // 'UCMS' 
    sigEFI                            = 0x45464920,  // 'EFI ' 
    sigExactScan                      = 0x45584143,  // 'EXAC' 
    sigFujiFilm                       = 0x46462020,  // 'FF  ' 
    sigHarlequinRIP                   = 0x48434d4d,  // 'HCMM' 
    sigArgyllCMS                      = 0x6172676C,  // 'argl' 
    sigLogoSync                       = 0x44676f53,  // 'LgoS' 
    sigHeidelberg                     = 0x48444d20,  // 'HDM ' 
    sigLittleCMS                      = 0x6C636D73,  // 'lcms' 
    sigKodak                          = 0x4b434d53,  // 'KCMS' 
    sigKonicaMinolta                  = 0x4d434d44,  // 'MCML' 
    sigWindowsCMS                     = 0x57435320,  // 'WCS ' 
    sigMutoh                          = 0x5349474E,  // 'SIGN' 
    sigOnyxGraphics                   = 0x4f4e5958,  // 'ONYX' 
    sigRefIccMAX                      = 0x52494343,  // 'RIMX' 
    sigRolfGierling                   = 0x52474d53,  // 'RGMS' 
    sigSampleICC                      = 0x53494343,  // 'SICC' 
    sigToshiba                        = 0x54434D4D,  // 'TCMM' 
    sigTheImagingFactory              = 0x33324254,  // '32BT' 
    sigVivo                           = 0x7669766F,  // 'VIVO' 
    sigWareToGo                       = 0x57544720,  // 'WTG ' 
    sigZoran                          = 0x7a633030   // 'zc00'     
};

enum class ReferenceMediumGamutSignature : uint32_t
{
    sigPerceptualReferenceMediumGamut = 0x70726d67,  // 'prmg' 
};

enum class ColorimetricIntentImageStateSignature : uint32_t
{
    sigSceneColorimetryEstimates             = 0x73636F65,  // 'scoe' 
    sigSceneAppearanceEstimates              = 0x73617065,  // 'sape' 
    sigFocalPlaneColorimetryEstimates        = 0x66706365,  // 'fpce' 
    sigReflectionHardcopyOriginalColorimetry = 0x72686F63,  // 'rhoc' 
    sigReflectionPrintOutputColorimetry      = 0x72706F63,  // 'rpoc' 
};


enum class CurveSegSignature : uint32_t
{
    sigFormulaCurveSeg = 0x70617266,  // 'parf' 
    sigSampledCurveSeg = 0x73616D66,  // 'samf' 
};


enum class CurveElemSignature : uint32_t
{
    sigSegmentedCurve               = 0x63757266,  // 'curf' 
    sigSingleSampledCurve           = 0x736e6766,  // 'sngf' 
};

/**
*  This class packs a description of how buffers containing pixels are organized. Each object fits
*  in a single uint32 since no vtables are being used. Examples of bit-coded identifiers are
*  PixelType::TYPE_RGB_8 or PixelType::TYPE_GRAY_16.
*
*      28..31 TTTT : Color model
*      25..27 DDD  : Channels layout: 0 none
*                               1 byte per channel, 
*                               2 bytes per channel (int), 
*                               2 bytes per channel (half float), 
*                               4 bytes per channel (float), 
*                               8 bytes per channel (double)
*      24 P        : Planar
*      23 E        : Channels Endianess 
*      22 X        : Bytes endianess
*      21 F        : Extra channels position (0=begin, 1=end)  
*
*      0..16       : Extra channels or spectral/bispectral
*      
*/

class PixelFormat
{
public:

    enum class ColorModel
    {
        Unknown = 0,
        Monochrome = 1,
        Gray = 2,
        RGB = 3,
        CMY = 4,
        CMYK = 5,
        YCbCr = 6,
        YUV = 7,  
        XYZ = 8,
        Lab = 9, 
        LabV2 = 10,    
        HSV = 11,
        HLS = 12,
        Yxy = 13,
        Spectral = 14,
        BiSpectral = 15
    };

    inline constexpr uint32_t ColorModelShift(ColorModel m) const noexcept
    { 
        return (static_cast<uint32_t>(m) & 0xf) << 27; 
    }
        
    inline constexpr ColorModel ColorModelOf(int32_t t) const noexcept
    { 
        return static_cast<ColorModel>((t >> 27) & 0xf); 
    }

    enum class ChannelLayout : uint32_t
    {
        None     = 0,   // 0 none
        Int8     = 1,   // 1 byte per channel,
        Int16    = 2,   // 2 bytes per channel(int),
        Half16   = 3,   // 2 bytes per channel(half float),
        Float32  = 4,   // 4 bytes per channel(float),
        Double64 = 5    // 8 bytes per channel(double)
    };

    inline constexpr uint32_t ChannelLayoutShift(ChannelLayout m) const noexcept
    {
        return (static_cast<uint32_t>(m) & 0x7) << 20;
    }

    inline constexpr ChannelLayout ChannelLayoutOf(int32_t t) const noexcept
    {
        return static_cast<ChannelLayout>((t >> 20) & 0x7);
    }

    inline constexpr uint32_t PlanarShift(bool p) const noexcept
    {
        return p ? (1 << 24) : 0;
    }

    inline constexpr bool PlanarOf(int32_t t) const noexcept
    {
        return (t & (1 << 24)) ? true : false;
    }

    inline constexpr uint32_t ChannelsEndianessShift(bool p) const noexcept
    {
        return p ? (1 << 23) : 0;
    }

    inline constexpr bool ChannelsEndianessOf(int32_t t) const noexcept
    {
        return (t & (1 << 23)) ? true : false;
    }


    inline constexpr uint32_t BytesEndianessShift(bool p) const noexcept
    {
        return p ? (1 << 22) : 0;
    }

    inline constexpr bool BytesEndianessOf(int32_t t) const noexcept
    {
        return (t & (1 << 22)) ? true : false;
    }

    inline constexpr uint32_t ExtraChannelsOnEndShift(bool p) const noexcept
    {
        return p ? (1 << 21) : 0;
    }

    inline constexpr bool ExtraChannelsOnEndOf(int32_t t) const noexcept
    {
        return (t & (1 << 21)) ? true : false;
    }


    inline constexpr uint32_t ExtraChannelsShift(uint32_t e) const noexcept
    {
        return p & 0xffff;
    }

    inline constexpr uint32_t ExtraChannelsOf(int32_t t) const noexcept
    {
        return p & 0xffff;
    }

    
    PixelFormat(uint32_t t) noexcept               
    {
        format_ = t;
    }

    PixelFormat(const PixelFormat& Copy) noexcept 
    {
        format_ = Copy.format_;
    }

    ~PixelFormat() noexcept
    {
    }

    PixelFormat& operator =(uint32_t t) noexcept
    {
        format_ = t;
        return *this;
    }

    void setBits(uint32_t t) noexcept
    {
        format_ = t;
    }

    uint32_t getPixelFormat() const noexcept
    {
        return format_;
    }



    ColorModel colorModel() const noexcept
    {
        return ColorModelOf(format_);
    }

    ChannelLayout channelLayout() const noexcept
    {
        return ChannelLayoutOf(format_);
    }

    bool planar() const noexcept
    {
        return PlanarOf(format_);
    }
       
    bool swapped() const noexcept
    {
        return ChannelsEndianessOf(format_);
    }
         
    bool swapNibblesOn16bits() const noexcept
    {
        return BytesEndianessOf(format_);
    }

    bool swapFirst() const noexcept
    {
        return T_SWAPFIRST(format_) ? true : false;
    }

    uint32_t channels() const noexcept
    {
        return ChannelsOf(format_);
    }

    uint32_t extraChannels() const noexcept
    {
        return T_EXTRA(format_);
    }

    uint32_t totalChannels() const noexcept
    {
        return (channels() + extraChannels());
    }

    /**
    * How many bytes take one pixel
    */
    uint32_t bytesPerPixel() const noexcept
    {
        if (planar())
            return bytesPerChannel();
        else
            return bytesPerChannel() * totalChannels();
    }

    /**
    * How many planes are being used
    */
    uint32_t planes() const noexcept
    {
        if (planar())
            return totalChannels();
        else
            return 1;
    }

    /**
    * Here you get a exhaustive list of different encodings
    */
    enum
    {
        TYPE_GRAY_8 = (COLORSPACE_SH(PT_GRAY) | CHANNELS_SH(1) | BYTES_SH(1)),
        TYPE_GRAY_8_REV = (COLORSPACE_SH(PT_GRAY) | CHANNELS_SH(1) | BYTES_SH(1) | FLAVOR_SH(1)),
        TYPE_GRAY_16 = (COLORSPACE_SH(PT_GRAY) | CHANNELS_SH(1) | BYTES_SH(2)),
        TYPE_GRAY_16_REV = (COLORSPACE_SH(PT_GRAY) | CHANNELS_SH(1) | BYTES_SH(2) | FLAVOR_SH(1)),
        TYPE_GRAY_16_SE = (COLORSPACE_SH(PT_GRAY) | CHANNELS_SH(1) | BYTES_SH(2) | ENDIAN16_SH(1)),
        TYPE_GRAYA_8 = (COLORSPACE_SH(PT_GRAY) | EXTRA_SH(1) | CHANNELS_SH(1) | BYTES_SH(1)),
        TYPE_GRAYA_16 = (COLORSPACE_SH(PT_GRAY) | EXTRA_SH(1) | CHANNELS_SH(1) | BYTES_SH(2)),
        TYPE_GRAYA_16_SE = (COLORSPACE_SH(PT_GRAY) | EXTRA_SH(1) | CHANNELS_SH(1) | BYTES_SH(2) | ENDIAN16_SH(1)),
        TYPE_GRAYA_8_PLANAR = (COLORSPACE_SH(PT_GRAY) | EXTRA_SH(1) | CHANNELS_SH(1) | BYTES_SH(1) | PLANAR_SH(1)),
        TYPE_GRAYA_16_PLANAR = (COLORSPACE_SH(PT_GRAY) | EXTRA_SH(1) | CHANNELS_SH(1) | BYTES_SH(2) | PLANAR_SH(1)),

        TYPE_RGB_8 = (COLORSPACE_SH(PT_RGB) | CHANNELS_SH(3) | BYTES_SH(1)),
        TYPE_RGB_8_PLANAR = (COLORSPACE_SH(PT_RGB) | CHANNELS_SH(3) | BYTES_SH(1) | PLANAR_SH(1)),
        TYPE_BGR_8 = (COLORSPACE_SH(PT_RGB) | CHANNELS_SH(3) | BYTES_SH(1) | DOSWAP_SH(1)),
        TYPE_BGR_8_PLANAR = (COLORSPACE_SH(PT_RGB) | CHANNELS_SH(3) | BYTES_SH(1) | DOSWAP_SH(1) | PLANAR_SH(1)),
        TYPE_RGB_16 = (COLORSPACE_SH(PT_RGB) | CHANNELS_SH(3) | BYTES_SH(2)),
        TYPE_RGB_16_PLANAR = (COLORSPACE_SH(PT_RGB) | CHANNELS_SH(3) | BYTES_SH(2) | PLANAR_SH(1)),
        TYPE_RGB_16_SE = (COLORSPACE_SH(PT_RGB) | CHANNELS_SH(3) | BYTES_SH(2) | ENDIAN16_SH(1)),
        TYPE_BGR_16 = (COLORSPACE_SH(PT_RGB) | CHANNELS_SH(3) | BYTES_SH(2) | DOSWAP_SH(1)),
        TYPE_BGR_16_PLANAR = (COLORSPACE_SH(PT_RGB) | CHANNELS_SH(3) | BYTES_SH(2) | DOSWAP_SH(1) | PLANAR_SH(1)),
        TYPE_BGR_16_SE = (COLORSPACE_SH(PT_RGB) | CHANNELS_SH(3) | BYTES_SH(2) | DOSWAP_SH(1) | ENDIAN16_SH(1)),

        TYPE_RGBA_8 = (COLORSPACE_SH(PT_RGB) | EXTRA_SH(1) | CHANNELS_SH(3) | BYTES_SH(1)),
        TYPE_RGBA_8_PLANAR = (COLORSPACE_SH(PT_RGB) | EXTRA_SH(1) | CHANNELS_SH(3) | BYTES_SH(1) | PLANAR_SH(1)),
        TYPE_RGBA_16 = (COLORSPACE_SH(PT_RGB) | EXTRA_SH(1) | CHANNELS_SH(3) | BYTES_SH(2)),
        TYPE_RGBA_16_PLANAR = (COLORSPACE_SH(PT_RGB) | EXTRA_SH(1) | CHANNELS_SH(3) | BYTES_SH(2) | PLANAR_SH(1)),
        TYPE_RGBA_16_SE = (COLORSPACE_SH(PT_RGB) | EXTRA_SH(1) | CHANNELS_SH(3) | BYTES_SH(2) | ENDIAN16_SH(1)),

        TYPE_ARGB_8 = (COLORSPACE_SH(PT_RGB) | EXTRA_SH(1) | CHANNELS_SH(3) | BYTES_SH(1) | SWAPFIRST_SH(1)),
        TYPE_ARGB_16 = (COLORSPACE_SH(PT_RGB) | EXTRA_SH(1) | CHANNELS_SH(3) | BYTES_SH(2) | SWAPFIRST_SH(1)),

        TYPE_ABGR_8 = (COLORSPACE_SH(PT_RGB) | EXTRA_SH(1) | CHANNELS_SH(3) | BYTES_SH(1) | DOSWAP_SH(1)),
        TYPE_ABGR_16 = (COLORSPACE_SH(PT_RGB) | EXTRA_SH(1) | CHANNELS_SH(3) | BYTES_SH(2) | DOSWAP_SH(1)),
        TYPE_ABGR_16_PLANAR = (COLORSPACE_SH(PT_RGB) | EXTRA_SH(1) | CHANNELS_SH(3) | BYTES_SH(2) | DOSWAP_SH(1) | PLANAR_SH(1)),
        TYPE_ABGR_16_SE = (COLORSPACE_SH(PT_RGB) | EXTRA_SH(1) | CHANNELS_SH(3) | BYTES_SH(2) | DOSWAP_SH(1) | ENDIAN16_SH(1)),

        TYPE_BGRA_8 = (COLORSPACE_SH(PT_RGB) | EXTRA_SH(1) | CHANNELS_SH(3) | BYTES_SH(1) | DOSWAP_SH(1) | SWAPFIRST_SH(1)),
        TYPE_BGRA_16 = (COLORSPACE_SH(PT_RGB) | EXTRA_SH(1) | CHANNELS_SH(3) | BYTES_SH(2) | DOSWAP_SH(1) | SWAPFIRST_SH(1)),
        TYPE_BGRA_16_SE = (COLORSPACE_SH(PT_RGB) | EXTRA_SH(1) | CHANNELS_SH(3) | BYTES_SH(2) | ENDIAN16_SH(1) | SWAPFIRST_SH(1)),

        TYPE_CMY_8 = (COLORSPACE_SH(PT_CMY) | CHANNELS_SH(3) | BYTES_SH(1)),
        TYPE_CMY_8_PLANAR = (COLORSPACE_SH(PT_CMY) | CHANNELS_SH(3) | BYTES_SH(1) | PLANAR_SH(1)),
        TYPE_CMY_16 = (COLORSPACE_SH(PT_CMY) | CHANNELS_SH(3) | BYTES_SH(2)),
        TYPE_CMY_16_PLANAR = (COLORSPACE_SH(PT_CMY) | CHANNELS_SH(3) | BYTES_SH(2) | PLANAR_SH(1)),
        TYPE_CMY_16_SE = (COLORSPACE_SH(PT_CMY) | CHANNELS_SH(3) | BYTES_SH(2) | ENDIAN16_SH(1)),

        TYPE_CMYK_8 = (COLORSPACE_SH(PT_CMYK) | CHANNELS_SH(4) | BYTES_SH(1)),
        TYPE_CMYKA_8 = (COLORSPACE_SH(PT_CMYK) | EXTRA_SH(1) | CHANNELS_SH(4) | BYTES_SH(1)),
        TYPE_CMYK_8_REV = (COLORSPACE_SH(PT_CMYK) | CHANNELS_SH(4) | BYTES_SH(1) | FLAVOR_SH(1)),
        TYPE_YUVK_8 = TYPE_CMYK_8_REV,
        TYPE_CMYK_8_PLANAR = (COLORSPACE_SH(PT_CMYK) | CHANNELS_SH(4) | BYTES_SH(1) | PLANAR_SH(1)),
        TYPE_CMYK_16 = (COLORSPACE_SH(PT_CMYK) | CHANNELS_SH(4) | BYTES_SH(2)),
        TYPE_CMYK_16_REV = (COLORSPACE_SH(PT_CMYK) | CHANNELS_SH(4) | BYTES_SH(2) | FLAVOR_SH(1)),
        TYPE_YUVK_16 = TYPE_CMYK_16_REV,
        TYPE_CMYK_16_PLANAR = (COLORSPACE_SH(PT_CMYK) | CHANNELS_SH(4) | BYTES_SH(2) | PLANAR_SH(1)),
        TYPE_CMYK_16_SE = (COLORSPACE_SH(PT_CMYK) | CHANNELS_SH(4) | BYTES_SH(2) | ENDIAN16_SH(1)),

        TYPE_KYMC_8 = (COLORSPACE_SH(PT_CMYK) | CHANNELS_SH(4) | BYTES_SH(1) | DOSWAP_SH(1)),
        TYPE_KYMC_16 = (COLORSPACE_SH(PT_CMYK) | CHANNELS_SH(4) | BYTES_SH(2) | DOSWAP_SH(1)),
        TYPE_KYMC_16_SE = (COLORSPACE_SH(PT_CMYK) | CHANNELS_SH(4) | BYTES_SH(2) | DOSWAP_SH(1) | ENDIAN16_SH(1)),

        TYPE_KCMY_8 = (COLORSPACE_SH(PT_CMYK) | CHANNELS_SH(4) | BYTES_SH(1) | SWAPFIRST_SH(1)),
        TYPE_KCMY_8_REV = (COLORSPACE_SH(PT_CMYK) | CHANNELS_SH(4) | BYTES_SH(1) | FLAVOR_SH(1) | SWAPFIRST_SH(1)),
        TYPE_KCMY_16 = (COLORSPACE_SH(PT_CMYK) | CHANNELS_SH(4) | BYTES_SH(2) | SWAPFIRST_SH(1)),
        TYPE_KCMY_16_REV = (COLORSPACE_SH(PT_CMYK) | CHANNELS_SH(4) | BYTES_SH(2) | FLAVOR_SH(1) | SWAPFIRST_SH(1)),
        TYPE_KCMY_16_SE = (COLORSPACE_SH(PT_CMYK) | CHANNELS_SH(4) | BYTES_SH(2) | ENDIAN16_SH(1) | SWAPFIRST_SH(1)),

        TYPE_CMYK5_8 = (COLORSPACE_SH(PT_MCH5) | CHANNELS_SH(5) | BYTES_SH(1)),
        TYPE_CMYK5_16 = (COLORSPACE_SH(PT_MCH5) | CHANNELS_SH(5) | BYTES_SH(2)),
        TYPE_CMYK5_16_SE = (COLORSPACE_SH(PT_MCH5) | CHANNELS_SH(5) | BYTES_SH(2) | ENDIAN16_SH(1)),
        TYPE_KYMC5_8 = (COLORSPACE_SH(PT_MCH5) | CHANNELS_SH(5) | BYTES_SH(1) | DOSWAP_SH(1)),
        TYPE_KYMC5_16 = (COLORSPACE_SH(PT_MCH5) | CHANNELS_SH(5) | BYTES_SH(2) | DOSWAP_SH(1)),
        TYPE_KYMC5_16_SE = (COLORSPACE_SH(PT_MCH5) | CHANNELS_SH(5) | BYTES_SH(2) | DOSWAP_SH(1) | ENDIAN16_SH(1)),
        TYPE_CMYK6_8 = (COLORSPACE_SH(PT_MCH6) | CHANNELS_SH(6) | BYTES_SH(1)),
        TYPE_CMYK6_8_PLANAR = (COLORSPACE_SH(PT_MCH6) | CHANNELS_SH(6) | BYTES_SH(1) | PLANAR_SH(1)),
        TYPE_CMYK6_16 = (COLORSPACE_SH(PT_MCH6) | CHANNELS_SH(6) | BYTES_SH(2)),
        TYPE_CMYK6_16_PLANAR = (COLORSPACE_SH(PT_MCH6) | CHANNELS_SH(6) | BYTES_SH(2) | PLANAR_SH(1)),
        TYPE_CMYK6_16_SE = (COLORSPACE_SH(PT_MCH6) | CHANNELS_SH(6) | BYTES_SH(2) | ENDIAN16_SH(1)),
        TYPE_CMYK7_8 = (COLORSPACE_SH(PT_MCH7) | CHANNELS_SH(7) | BYTES_SH(1)),
        TYPE_CMYK7_16 = (COLORSPACE_SH(PT_MCH7) | CHANNELS_SH(7) | BYTES_SH(2)),
        TYPE_CMYK7_16_SE = (COLORSPACE_SH(PT_MCH7) | CHANNELS_SH(7) | BYTES_SH(2) | ENDIAN16_SH(1)),
        TYPE_KYMC7_8 = (COLORSPACE_SH(PT_MCH7) | CHANNELS_SH(7) | BYTES_SH(1) | DOSWAP_SH(1)),
        TYPE_KYMC7_16 = (COLORSPACE_SH(PT_MCH7) | CHANNELS_SH(7) | BYTES_SH(2) | DOSWAP_SH(1)),
        TYPE_KYMC7_16_SE = (COLORSPACE_SH(PT_MCH7) | CHANNELS_SH(7) | BYTES_SH(2) | DOSWAP_SH(1) | ENDIAN16_SH(1)),
        TYPE_CMYK8_8 = (COLORSPACE_SH(PT_MCH8) | CHANNELS_SH(8) | BYTES_SH(1)),
        TYPE_CMYK8_16 = (COLORSPACE_SH(PT_MCH8) | CHANNELS_SH(8) | BYTES_SH(2)),
        TYPE_CMYK8_16_SE = (COLORSPACE_SH(PT_MCH8) | CHANNELS_SH(8) | BYTES_SH(2) | ENDIAN16_SH(1)),
        TYPE_KYMC8_8 = (COLORSPACE_SH(PT_MCH8) | CHANNELS_SH(8) | BYTES_SH(1) | DOSWAP_SH(1)),
        TYPE_KYMC8_16 = (COLORSPACE_SH(PT_MCH8) | CHANNELS_SH(8) | BYTES_SH(2) | DOSWAP_SH(1)),
        TYPE_KYMC8_16_SE = (COLORSPACE_SH(PT_MCH8) | CHANNELS_SH(8) | BYTES_SH(2) | DOSWAP_SH(1) | ENDIAN16_SH(1)),
        TYPE_CMYK9_8 = (COLORSPACE_SH(PT_MCH9) | CHANNELS_SH(9) | BYTES_SH(1)),
        TYPE_CMYK9_16 = (COLORSPACE_SH(PT_MCH9) | CHANNELS_SH(9) | BYTES_SH(2)),
        TYPE_CMYK9_16_SE = (COLORSPACE_SH(PT_MCH9) | CHANNELS_SH(9) | BYTES_SH(2) | ENDIAN16_SH(1)),
        TYPE_KYMC9_8 = (COLORSPACE_SH(PT_MCH9) | CHANNELS_SH(9) | BYTES_SH(1) | DOSWAP_SH(1)),
        TYPE_KYMC9_16 = (COLORSPACE_SH(PT_MCH9) | CHANNELS_SH(9) | BYTES_SH(2) | DOSWAP_SH(1)),
        TYPE_KYMC9_16_SE = (COLORSPACE_SH(PT_MCH9) | CHANNELS_SH(9) | BYTES_SH(2) | DOSWAP_SH(1) | ENDIAN16_SH(1)),
        TYPE_CMYK10_8 = (COLORSPACE_SH(PT_MCH10) | CHANNELS_SH(10) | BYTES_SH(1)),
        TYPE_CMYK10_16 = (COLORSPACE_SH(PT_MCH10) | CHANNELS_SH(10) | BYTES_SH(2)),
        TYPE_CMYK10_16_SE = (COLORSPACE_SH(PT_MCH10) | CHANNELS_SH(10) | BYTES_SH(2) | ENDIAN16_SH(1)),
        TYPE_KYMC10_8 = (COLORSPACE_SH(PT_MCH10) | CHANNELS_SH(10) | BYTES_SH(1) | DOSWAP_SH(1)),
        TYPE_KYMC10_16 = (COLORSPACE_SH(PT_MCH10) | CHANNELS_SH(10) | BYTES_SH(2) | DOSWAP_SH(1)),
        TYPE_KYMC10_16_SE = (COLORSPACE_SH(PT_MCH10) | CHANNELS_SH(10) | BYTES_SH(2) | DOSWAP_SH(1) | ENDIAN16_SH(1)),
        TYPE_CMYK11_8 = (COLORSPACE_SH(PT_MCH11) | CHANNELS_SH(11) | BYTES_SH(1)),
        TYPE_CMYK11_16 = (COLORSPACE_SH(PT_MCH11) | CHANNELS_SH(11) | BYTES_SH(2)),
        TYPE_CMYK11_16_SE = (COLORSPACE_SH(PT_MCH11) | CHANNELS_SH(11) | BYTES_SH(2) | ENDIAN16_SH(1)),
        TYPE_KYMC11_8 = (COLORSPACE_SH(PT_MCH11) | CHANNELS_SH(11) | BYTES_SH(1) | DOSWAP_SH(1)),
        TYPE_KYMC11_16 = (COLORSPACE_SH(PT_MCH11) | CHANNELS_SH(11) | BYTES_SH(2) | DOSWAP_SH(1)),
        TYPE_KYMC11_16_SE = (COLORSPACE_SH(PT_MCH11) | CHANNELS_SH(11) | BYTES_SH(2) | DOSWAP_SH(1) | ENDIAN16_SH(1)),
        TYPE_CMYK12_8 = (COLORSPACE_SH(PT_MCH12) | CHANNELS_SH(12) | BYTES_SH(1)),
        TYPE_CMYK12_16 = (COLORSPACE_SH(PT_MCH12) | CHANNELS_SH(12) | BYTES_SH(2)),
        TYPE_CMYK12_16_SE = (COLORSPACE_SH(PT_MCH12) | CHANNELS_SH(12) | BYTES_SH(2) | ENDIAN16_SH(1)),
        TYPE_KYMC12_8 = (COLORSPACE_SH(PT_MCH12) | CHANNELS_SH(12) | BYTES_SH(1) | DOSWAP_SH(1)),
        TYPE_KYMC12_16 = (COLORSPACE_SH(PT_MCH12) | CHANNELS_SH(12) | BYTES_SH(2) | DOSWAP_SH(1)),
        TYPE_KYMC12_16_SE = (COLORSPACE_SH(PT_MCH12) | CHANNELS_SH(12) | BYTES_SH(2) | DOSWAP_SH(1) | ENDIAN16_SH(1)),

        // Colorimetric               
        TYPE_XYZ_16 = (COLORSPACE_SH(PT_XYZ) | CHANNELS_SH(3) | BYTES_SH(2)),
        TYPE_Lab_8 = (COLORSPACE_SH(PT_Lab) | CHANNELS_SH(3) | BYTES_SH(1)),
        TYPE_LabV2_8 = (COLORSPACE_SH(PT_LabV2) | CHANNELS_SH(3) | BYTES_SH(1)),

        TYPE_ALab_8 = (COLORSPACE_SH(PT_Lab) | CHANNELS_SH(3) | BYTES_SH(1) | EXTRA_SH(1) | DOSWAP_SH(1)),
        TYPE_ALabV2_8 = (COLORSPACE_SH(PT_LabV2) | CHANNELS_SH(3) | BYTES_SH(1) | EXTRA_SH(1) | DOSWAP_SH(1)),
        TYPE_Lab_16 = (COLORSPACE_SH(PT_Lab) | CHANNELS_SH(3) | BYTES_SH(2)),
        TYPE_LabV2_16 = (COLORSPACE_SH(PT_LabV2) | CHANNELS_SH(3) | BYTES_SH(2)),
        TYPE_Yxy_16 = (COLORSPACE_SH(PT_Yxy) | CHANNELS_SH(3) | BYTES_SH(2)),

        // YCbCr                      
        TYPE_YCbCr_8 = (COLORSPACE_SH(PT_YCbCr) | CHANNELS_SH(3) | BYTES_SH(1)),
        TYPE_YCbCr_8_PLANAR = (COLORSPACE_SH(PT_YCbCr) | CHANNELS_SH(3) | BYTES_SH(1) | PLANAR_SH(1)),
        TYPE_YCbCr_16 = (COLORSPACE_SH(PT_YCbCr) | CHANNELS_SH(3) | BYTES_SH(2)),
        TYPE_YCbCr_16_PLANAR = (COLORSPACE_SH(PT_YCbCr) | CHANNELS_SH(3) | BYTES_SH(2) | PLANAR_SH(1)),
        TYPE_YCbCr_16_SE = (COLORSPACE_SH(PT_YCbCr) | CHANNELS_SH(3) | BYTES_SH(2) | ENDIAN16_SH(1)),

        // YUV                        
        TYPE_YUV_8 = (COLORSPACE_SH(PT_YUV) | CHANNELS_SH(3) | BYTES_SH(1)),
        TYPE_YUV_8_PLANAR = (COLORSPACE_SH(PT_YUV) | CHANNELS_SH(3) | BYTES_SH(1) | PLANAR_SH(1)),
        TYPE_YUV_16 = (COLORSPACE_SH(PT_YUV) | CHANNELS_SH(3) | BYTES_SH(2)),
        TYPE_YUV_16_PLANAR = (COLORSPACE_SH(PT_YUV) | CHANNELS_SH(3) | BYTES_SH(2) | PLANAR_SH(1)),
        TYPE_YUV_16_SE = (COLORSPACE_SH(PT_YUV) | CHANNELS_SH(3) | BYTES_SH(2) | ENDIAN16_SH(1)),

        // HLS                        
        TYPE_HLS_8 = (COLORSPACE_SH(PT_HLS) | CHANNELS_SH(3) | BYTES_SH(1)),
        TYPE_HLS_8_PLANAR = (COLORSPACE_SH(PT_HLS) | CHANNELS_SH(3) | BYTES_SH(1) | PLANAR_SH(1)),
        TYPE_HLS_16 = (COLORSPACE_SH(PT_HLS) | CHANNELS_SH(3) | BYTES_SH(2)),
        TYPE_HLS_16_PLANAR = (COLORSPACE_SH(PT_HLS) | CHANNELS_SH(3) | BYTES_SH(2) | PLANAR_SH(1)),
        TYPE_HLS_16_SE = (COLORSPACE_SH(PT_HLS) | CHANNELS_SH(3) | BYTES_SH(2) | ENDIAN16_SH(1)),

        // HSV                        
        TYPE_HSV_8 = (COLORSPACE_SH(PT_HSV) | CHANNELS_SH(3) | BYTES_SH(1)),
        TYPE_HSV_8_PLANAR = (COLORSPACE_SH(PT_HSV) | CHANNELS_SH(3) | BYTES_SH(1) | PLANAR_SH(1)),
        TYPE_HSV_16 = (COLORSPACE_SH(PT_HSV) | CHANNELS_SH(3) | BYTES_SH(2)),
        TYPE_HSV_16_PLANAR = (COLORSPACE_SH(PT_HSV) | CHANNELS_SH(3) | BYTES_SH(2) | PLANAR_SH(1)),
        TYPE_HSV_16_SE = (COLORSPACE_SH(PT_HSV) | CHANNELS_SH(3) | BYTES_SH(2) | ENDIAN16_SH(1)),

        // Float formatters.
        TYPE_XYZ_FLT = (FLOAT_SH(1) | COLORSPACE_SH(PT_XYZ) | CHANNELS_SH(3) | BYTES_SH(4)),
        TYPE_XYZA_FLT = (FLOAT_SH(1) | COLORSPACE_SH(PT_XYZ) | EXTRA_SH(1) | CHANNELS_SH(3) | BYTES_SH(4)),
        TYPE_Lab_FLT = (FLOAT_SH(1) | COLORSPACE_SH(PT_Lab) | CHANNELS_SH(3) | BYTES_SH(4)),
        TYPE_LabA_FLT = (FLOAT_SH(1) | COLORSPACE_SH(PT_Lab) | EXTRA_SH(1) | CHANNELS_SH(3) | BYTES_SH(4)),
        TYPE_GRAY_FLT = (FLOAT_SH(1) | COLORSPACE_SH(PT_GRAY) | CHANNELS_SH(1) | BYTES_SH(4)),
        TYPE_RGB_FLT = (FLOAT_SH(1) | COLORSPACE_SH(PT_RGB) | CHANNELS_SH(3) | BYTES_SH(4)),
        TYPE_RGBA_FLT = (FLOAT_SH(1) | COLORSPACE_SH(PT_RGB) | EXTRA_SH(1) | CHANNELS_SH(3) | BYTES_SH(4)),
        TYPE_ARGB_FLT = (FLOAT_SH(1) | COLORSPACE_SH(PT_RGB) | EXTRA_SH(1) | CHANNELS_SH(3) | BYTES_SH(4) | SWAPFIRST_SH(1)),
        TYPE_BGR_FLT = (FLOAT_SH(1) | COLORSPACE_SH(PT_RGB) | CHANNELS_SH(3) | BYTES_SH(4) | DOSWAP_SH(1)),
        TYPE_BGRA_FLT = (FLOAT_SH(1) | COLORSPACE_SH(PT_RGB) | EXTRA_SH(1) | CHANNELS_SH(3) | BYTES_SH(4) | DOSWAP_SH(1) | SWAPFIRST_SH(1)),
        TYPE_ABGR_FLT = (FLOAT_SH(1) | COLORSPACE_SH(PT_RGB) | EXTRA_SH(1) | CHANNELS_SH(3) | BYTES_SH(4) | DOSWAP_SH(1)),

        TYPE_CMYK_FLT = (FLOAT_SH(1) | COLORSPACE_SH(PT_CMYK) | CHANNELS_SH(4) | BYTES_SH(4)),

        // Floating point formatters.  
        // NOTE THAT 'BYTES' FIELD IS SET TO ZERO ON DLB because 8 bytes overflows the bitfield
        TYPE_XYZ_DBL = (FLOAT_SH(1) | COLORSPACE_SH(PT_XYZ) | CHANNELS_SH(3) | BYTES_SH(0)),
        TYPE_Lab_DBL = (FLOAT_SH(1) | COLORSPACE_SH(PT_Lab) | CHANNELS_SH(3) | BYTES_SH(0)),
        TYPE_GRAY_DBL = (FLOAT_SH(1) | COLORSPACE_SH(PT_GRAY) | CHANNELS_SH(1) | BYTES_SH(0)),
        TYPE_RGB_DBL = (FLOAT_SH(1) | COLORSPACE_SH(PT_RGB) | CHANNELS_SH(3) | BYTES_SH(0)),
        TYPE_BGR_DBL = (FLOAT_SH(1) | COLORSPACE_SH(PT_RGB) | CHANNELS_SH(3) | BYTES_SH(0) | DOSWAP_SH(1)),
        TYPE_CMYK_DBL = (FLOAT_SH(1) | COLORSPACE_SH(PT_CMYK) | CHANNELS_SH(4) | BYTES_SH(0)),


        // IEEE 754-2008 "half"
        TYPE_GRAY_HALF_FLT = (FLOAT_SH(1) | COLORSPACE_SH(PT_GRAY) | CHANNELS_SH(1) | BYTES_SH(2)),
        TYPE_RGB_HALF_FLT = (FLOAT_SH(1) | COLORSPACE_SH(PT_RGB) | CHANNELS_SH(3) | BYTES_SH(2)),
        TYPE_CMYK_HALF_FLT = (FLOAT_SH(1) | COLORSPACE_SH(PT_CMYK) | CHANNELS_SH(4) | BYTES_SH(2)),
        TYPE_RGBA_HALF_FLT = (FLOAT_SH(1) | COLORSPACE_SH(PT_RGB) | EXTRA_SH(1) | CHANNELS_SH(3) | BYTES_SH(2)),
        TYPE_ARGB_HALF_FLT = (FLOAT_SH(1) | COLORSPACE_SH(PT_RGB) | EXTRA_SH(1) | CHANNELS_SH(3) | BYTES_SH(2) | SWAPFIRST_SH(1)),
        TYPE_BGR_HALF_FLT = (FLOAT_SH(1) | COLORSPACE_SH(PT_RGB) | CHANNELS_SH(3) | BYTES_SH(2) | DOSWAP_SH(1)),
        TYPE_BGRA_HALF_FLT = (FLOAT_SH(1) | COLORSPACE_SH(PT_RGB) | EXTRA_SH(1) | CHANNELS_SH(3) | BYTES_SH(2) | DOSWAP_SH(1) | SWAPFIRST_SH(1)),
        TYPE_ABGR_HALF_FLT = (FLOAT_SH(1) | COLORSPACE_SH(PT_RGB) | CHANNELS_SH(3) | BYTES_SH(2) | DOSWAP_SH(1))
    };

protected:

    /**
    * Everything goes packed as a 32-bit unsigned int
    */
    uint32_t  format_;
};


    class ICCProfile
    {

    public:

       ICCProfile(const ICCProfile& cpy);
       ICCProfile(Context* ctx);

       void save(const wchar_t* fileName);
       void save(std::ostream& strm);
       void save(const uint8_t* ptr, size_t size);
       void save(IIOhandler* out);

       enum class InfoType
       {
           InfoDescription  = 0,
           InfoManufacturer = 1,
           InfoModel        = 2,
           InfoCopyright    = 3
       };

       void info(InfoType t, 
                const char LanguageCode[3], const char CountryCode[3],
                wchar_t* buffer, uint32_t size));

       Context* context();

       // TODO: review enumerator
       uint32_t tagCount();
       TagSignature tagSignature(uint32_t n);

       bool containsTag(TagSignature tag);

       Tag* readTag(TagSignature tag);
       void writeTag(TagSignature tag, const Tag* tag);
       void linkTag(TagSignature sig, TagSignature dest);
       TagSignature tagLinkedTo(TagSignature sig);


       uint32_t readRawTag(TagSignature sig, void* buffer, size_t size);
       void     writeRawTag(cmsTagSignature sig, const void* buffer, size_t size);

       uint32_t        headerFlags();
       uint64_t        headerAttributes();
       const uint8_t*  profileID();
       void            headerCreationTime(tm& t);
       RenderingIntent headerRenderingIntent();

       uint32_t        manufacturer();
       uint32_t        model();

       ColorSpaceSignature pcs();
       ColorSpaceSignature colorSpace();

       DeviceClass deviceClass();
       double version();
       uint32_t encodedVersion();

       enum class Directions
       {
           AsInput,
           AsOutput,
           AsProofing
       };

       bool isIntentSupported(RenderingIntent ri, Directions direction);
       bool isMatrixShaper(Directions direction);
       bool isCLUT(Directions direction);


       void setHeaderFlags(uint32_t flags);
       void setHeaderAttributes(uint64_t attributes);
       void setHeaderCreationTime(const tm& t);
       void settHeaderManufacturer(uint32_t m);
       void settHeaderModel(uint32_t m);
       void setProfileID(const uint8_t* id);
       void settHeaderRenderingIntent(RenderingIntent ri);
       void setPCS(ColorSpace pcs);
       void setColorSpace(ColorSpace pcs);
       void setDeviceClass(DeviceClass dev);

       void setVersion(double version);
       void setEncodedVersion(uint32_t v);



       formatterForColorspace();
       formatterForPCS();


    };


        static IICCProfile* openFromFile(Context* ctx, const wchar_t* fileName, const char* access);
        static IICCProfile* openFromStream(Context* ctx, std::istream& strm);
        static IICCProfile* openFromMem(Context* ctx, const uint8_t* ptr, size_t size);
        static IICCProfile* openFromIOhandler(Context* ctx, IIOhandler* io);

        static IICCProfile* createRGB(Context* ctx,
                                      const cmsCIExyY* WhitePoint,
                                      const CIExyYTRIPLE* Primaries,
                                      ToneCurve* const TransferFunction[3]);

       static IICCProfile* createGray(Context* ctx,
                                      const CIExyY* WhitePoint,                                      
                                      const ToneCurve* TransferFunction);

       static IICCProfile* createLinearizationDeviceLink(Context* ctx,
                                     ColorSpaceSignature ColorSpace,
                                     ToneCurve* const TransferFunctions[]);


       static IICCProfile* createInkLimitingDeviceLink(Context* ctx,
                                     ColorSpaceSignature ColorSpace, double limit);

       static IICCProfile* createBCHSWabstract(Context* ctx,
                                     uint32_t nLUTPoints,
                                     double bright,
                                     double contrast,
                                     double Hue,
                                     double Saturation,
                                     double TempSrc,
                                     double TempDest);

       static IICCProfile* createDevicelink(IColorTrasform* src, double Version, uint32_t flags);

       static IICCProfile* createNULL(Context* ctx);




}


#endif

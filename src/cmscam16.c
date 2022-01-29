//---------------------------------------------------------------------------------
//
//  Little Color Management System
//  Copyright (c) 1998-2022 Marti Maria Saguer
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

#include "cmscam16.h"

// CIECAM 16 and CIECAM 16 UCS appearance models. 

// ---------- Implementation --------------------------------------------

// Some needed constants
#ifndef M_PI
#       define M_PI        3.14159265358979323846
#endif


typedef struct {

    cmsContext ContextID;

    cmsCIEXYZ XYZ_w;
    cmsFloat64Number RGB_w[3];
    cmsFloat64Number D_RGB[3];
    cmsFloat64Number D_RGB_inv[3];
    cmsFloat64Number RGB_cw[3];
    cmsFloat64Number RGB_aw[3];

    cmsFloat64Number LA, Yb, Yw;
    cmsFloat64Number F, c, Nc;
    cmsUInt32Number surround;
    cmsFloat64Number n, Nbb, Ncb, z, FL, FL4, D, k, A_w;


} cmsCIECAM16;


/**
* Valid correlates are 
*/

static
cmsFloat64Number radians(cmsFloat64Number deg)
{
    return (fmod(deg, 360.0) * M_PI) / 180.0;
}

static
cmsFloat64Number degrees(cmsFloat64Number rad)
{
    cmsFloat64Number deg = fmod(rad * 180.0 / M_PI, 360);

    while (deg < 0)
    {
        deg += 360.0;
    }

    return deg;
}

static 
cmsFloat64Number lerp(cmsFloat64Number l, cmsFloat64Number h, cmsFloat64Number a)
{
    return (cmsFloat64Number) l + (h-l) * a;
}

static
cmsFloat64Number clip(cmsFloat64Number l, cmsFloat64Number h, cmsFloat64Number a)
{
    if (a < l) return l;
    if (a > h) return h;
    return a;
}

static
cmsFloat64Number sgn(cmsFloat64Number s)
{
    if (s < 0.0) return -1.0;
    if (s > 0.0) return +1.0;
    return 0.0;
}


static
void M16(cmsFloat64Number RGB[3], const cmsCIEXYZ* xyz)
{
    RGB[0] = +0.401288 * xyz->X + 0.650173 * xyz->Y - 0.051461 * xyz->Z;
    RGB[1] = -0.250268 * xyz->X + 1.204414 * xyz->Y + 0.045854 * xyz->Z;
    RGB[2] = -0.002079 * xyz->X + 0.048952 * xyz->Y + 0.953127 * xyz->Z;
}

static
void M16_inv(cmsCIEXYZ* xyz, const cmsFloat64Number RGB[3])
{
    xyz->X = +1.862067855087233e+0 * RGB[0] - 1.011254630531685e+0 * RGB[1] + 1.491867754444518e-1 * RGB[2];
    xyz->Y = +3.875265432361372e-1 * RGB[0] + 6.214474419314753e-1 * RGB[1] - 8.973985167612518e-3 * RGB[2];
    xyz->Z = -1.584149884933386e-2 * RGB[0] - 3.412293802851557e-2 * RGB[1] + 1.049964436877850e+0 * RGB[2];
}

static 
cmsFloat64Number adapt(cmsFloat64Number F_L, cmsFloat64Number component)
{
    cmsFloat64Number x = pow(F_L * fabs(component) * 0.01, 0.42);

    return sgn(component) * 400.0 * x / (x + 27.13); 
}

static
cmsFloat64Number unadapt(cmsFloat64Number F_L, cmsFloat64Number component)
{
    const cmsFloat64Number exponent = 1.0 / 0.42;
    cmsFloat64Number constant = 100 / F_L * pow(27.13, exponent);
    cmsFloat64Number cabs;
  
    cabs = fabs(component);

    return sgn(component) * constant * pow(cabs / (400.0 - cabs), exponent); 
}


static 
void elem_mul(cmsFloat64Number res[3], const cmsFloat64Number v0[3], const cmsFloat64Number v1[3])
{
    res[0] = v0[0] * v1[0];
    res[1] = v0[1] * v1[1];
    res[2] = v0[2] * v1[2];    
}


cmsHANDLE CMSEXPORT cmsCIECAM16Init(cmsContext ContextID, const cmsViewingConditions* vc)
{
    cmsCIECAM16* p;
    cmsFloat64Number k4;
    
    p = (cmsCIECAM16*)_cmsMallocZero(ContextID, sizeof(cmsCIECAM16));

    if (p == NULL) {
        return NULL;
    }

    p->ContextID = ContextID;

    p->XYZ_w = vc->whitePoint;
    
    p->LA = vc->La;
    p->Yb = vc->Yb;
    p->Yw = vc->whitePoint.Y;

    p->surround = vc->surround;

    p->c = (p->surround >= 1) ? lerp(0.59, 0.69, p->surround - 1) : lerp(0.525, 0.59, p->surround);

    p->F = (p->c >= 0.59) ? lerp(0.9, 1.0, (p->c - 0.59) / .1) : lerp(0.8, 0.9, (p->c - 0.525) / 0.065);

    p->Nc = p->F;
    p->k = 1.0 / (5.0 * p->LA + 1.0);

    k4 = p->k * p->k * p->k * p->k;

    p->FL = k4 * p->LA + 0.1 * (1 - k4) * (1 - k4) * pow(5.0 * p->LA, 1.0 / 3.0);

    p->FL4 = pow(p->FL, 0.25);

    p->n = p->Yb / p->Yw;

    p->z = 1.48 + sqrt(p->n);         // Lightness non-linearity exponent (modified by c)
    p->Nbb = 0.725 * pow(p->n, -0.2); // Chromatic induction factors
    p->Ncb = p->Nbb;

    // CAM16 has discounting for anything but d==1
    p->D = (vc->D_value < 1.0) ? clip(0, 1, p->F * (1.0 - 1.0 / 3.6 * exp((-p->LA - 42.0) / 92.0))) : 1.0;
    
    M16(p->RGB_w, &p->XYZ_w); // Cone responses of the white point

    p->D_RGB[0] = lerp(1, p->Yw / p->RGB_w[0], p->D);   //     p->D * p->Yw / p->RGB_w[0] + 1.0 - p->D;
    p->D_RGB[1] = lerp(1, p->Yw / p->RGB_w[1], p->D);   //     p->D * p->Yw / p->RGB_w[1] + 1.0 - p->D;
    p->D_RGB[2] = lerp(1, p->Yw / p->RGB_w[2], p->D);   //     p->D * p->Yw / p->RGB_w[2] + 1.0 - p->D;
                                            
    p->D_RGB_inv[0] = 1.0 / p->D_RGB[0];
    p->D_RGB_inv[1] = 1.0 / p->D_RGB[1];
    p->D_RGB_inv[2] = 1.0 / p->D_RGB[2];

    p->RGB_cw[0] = p-> RGB_w[0] * p->D_RGB[0];
    p->RGB_cw[1] = p-> RGB_w[1] * p->D_RGB[1];
    p->RGB_cw[2] = p-> RGB_w[2] * p->D_RGB[2];
        
    p->RGB_aw[0] = adapt(p->FL, p->RGB_cw[0]);
    p->RGB_aw[1] = adapt(p->FL, p->RGB_cw[1]);
    p->RGB_aw[2] = adapt(p->FL, p->RGB_cw[2]);

    p->A_w = p->Nbb * (2.0 * p->RGB_aw[0] + p->RGB_aw[1] + 0.05 * p->RGB_aw[2]);


    return (cmsHANDLE)p;

}

void CMSEXPORT cmsCIECAM16Forward(cmsHANDLE hModel, const cmsCIEXYZ* pIn, cmsCIECAM16Color* pOut)
{
    cmsFloat64Number RGB[3], RGB_a[3];
    cmsFloat64Number a, b, h_rad, e_t, A, J_root, t, alpha;

    cmsCIECAM16* p = (cmsCIECAM16*)hModel;

    M16(RGB, pIn);

    RGB_a[0] = adapt(p->FL, RGB[0] * p->D_RGB[0]);
    RGB_a[1] = adapt(p->FL, RGB[1] * p->D_RGB[1]);
    RGB_a[2] = adapt(p->FL, RGB[2] * p->D_RGB[2]);

    a = RGB_a[0] + (-12 * RGB_a[1] + RGB_a[2]) / 11;          // redness-greenness
    b = (RGB_a[0] + RGB_a[1] - 2 * RGB_a[2]) / 9;             // yellowness-blueness

    h_rad = atan2(b, a);                                      // hue in radians
    pOut->h = degrees(h_rad);                                // hue in degrees

    e_t = 0.25 * (cos(h_rad + 2) + 3.8);

    A = p->Nbb * (2.0 * RGB_a[0] + RGB_a[1] + 0.05 * RGB_a[2]);
    
    J_root = pow(A / p->A_w, 0.5 * p->c * p->z);

    pOut->J = 100 * J_root * J_root;                          // lightness
    pOut->Q = (4 / p->c * J_root * (p->A_w + 4) * p->FL4);    // brightness

    t = (5e4 / 13.0 * p->Nc * p->Ncb * e_t * sqrt(a * a + b * b) /
        (RGB_a[0] + RGB_a[1] + 1.05 * RGB_a[2] + 0.305));

    alpha = pow(t, 0.9) * pow(1.64 - pow(0.29, p->n), 0.73);

    pOut->C = alpha * J_root;                               // chroma
    pOut->M = pOut->C * p->FL4;                             // colorfulness
    pOut->s = 50 * sqrt(p->c * alpha / (p->A_w + 4));       // saturation
    
}



void CMSEXPORT cmsCIECAM16Reverse(cmsHANDLE hModel, cmsUInt32Number flags, const cmsCIECAM16Color* pIn, cmsCIEXYZ* pOut)
{
    cmsCIECAM16* p = (cmsCIECAM16*)hModel;

    cmsFloat64Number h_rad, sin_h, cos_h;
    cmsFloat64Number J_root, alpha, t, e_t, A, p_1, p_2, r, a, b, denom;
    cmsFloat64Number RGB_c[3], RGB_f[3];

    h_rad = radians(pIn->h);
    sin_h = sin(h_rad);
    cos_h = cos(h_rad);

    if (flags & cmsCAM16_FROM_Q)
    {
        if (pIn->Q == 0.0)
        {
            pOut->X = pOut->Y = pOut->Z = 0.0;
            return;
        }

        J_root = 0.25 * p->c * pIn->Q / ((p->A_w + 4) * p->FL4);
    }
    else
    {
        if (pIn->J == 0.0)
        {
            pOut->X = pOut->Y = pOut->Z = 0.0;
            return;
        }

        J_root = sqrt(pIn->J) * 0.1;
    }


    if (flags & cmsCAM16_FROM_s)
    {
        alpha = 0.0004 * pIn->s * pIn->s * (p->A_w + 4.0) / p->c;
    }
    else
        if (flags & cmsCAM16_FROM_M)
        {
            alpha = (pIn->M / p->FL4) / J_root;
        }
        else
        {
            alpha = pIn->C / J_root;
        }


    t = pow(alpha * pow(1.64 - pow(0.29, p->n), -0.73), 10.0 / 9.0);

    e_t = 0.25 * (cos(h_rad + 2) + 3.8);

    A = p->A_w * pow(J_root, 2.0 / p->c / p->z);

    p_1 = 5e4 / 13.0 * p->Nc * p->Ncb * e_t;

    p_2 = A / p->Nbb;

    r = 23 * (p_2 + 0.305) * t / (23 * p_1 + t * (11 * cos_h + 108 * sin_h));
    a = r * cos_h;
    b = r * sin_h;

    denom = 1.0 / 1403.0;

    RGB_c[0] = unadapt(p->FL, (460 * p_2 + 451 * a + 288 * b) * denom);
    RGB_c[1] = unadapt(p->FL, (460 * p_2 - 891 * a - 261 * b) * denom);
    RGB_c[2] = unadapt(p->FL, (460 * p_2 - 220 * a - 6300 * b) * denom);

    elem_mul(RGB_f, p->D_RGB_inv, RGB_c);

    M16_inv(pOut, RGB_f);

}


void CMSEXPORT cmsCIECAM16Done(cmsHANDLE hModel)
{
    cmsCIECAM16* lpMod = (cmsCIECAM16*)hModel;

    if (lpMod) _cmsFree(lpMod->ContextID, lpMod);
}


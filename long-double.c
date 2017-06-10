/* See LICENSE file for copyright and license details. */
#include "libcolour.h"

#include <alloca.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define libcolour_colour_t     libcolour_colour_llf_t
#define libcolour_rgb_t        libcolour_rgb_llf_t
#define libcolour_srgb_t       libcolour_srgb_llf_t
#define libcolour_ciexyy_t     libcolour_ciexyy_llf_t
#define libcolour_ciexyz_t     libcolour_ciexyz_llf_t
#define libcolour_cielab_t     libcolour_cielab_llf_t
#define libcolour_cieluv_t     libcolour_cieluv_llf_t
#define libcolour_cielchuv_t   libcolour_cielchuv_llf_t
#define libcolour_yiq_t        libcolour_yiq_llf_t
#define libcolour_ydbdr_t      libcolour_ydbdr_llf_t
#define libcolour_yuv_t        libcolour_yuv_llf_t
#define libcolour_ypbpr_t      libcolour_ypbpr_llf_t
#define libcolour_ycgco_t      libcolour_ycgco_llf_t
#define libcolour_cie1960ucs_t libcolour_cie1960ucs_llf_t
#define libcolour_cieuvw_t     libcolour_cieuvw_llf_t

#define TYPE long double
#define LIST_MODELS(X) LIBCOLOUR_LIST_MODELS(X, llf)
#define D(X) ((TYPE)(X##L))

#define LIBCOLOUR_ILLUMINANT_A   LIBCOLOUR_ILLUMINANT_A_LLF
#define LIBCOLOUR_ILLUMINANT_B   LIBCOLOUR_ILLUMINANT_B_LLF
#define LIBCOLOUR_ILLUMINANT_C   LIBCOLOUR_ILLUMINANT_C_LLF
#define LIBCOLOUR_ILLUMINANT_D50 LIBCOLOUR_ILLUMINANT_D50_LLF
#define LIBCOLOUR_ILLUMINANT_D55 LIBCOLOUR_ILLUMINANT_D55_LLF
#define LIBCOLOUR_ILLUMINANT_D65 LIBCOLOUR_ILLUMINANT_D65_LLF
#define LIBCOLOUR_ILLUMINANT_D75 LIBCOLOUR_ILLUMINANT_D75_LLF
#define LIBCOLOUR_ILLUMINANT_E   LIBCOLOUR_ILLUMINANT_E_LLF
#define LIBCOLOUR_ILLUMINANT_F1  LIBCOLOUR_ILLUMINANT_F1_LLF
#define LIBCOLOUR_ILLUMINANT_F2  LIBCOLOUR_ILLUMINANT_F2_LLF
#define LIBCOLOUR_ILLUMINANT_F3  LIBCOLOUR_ILLUMINANT_F3_LLF
#define LIBCOLOUR_ILLUMINANT_F4  LIBCOLOUR_ILLUMINANT_F4_LLF
#define LIBCOLOUR_ILLUMINANT_F5  LIBCOLOUR_ILLUMINANT_F5_LLF
#define LIBCOLOUR_ILLUMINANT_F6  LIBCOLOUR_ILLUMINANT_F6_LLF
#define LIBCOLOUR_ILLUMINANT_F7  LIBCOLOUR_ILLUMINANT_F7_LLF
#define LIBCOLOUR_ILLUMINANT_F8  LIBCOLOUR_ILLUMINANT_F8_LLF
#define LIBCOLOUR_ILLUMINANT_F9  LIBCOLOUR_ILLUMINANT_F9_LLF
#define LIBCOLOUR_ILLUMINANT_F10 LIBCOLOUR_ILLUMINANT_F10_LLF
#define LIBCOLOUR_ILLUMINANT_F11 LIBCOLOUR_ILLUMINANT_F11_LLF
#define LIBCOLOUR_ILLUMINANT_F12 LIBCOLOUR_ILLUMINANT_F12_LLF0

#define xisinf(X)    (isinfl(X))
#define xisnan(X)    (isnanl(X))
#define xpow(X, Y)   (powl((X), (Y)))
#define xcbrt(X)     (cbrtl(X))
#define xsqrt(X)     (sqrtl(X))
#define xlog(X)      (logl(X))
#define xexp(X)      (expl(X))
#define xsin(X)      (sinl(X))
#define xcos(X)      (cosl(X))
#define xatan2(X, Y) (atan2l((X), (Y)))

#undef libcolour_convert
#undef libcolour_srgb_encode
#undef libcolour_srgb_decode
#undef libcolour_delta_e
#undef libcolour_proper
#undef libcolour_get_rgb_colour_space
#undef libcolour_marshal
#undef libcolour_unmarshal
#undef libcolour_convert_en_masse

#define libcolour_convert              libcolour_convert_llf
#define libcolour_srgb_encode          libcolour_srgb_encode_llf
#define libcolour_srgb_decode          libcolour_srgb_decode_llf
#define libcolour_delta_e              libcolour_delta_e_llf
#define libcolour_proper               libcolour_proper_llf
#define libcolour_get_rgb_colour_space libcolour_get_rgb_colour_space_llf
#define libcolour_marshal              libcolour_marshal_llf
#define libcolour_unmarshal            libcolour_unmarshal_llf
#define libcolour_convert_en_masse     libcolour_convert_en_masse_llf

#include "common.h"
#include "convert-template.c"
#include "libcolour-template.c"
#include "en_masse-template.c"

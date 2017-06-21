/* See LICENSE file for copyright and license details. */

#define MATRIX(M11, M12, M13, M21, M22, M23, M31, M32, M33)\
	D(M11), D(M12), D(M13),\
	D(M21), D(M22), D(M23),\
	D(M31), D(M32), D(M33)



#define CIEXYZ_TO_RGB(TO_MINV)\
	(TO_MINV)[0][0], (TO_MINV)[0][1], (TO_MINV)[0][2],\
	(TO_MINV)[1][0], (TO_MINV)[1][1], (TO_MINV)[1][2],\
	(TO_MINV)[2][0], (TO_MINV)[2][1], (TO_MINV)[2][2]

#define YPBPR_TO_SRGB(FROM_Y, FROM_PB, FROM_PR, TO_R, TO_G, TO_B)\
	do {\
		TYPE y__ = (FROM_Y);\
		TYPE r__ = (FROM_PR) + y__;\
		TYPE b__ = (FROM_PB) + y__;\
		(TO_R) = r__;\
		(TO_B) = b__;\
		(TO_G) = (y__ - r__ * D(0.2126) - b__ * D(0.0722)) / D(0.7152);\
	} while (0)

#define CIEXYZ_TO_CIEXYY(FROM_X, FROM_Y, FROM_Z, TO_X, TO_Y, TO_YY)\
	do {\
		TYPE x__ = (FROM_X), y__ = (FROM_Y), z__ = (FROM_Z);\
		TYPE s__ = x__ + y__ + z__;\
		x__ = x__ / s__;\
		s__ = y__ / s__;\
		if (WASDIV0(x__) || WASDIV0(s__))\
			x__ = s__ = 0;\
		(TO_X) = x__;\
		(TO_Y) = s__;\
		(TO_YY) = y__;\
	} while (0)

#define RGB_TO_CIEXYZ(FROM_M)\
	(FROM_M)[0][0], (FROM_M)[0][1], (FROM_M)[0][2],\
	(FROM_M)[1][0], (FROM_M)[1][1], (FROM_M)[1][2],\
	(FROM_M)[2][0], (FROM_M)[2][1], (FROM_M)[2][2]

#define CIEXYY_TO_CIEXYZ(FROM_X, FROM_Y, FROM_YY, TO_X, TO_Y, TO_Z)\
	do {\
		TYPE x__ = (FROM_X), y__ = (FROM_Y), Y__ = (FROM_YY);\
		TYPE Yy__ = Y__ / y__;\
		if (WASDIV0(Yy__)) {\
			(TO_X) = (TO_Y) = (TO_Z) = Y__;\
		} else {\
			(TO_X) = x__ * Yy__;\
			(TO_Y) = Y__;\
			(TO_Z) = (1 - x__ - y__) * Yy__;\
		}\
	} while (0)

#define CIELAB_TO_CIEXYZ(FROM_L, FROM_A, FROM_B, TO_X, TO_Y, TO_Z)\
	do {\
		TYPE Y__ = ((FROM_L) + 16) / 116;\
		TYPE X__ = Y__ + (FROM_A) / 500;\
		TYPE Z__ = Y__ - (FROM_B) / 200;\
		(TO_X) = cielab_finv(X__) * D(0.95047);\
		(TO_Y) = cielab_finv(Y__);\
		(TO_Z) = cielab_finv(Z__) * D(1.08883);\
	} while (0)

#define CIELUV_TO_CIEXYZ(FROM_L, FROM_U, FROM_V, TO_X, TO_Y, TO_Z, FROM_WHITE_X, FROM_WHITE_Y, FROM_WHITE_Z)\
	do {\
		TYPE L__ = (FROM_L), X__ = (FROM_WHITE_X), Y__ = (FROM_WHITE_Y), Z__ = (FROM_WHITE_Z);\
		TYPE L13__ = L__ * 13;\
		TYPE t__ = X__ + 15 * Y__ + 3 * Z__;\
		TYPE u__ = (FROM_U) / L13__ + 4 * X__ / t__;\
		TYPE v__ = (FROM_V) / L13__ + 9 * Y__ / t__;\
		if (L__ <= 8) {\
			Y__ *= L__ * 27 / 24389;\
		} else {\
			L__ = (L__ + 16) / 116;\
			Y__ *= L__ * L__ * L__;\
		}\
		(TO_X) = D(2.25) * Y__ * u__ / v__;\
		(TO_Y) = Y__;\
		(TO_Z) = Y__ * (3 / v__ - D(0.75) * u__ / v__ - 5);\
	} while (0)

#define CIE1960UCS_TO_CIEXYZ(FROM_U, FROM_V, FROM_Y, TO_X, TO_Y, TO_Z)\
	do {\
		TYPE u__ = (FROM_U), v__ = (FROM_V), Y__ = (FROM_Y);\
		(TO_X) = D(1.5) * Y__ * u__ / v__;\
		(TO_Y) = Y__;\
		(TO_Z) = (4 * Y__ - Y__ * u__ - 10 * Y__ * v__) / (2 * v__);\
	} while (0)

#define CIEXYZ_TO_CIELAB(FROM_X, FROM_Y, FROM_Z, TO_L, TO_A, TO_B)\
	do {\
		TYPE X__ = (FROM_X), Y__ = (FROM_Y), Z__ = (FROM_Z);\
		X__ /= D(0.95047);\
		Z__ /= D(1.08883);\
		Y__ = cielab_f(Y__);\
		(TO_L) = 116 * Y__ - 16;\
		(TO_A) = 500 * (cielab_f(X__) - Y__);\
		(TO_B) = 200 * (Y__ - cielab_f(Z__));\
	} while (0)

#define CIEXYZ_TO_CIELUV(FROM_X, FROM_Y, FROM_Z, TO_L, TO_U, TO_V, TO_WHITE_X, TO_WHITE_Y, TO_WHITE_Z)\
	do {\
		TYPE WX__ = (TO_WHITE_X), WY__ = (TO_WHITE_Y), WZ__ = (TO_WHITE_Z);\
		TYPE X__ = (FROM_X), Y__ = (FROM_Y), Z__ = (FROM_Z);\
		TYPE L2__, L__, u__, v__, t__;\
		t__ = X__ + 15 * Y__ + 3 * Z__;\
		u__ = 4 * X__ / t__;\
		v__ = 9 * Y__ / t__;\
		t__ = WX__ + 15 * WY__ + 3 * WZ__;\
		u__ -= 4 * WX__ / t__;\
		v__ -= 9 * WY__ / t__;\
		L__ = Y__ / WY__;\
		L2__ = L__ * 24389;\
		L__ = L2__ <= 216 ? L2__ / 27 : xcbrt(L__) * 116 - 16;\
		(TO_L) = L__;\
		L__ *= 13;\
		(TO_U) = u__ * L__;\
		(TO_V) = v__ * L__;\
	} while (0)

#define CIELCHUV_TO_CIELUV(FROM_L, FROM_C, FROM_H, TO_L, TO_U, TO_V)\
	do {\
		TYPE C__ = (FROM_C), h__ = (FROM_H);\
		(TO_L) = (FROM_L);\
		(TO_U) = C__ * xcos(h__);\
		(TO_V) = C__ * xsin(h__);\
	} while (0)

#define CIELUV_TO_CIELCHUV(FROM_L, FROM_U, FROM_V, TO_L, TO_C, TO_H, TO_ONE_REVOLUTION)\
	do {\
		TYPE u__ = (FROM_U), v__ = (FROM_V), rev__ = (TO_ONE_REVOLUTION), h__;\
		(TO_L) = (FROM_L);\
		(TO_C) = xsqrt(u__ * u__ + v__ * v__);\
		h__ = xatan2(v__, u__) / PI2 * rev__;\
		if (!WASDIV0(h__) && (h__ < 0))\
			h__ += rev__;\
		(TO_H) = h__;\
	} while (0)

#define YUV_TO_YDBDR(FROM_Y, FROM_U, FROM_V, TO_Y, TO_DB, TO_DR)\
	do {\
		TYPE U__ = (FROM_U), V__ = (FROM_V);\
		(TO_Y)  = (FROM_Y);\
		(TO_DB) = U__ *  D(3.069);\
		(TO_DR) = V__ * D(-2.169);\
	} while (0)

#define YDBDR_TO_YUV(FROM_Y, FROM_DB, FROM_DR, TO_Y, TO_U, TO_V)\
	do {\
		TYPE DB__ = (FROM_DB), DR__ = (FROM_DR);\
		(TO_Y) = (FROM_Y);\
		(TO_U) = DB__ /  D(3.069);\
		(TO_V) = DR__ / D(-2.169);\
	} while (0)

#define SRGB_TO_YPBPR(FROM_R, FROM_G, FROM_B, TO_Y, TO_PB, TO_PR)\
	do {\
		TYPE R__ = (FROM_R), G__ = (FROM_G), B__ = (FROM_B);\
		TYPE Y__ = R__ * D(0.2126) +\
		           G__ * D(0.7152) +\
		           B__ * D(0.0722);\
		(TO_Y)  = Y__;\
		(TO_PB) = B__ - Y__;\
		(TO_PR) = R__ - Y__;\
	 } while (0)

#define CIEUVW_TO_CIE1960UCS(FROM_U, FROM_V, FROM_W, TO_U, TO_V, TO_Y, FROM_U0, FROM_V0)\
	do {\
		TYPE U__ = (FROM_U), V__ = (FROM_V), W__ = (FROM_W), Y__;\
		Y__ = (W__ + 17) / 25;\
		Y__ *= Y__ * Y__;\
		W__ *= 13;\
		(TO_U) = U__ / W__ + (FROM_U0);\
		(TO_V) = V__ / W__ + (FROM_V0);\
		(TO_Y) = Y__;\
	} while (0)

#define CIEXYZ_TO_CIE1960UCS(FROM_X, FROM_Y, FROM_Z, TO_U, TO_V, TO_Y)\
	do {\
		TYPE X__ = (FROM_X), Y__ = (FROM_Y), u__, v__;\
		v__ = X__ + 15 * Y__ + 3 * (FROM_Z);\
		u__ = 4 * X__ / v__;\
		v__ = 6 * Y__ / v__;\
		if (WASDIV0(u__) || WASDIV0(v__)) {\
			u__ = D(0.19783997438029748971999310924729797989130020141602);\
			v__ = D(0.31222425446887241973215054713364224880933761596680);\
		}\
		(TO_U) = u__;\
		(TO_V) = v__;\
		(TO_Y) = Y__;\
	} while (0)

#define CIEUVW_TO_CIEUVW(FROM_U, FROM_V, FROM_W, TO_U, TO_V, TO_W, FROM_U0, FROM_V0, TO_U0, TO_V0)\
	do {\
		TYPE U__ = (FROM_U), V__ = (FROM_V), W__ = (FROM_W);\
		TYPE w__ = W__ * 13;\
		U__ += w__ * ((FROM_U0) - (TO_U0));\
		V__ += w__ * ((FROM_V0) - (TO_V0));\
		(TO_U) = U__;\
		(TO_V) = V__;\
		(TO_W) = W__;\
	} while (0)

#define CIE1960UCS_TO_CIEUVW(FROM_U, FROM_V, FROM_Y, TO_U, TO_V, TO_W, TO_U0, TO_V0)\
	do {\
		TYPE U__ = (FROM_U), V__ = (FROM_V), u0__ = (TO_U0), v0__ = (TO_V0);\
		TYPE Y__ = 25 * xcbrt((FROM_Y)) - 17;\
		TYPE w__ = Y__ * 13;\
		(TO_U) = w__ * (U__ - u0__);\
		(TO_V) = w__ * (V__ - v0__);\
		(TO_W) = Y__;\
	} while (0)

#include "conversion-matrices.h"

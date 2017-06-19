/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "libcolour.h"


#define TYPE_COLOUR libcolour_colour_llf_t
#define TYPE_PRINTF ".10Lf"
#define TYPE_STRTOD strtold


const char *argv0;


static void
get_colour_space(char ***argvp, TYPE_COLOUR *cs)
{
#define argv (*argvp)

	if (!strcasecmp(*argv, "srgb")) {
		argv++;
		cs->model = LIBCOLOUR_SRGB;
		if (!*argv || !**argv) exit(2);
		cs->srgb.with_transfer = !!strchr("yYtT", **argv);
		argv++;
	} else if (!strcasecmp(*argv, "xyy") || !strcasecmp(*argv, "ciexyy")) {
		argv++;
		cs->model = LIBCOLOUR_CIEXYY;
	} else if (!strcasecmp(*argv, "xyz") || !strcasecmp(*argv, "ciexyz")) {
		argv++;
		cs->model = LIBCOLOUR_CIEXYZ;
	} else if (!strcasecmp(*argv, "lab") || !strcasecmp(*argv, "cielab")) {
		argv++;
		cs->model = LIBCOLOUR_CIELAB;
	} else if (!strcasecmp(*argv, "yiq")) {
		argv++;
		cs->model = LIBCOLOUR_YIQ;
	} else if (!strcasecmp(*argv, "ydbdr")) {
		argv++;
		cs->model = LIBCOLOUR_YDBDR;
	} else if (!strcasecmp(*argv, "yuv")) {
		argv++;
		cs->model = LIBCOLOUR_YUV;
	} else if (!strcasecmp(*argv, "ypbpr")) {
		argv++;
		cs->model = LIBCOLOUR_YPBPR;
	} else if (!strcasecmp(*argv, "ycgco")) {
		argv++;
		cs->model = LIBCOLOUR_YCGCO;
	} else if (!strcasecmp(*argv, "cie1960ucs") || !strcasecmp(*argv, "cieucs") || !strcasecmp(*argv, "ucs")) {
		argv++;
		cs->model = LIBCOLOUR_CIE1960UCS;
	} else if (!strcasecmp(*argv, "cieuvw") || !strcasecmp(*argv, "uvw")) {
		argv++;
		cs->model = LIBCOLOUR_CIEUVW;
		if (!*argv) exit(2);
		cs->cieuvw.u0 = TYPE_STRTOD(*argv++, NULL);
		if (!*argv) exit(2);
		cs->cieuvw.v0 = TYPE_STRTOD(*argv++, NULL);
	} else if (!strcasecmp(*argv, "cieluv") || !strcasecmp(*argv, "luv")) {
		argv++;
		cs->model = LIBCOLOUR_CIELUV;
		cs->cieluv.white.model = LIBCOLOUR_CIEXYZ;
		if (!*argv) exit(2);
		cs->cieluv.white.X = TYPE_STRTOD(*argv++, NULL);
		if (!*argv) exit(2);
		cs->cieluv.white.Y = TYPE_STRTOD(*argv++, NULL);
		if (!*argv) exit(2);
		cs->cieluv.white.Z = TYPE_STRTOD(*argv++, NULL);
	} else if (!strcasecmp(*argv, "cielchuv") || !strcasecmp(*argv, "lchuv")) {
		argv++;
		cs->model = LIBCOLOUR_CIELCHUV;
		cs->cielchuv.white.model = LIBCOLOUR_CIEXYZ;
		if (!*argv) exit(2);
		cs->cielchuv.white.X = TYPE_STRTOD(*argv++, NULL);
		if (!*argv) exit(2);
		cs->cielchuv.white.Y = TYPE_STRTOD(*argv++, NULL);
		if (!*argv) exit(2);
		cs->cielchuv.white.Z = TYPE_STRTOD(*argv++, NULL);
		if (!*argv) exit(2);
		cs->cielchuv.one_revolution = TYPE_STRTOD(*argv++, NULL);
	} else if (!strcasecmp(*argv, "yes")) {
		argv++;
		cs->model = LIBCOLOUR_YES;
	}

#undef argv
}


int
main(int argc, char *argv[])
{
	TYPE_COLOUR from, to;

	if (!*argv) return 2;
	argv0 = *argv++;

	if (!*argv) return 2;
	get_colour_space(&argv, &from);

	if (!*argv) return 2;
	from.srgb.R = TYPE_STRTOD(*argv++, NULL);
	if (!*argv) return 2;
	from.srgb.G = TYPE_STRTOD(*argv++, NULL);
	if (!*argv) return 2;
	from.srgb.B = TYPE_STRTOD(*argv++, NULL);

	if (!*argv) return 2;
	get_colour_space(&argv, &to);

	if (*argv) return 2;

	if (libcolour_convert(&from, &to)) return 1;

	printf("%"TYPE_PRINTF" %"TYPE_PRINTF" %"TYPE_PRINTF"\n", to.srgb.R, to.srgb.G, to.srgb.B);

	fflush(stdout);
	if (ferror(stdout))
		return 2;
	return 0;

	(void) argc;
}

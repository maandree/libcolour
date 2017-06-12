/* See LICENSE file for copyright and license details. */

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC diagnostic ignored "-Wfloat-equal"
#elif defined(__clang__)
# pragma clang diagnostic ignored "-Wcomma"
# pragma clang diagnostic ignored "-Wfloat-equal"
# pragma clang diagnostic ignored "-Wvla"
# pragma clang diagnostic ignored "-Wtautological-compare"
# pragma clang diagnostic ignored "-Wcovered-switch-default"
# pragma clang diagnostic ignored "-Wfloat-conversion"
# pragma clang diagnostic ignored "-Wconversion"
# pragma clang diagnostic ignored "-Wdouble-promotion"
# pragma clang diagnostic ignored "-Wswitch-enum"
# pragma clang diagnostic ignored "-Wcast-align"
#endif



#define PI          D(3.14159265358979323846)
#define PI2         (2 * D(3.14159265358979323846))
#define WASDIV0(X)  (xisinf(X) || xisnan(X))

#define REGULAR(S, T)    ((T) <= (S)->TRANSITION ? (S)->SLOPE * (T) : (1 + (S)->OFFSET) * xpow((T), 1 / (S)->GAMMA) - (S)->OFFSET)
#define INVREGULAR(S, T) ((T) <= (S)->TRANSITIONINV ? (T) / (S)->SLOPE : xpow(((T) + (S)->OFFSET) / (1 + (S)->OFFSET), (S)->GAMMA))


#define SLOPE transfer.regular.slope
#define TRANSITIONINV transfer.regular.transitioninv
#define TRANSITION transfer.regular.transition
#define GAMMA transfer.regular.gamma
#define OFFSET transfer.regular.offset
#define TO_ENCODED_RED transfer.custom.to_encoded_red
#define TO_DECODED_RED transfer.custom.to_decoded_red
#define TO_ENCODED_GREEN transfer.custom.to_encoded_green
#define TO_DECODED_GREEN transfer.custom.to_decoded_green
#define TO_ENCODED_BLUE transfer.custom.to_encoded_blue
#define TO_DECODED_BLUE transfer.custom.to_decoded_blue


static inline TYPE
srgb_encode(TYPE t)
{
	TYPE sign = 1;
	if (t < 0) {
		t = -t;
		sign = -1;
	}
	t = t <= D(0.0031306684425217108) ? D(12.92) * t
		: D(1.055) * xpow(t, 1 / D(2.4)) - D(0.055);
	return t * sign;
}

static inline TYPE
srgb_decode(TYPE t)
{
	TYPE sign = 1;
	if (t < 0) {
		t = -t;
		sign = -1;
	}
	t = t <= D(0.0031306684425217108) * D(12.92) ? t / D(12.92)
		: xpow((t + D(0.055)) / D(1.055), D(2.4));
	return t * sign;
}

static inline TYPE
cielab_finv(TYPE t)
{
	return (t > D(6.) / D(29.)) ? t * t * t : (t - D(4.) / D(29.)) * 108 / 841;
}

static inline TYPE
cielab_f(TYPE t)
{
	return (t > D(216.) / D(24389.)) ? xcbrt(t) : t * D(841.) / D(108.) + D(4.) / D(29.);
}

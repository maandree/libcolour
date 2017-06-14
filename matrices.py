#!/usr/bin/env python
# Both Python 2 and Python 3 will work

import sys, math

FORMAT = '%.50f'
DELIMITER = ', '

def _divrow(R, d):
    return [c / d for c in R]
    
def _subrow(A, B, m):
    return [a - b * m for a, b in zip(A, B)]
    
def transpose(M):
    r, c = len(M), len(M[0])
    return [[M[y][x] for y in range(r)] for x in range(c)]

def to_vector(M):
    if len(M[0]) == 1:
        return tuple(R[0] for R in M)
    elif len(M) == 1:
        return M[0]
    raise Exception('Invalid arguments')

def _multiply(A, B):
    if isinstance(A[0], int) or isinstance(A[0], float):
        A = [A]
    if isinstance(B[0], int) or isinstance(B[0], float):
        B = [B]
    ar, ac, br, bc = len(A), len(A[0]), len(B), len(B[0])
    if ac != br:
        if ac == bc:
            B, br, bc = transpose(B), bc, br
        elif ar == br:
            A, ar, ac = transpose(A), ac, ar
        else:
            raise Exception('Invalid arguments')
    return [[sum(A[r][i] * B[i][c] for i in range(ac)) for c in range(bc)] for r in range(ar)]
    
def multiply(*Ms):
    R, Ms = Ms[-1], list(Ms)[-2::-1]
    for M in Ms:
        R = _multiply(M, R)
    return R

def invert(M):
    r, c = len(M), len(M[0])
    if r != c:
        raise Exception('Invalid arguments')
    I = [[1 if x == y else 0 for x in range(c)] for y in range(r)]
    M = [MR + IR for MR, IR in zip(M, I)]
    for r1 in range(r):
        if M[r1][r1] == 0:
            for r2 in range(r1 + 1, r):
                if M[r2][r1] != 0:
                    break
            if r2 == r:
                raise Exception('Not invertable')
            M[r1], M[r2] = M[r2], M[r1]
        M[r1] = _divrow(M[r1], M[r1][r1])
        for r2 in range(r1 + 1, r):
            M[r2] = _subrow(M[r2], M[r1], M[r2][r1])
    for r1 in reversed(range(r)):
        for r2 in reversed(range(r1)):
            M[r2] = _subrow(M[r2], M[r1], M[r2][r1])
    return [R[c:] for R in M]

def str_matrix(M):
    rfmt = '%s%s%s%s%s' % (FORMAT, DELIMITER, FORMAT, DELIMITER, FORMAT)
    fmt = '%s\n%s\n%s' % (rfmt, rfmt, rfmt)
    return fmt % tuple(M[0] + M[1] +M[2])

def print_matrix(M):
    print(str_matrix(M))

def matrix(m11, m12, m13, m21, m22, m23, m31, m32, m33):
    return [[m11, m12, m13], [m21, m22, m23], [m31, m32, m33]]

def _cos(deg):
    return math.cos(deg * math.pi / 180.)

def _sin(deg):
    return math.sin(deg * math.pi / 180.)

def _yiq_i(k, a, b):
    return 0.877 * _cos(33.) * (a - k) - (0.492 * _sin(33.)) * (b - k)

def _yiq_q(k, a, b):
    return 0.877 * _sin(33.) * (a - k) + (0.492 * _cos(33.)) * (b - k)


matrices = {
    ('srgb', 'yes') : matrix(0.253,  0.684,  0.063,
                             0.50,  -0.50,   0.,
                             0.25,   0.25,  -0.50),

    ('ycgco', 'srgb') : matrix(1., -1.,  1.,
                               1.,  1.,  0.,
                               1., -1., -1.),

    ('srgb', 'yiq') : matrix(0.299, 0.587, 0.114,
                             _yiq_i(0.299, 1, 0), _yiq_i(0.587, 0, 0), _yiq_i(0.114, 0, 1),
                             _yiq_q(0.299, 1, 0), _yiq_q(0.587, 0, 0), _yiq_q(0.114, 0, 1)),

    ('srgb', 'ydbdr') : matrix( 0.299,  0.587, 0.114,
                               -0.450, -0.883, 1.333,
                               -1.333,  1.116, 0.217),

    ('srgb', 'ypbpr') : matrix(    0.2126,  0.7152,     0.0722,
                                  -0.2126, -0.7152, 1 - 0.0722,
                               1 - 0.2126, -0.7152,    -0.0722),

    ('srgb', 'ciexyz') : matrix(0.412457445582367576708548995157,
                                0.357575865245515878143578447634,
                                0.180437247826399665973085006954,
                                0.212673370378408277403536885686,
                                0.715151730491031756287156895269,
                                0.072174899130559869164791564344,
                                0.019333942761673460208893260415,
                                0.119191955081838593666354597644,
                                0.950302838552371742508739771438),

    ('yuv', 'ydbdr') : matrix(1., 0.,     0.,
                              0., 3.069,  0.,
                              0., 0.,    -2.169),
}

if __name__ == '__main__':
    models = set()
    for (f, t) in list(matrices.keys()):
        models.add(f)
        models.add(t)
        if (t, f) not in matrices:
            matrices[(t, f)] = invert(matrices[(f, t)])

    while True:
        ms = list(matrices.keys())
        added = 0
        for m1 in models:
            for m2 in models:
                if m1 == m2:
                    continue
                for m3 in models:
                    if m1 == m3 or m2 == m3:
                        continue
                    if (m1, m3) in ms:
                        continue
                    if (m1, m2) not in ms:
                        continue
                    if (m2, m3) not in ms:
                        continue
                    added += 1
                    m = multiply(matrices[(m2, m3)], matrices[(m1, m2)])
                    matrices[(m1, m3)] = m
        if not added:
            break

    for (f, t) in sorted(matrices.keys()):
        M = multiply(matrices[(t, f)], matrices[(f, t)])
        for r in range(len(M)):
            for c in range(len(M[r])):
                x = M[r][c]
                x -= 1 if r == c else 0
                x *= x
                if x > 0.0001:
                    sys.stderr.write('(%s, %s)(%s, %s) != I\n' % (t, f, f, t))
                    for r in M:
                        for c in r:
                            sys.stderr.write('\t%lg' % c)
                        sys.stderr.write('\n')
                    sys.exit(1)

    for (f, t) in sorted(matrices.keys()):
        M = matrices[(f, t)]
        macro = '%s_TO_%s' % (f.upper(), t.upper())
        print('')
        print('#ifndef %s' % macro)
        print('# define %s MATRIX(\\' % macro)
        for row in M:
            a = ', '.join([FORMAT] * len(row))
            b = ')' if row is M[-1] else ',\\'
            print(('\t' + a + b) % tuple(row))
        print('#endif')

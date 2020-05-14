import json
import argparse
import math
from Crypto.Util import number

#
cache_lines_38_26 = [
    [52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25],
    [26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51]
]

# Rust ideal configuration
cache_lines = [
    [62, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19],
    [20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51]
]

def getModInverse(a, m):
    """ Computes the modular inverse of a mod m using the Euclidean algorithm."""
    if math.gcd(a, m) != 1:
        return None
    u1, u2, u3 = 1, 0, a
    v1, v2, v3 = 0, 1, m

    while v3 != 0:
        q = u3 // v3
        v1, v2, v3, u1, u2, u3 = (u1 - q * v1), (u2 - q * v2), (u3 - q * v3), v1, v2, v3
    return u1 % m


def constr_encoding(x):
    """ Constructs the cacheline-encoding of x. The least significant block is first.

    First, add 0 to the beginning so that the length is divisible by 6.
    Then, check for each 6-Bit block whether it is in line1 (encoded by 0) or in line2 (encoded by 1).
    At the end, reverse the ordering, so that the least significant block is first.
    """
    s = bin(x)[2:]
    while len(s) % 6 != 0:
        s = "0" + s
    l = int(len(s) / 6)
    code = []
    for i in range(0, l):
        y = int(s[i * 6:(i + 1) * 6], 2)
        for j in range(0, len(cache_lines)):
            if y in cache_lines[j]:
                code.append(j)
    code.reverse()
    return code


def generate_rsa_params(bit_length):
    prime_length = int(bit_length / 2)
    max_prime_val = 2**(prime_length) - 1

    e = 65537

    while True:
        while True:
            p = number.getPrime(prime_length)
            if p % e != 1 and p < max_prime_val:
                break

        while True:
            q = number.getPrime(prime_length)
            if q % e != 1 and q < max_prime_val:
                break

        n = p * q
        if len(bin(n)[2:]) == bit_length:
            break

    phi = (p - 1) * (q - 1)
    d = getModInverse(e, phi)
    dp = getModInverse(e, p - 1)
    dq = getModInverse(e, q - 1)

    return {'n': n, 'e': e, 'p': p, 'q': q, 'd': d, 'dp': dp, 'dq': dq}

def test(params):
    m = 37
    c = pow(m, params['e'], params['n'])
    m_d = pow(c, params['d'], params['n'])

    return m == m_d

def main(args):

    bit_length = args.length
    params = generate_rsa_params(bit_length)

    enc_params = {}
    params_length = {}
    for key, value in params.items():
        enc_params['enc_' + key] = constr_encoding(value)
        params_length['length_' + key] = len(bin(value)[2:])

    params.update(enc_params)
    params.update(params_length)

    params['line1'] = cache_lines[0]
    params['line2'] = cache_lines[1]

    for key, value in params.items():
        print(key + ': ' + str(value))

    print("p % e: " + str(params['p'] % params['e']))
    print("q % e: " + str(params['q'] % params['e']))

    print('Test was: ' + str(test(params)))

    if args.out:
        out = args.out
    else:
        out = 'example_' + str(bit_length) + '.json'

    with open(out, 'w') as f:
        json.dump(params, f)


if __name__ == '__main__':
    default_length = 128
    arg_parser = argparse.ArgumentParser(description='Construct RSA Parameters')
    arg_parser.add_argument('--length', nargs='?', const=default_length,
                            default=default_length, type=int)
    arg_parser.add_argument('--out', nargs='?', type=str)
    args = arg_parser.parse_args()
    main(args)
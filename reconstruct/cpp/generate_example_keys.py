import json
import argparse
import math
from Crypto.Util import number
import random

cache_lines_38_26 = [
    [52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25],
    [26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51]
]

# Rust ideal configuration
cache_lines = [
    [62, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19],
    [20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51]
]

memjam_8_byte_partitions = [
    [62, 63], # 0x28
    [52, 53, 54, 55, 56, 57, 58, 59], # 0x30
    [60, 61], # 0x38
    [0, 1, 2, 3, 4, 5, 6], # 0x40
    [7, 8, 9, 10, 11, 12, 13, 14], # 0x48
    [15, 16, 17, 18, 19, 20, 21, 22], # 0x50
    [23, 24, 25], # 0x58
    [26, 27, 28, 29, 30, 31, 32], # 0x60
    [33, 34, 35, 36, 37, 38, 39, 40], # 0x68
    [41, 42, 43, 44, 45, 46, 47, 48], # 0x70
    [49, 50, 51] # 0x78
]

error_probability = 0.2
INVALID_OBSERVATION = 0xFF

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


def constr_encoding(x, partitions, simulate_missing):
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
        for j in range(0, len(partitions)):
            if y in partitions[j]:
                r = random.uniform(0, 1)
                if simulate_missing and r <= error_probability:
                    code.append(INVALID_OBSERVATION)
                else:
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
    partitions = memjam_8_byte_partitions
    simulate_missing = False
    if args.simulate_missing:
        simulate_missing = True
    bit_length = args.length
    params = generate_rsa_params(bit_length)

    enc_params = {}
    params_length = {}
    for key, value in params.items():
        enc_params['enc_' + key] = constr_encoding(value, partitions, simulate_missing)
        params_length['length_' + key] = len(bin(value)[2:])

    params.update(enc_params)
    params.update(params_length)

    line_nb = 1
    for part in partitions:
        params['line' + str(line_nb)] = part
        line_nb = line_nb + 1

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
    arg_parser.add_argument('--simulate_missing', action='store_true')
    args = arg_parser.parse_args()
    random.seed()
    main(args)
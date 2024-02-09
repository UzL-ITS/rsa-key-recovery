import json
import argparse
import math
from Crypto.PublicKey import RSA
import random

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


def get_mod_inverse(a, m):
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
    param_length = int(len(s) / 6)
    code = []
    for i in range(0, param_length):
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


def read_rsa_params(in_file):

    with open(in_file, 'r') as key_file:
        key_string = key_file.read()

    rsa_key = RSA.importKey(key_string)
    dp = get_mod_inverse(rsa_key.e, rsa_key.p - 1)
    dq = get_mod_inverse(rsa_key.e, rsa_key.q - 1)

    return {'n': rsa_key.n, 'e': rsa_key.e, 'p': rsa_key.p, 'q': rsa_key.q, 'd': rsa_key.d, 'dp': dp, 'dq': dq}


def test(params):
    m = 37
    c = pow(m, params['e'], params['n'])
    m_d = pow(c, params['d'], params['n'])
    print("m_d: " + str(m_d))

    return m == m_d


def main(args):
    partitions = memjam_8_byte_partitions
    simulate_missing = False
    if args.simulate_missing:
        simulate_missing = True
    in_file = args.infile
    params = read_rsa_params(in_file)

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
        out = 'openssl_old_construction_' + str(in_file) + '.json'

    with open(out, 'w') as f:
        json.dump(params, f)


if __name__ == '__main__':
    default_in = 'test_key_4096bit.pem'
    arg_parser = argparse.ArgumentParser(description='Construct RSA Parameters')
    arg_parser.add_argument('--infile', nargs='?', const=default_in,
                            default=default_in, type=str)
    arg_parser.add_argument('--out', nargs='?', type=str)
    arg_parser.add_argument('--simulate_missing', action='store_true')
    args = arg_parser.parse_args()
    random.seed()
    main(args)
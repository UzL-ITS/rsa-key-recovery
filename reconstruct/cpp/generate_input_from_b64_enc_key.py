import argparse
import json
from Crypto.PublicKey import RSA
import math
import random

memjam_8_byte_partitions = [
    [62, 63],  # 0x28
    [52, 53, 54, 55, 56, 57, 58, 59],  # 0x30
    [60, 61],  # 0x38
    [0, 1, 2, 3, 4, 5, 6],  # 0x40
    [7, 8, 9, 10, 11, 12, 13, 14],  # 0x48
    [15, 16, 17, 18, 19, 20, 21, 22],  # 0x50
    [23, 24, 25],  # 0x58
    [26, 27, 28, 29, 30, 31, 32],  # 0x60
    [33, 34, 35, 36, 37, 38, 39, 40],  # 0x68
    [41, 42, 43, 44, 45, 46, 47, 48],  # 0x70
    [49, 50, 51]  # 0x78
]

memjam_8_byte_partitions_lower_4 = [
    [14, 15],
    [4, 5, 6, 7, 8, 9, 10, 11],
    [12, 13],
    [0, 1, 2, 3, 4, 5, 6],
    [7, 8, 9, 10, 11, 12, 13, 14],
    [15, 0, 1, 2, 3, 4, 5, 6],
    [7, 8, 9],
    [10, 11, 12, 13, 14, 15, 0],
    [1, 2, 3, 4, 5, 6, 7, 8],
    [9, 10, 11, 12, 13, 14, 15, 0],
    [1, 2, 3]
]

memjam_8_byte_partitions_lower_2 = [
    [2, 3],
    [0, 1, 2, 3, 0, 1, 2, 3],
    [0, 1],
    [0, 1, 2, 3, 0, 1, 2],
    [3, 0, 1, 2, 3, 0, 1, 2],
    [3, 0, 1, 2, 3, 0, 1, 2],
    [3, 0, 1],
    [2, 3, 0, 1, 2, 3, 0],
    [1, 2, 3, 0, 1, 2, 3, 0],
    [1, 2, 3, 0, 1, 2, 3, 0],
    [1, 2, 3],
]

memjam_8_byte_partitions_upper_4 = [
    [60, 60],
    [52, 52, 52, 52, 56, 56, 56, 56],
    [60, 60],
    [0, 0, 0, 0, 4, 4, 4],
    [4, 8, 8, 8, 8, 12, 12, 12],
    [12, 16, 16, 16, 16, 20, 20, 20],
    [20, 24, 24],
    [24, 24, 28, 28, 28, 28, 32],
    [32, 32, 32, 36, 36, 36, 36, 40],
    [40, 40, 40, 44, 44, 44, 44, 48],
    [48, 48, 48]
]

memjam_8_byte_partitions_upper_2 = [
    [48, 48],
    [48, 48, 48, 48, 48, 48, 48, 48],
    [48, 48],
    [0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0, 0],
    [0, 16, 16, 16, 16, 16, 16, 16],
    [16, 16, 16],
    [16, 16, 16, 16, 16, 16, 32],
    [32, 32, 32, 32, 32, 32, 32, 32],
    [32, 32, 32, 32, 32, 32, 32, 48],
    [48, 48, 48]
]

mem_jam_partitions_translation = {
    '+': 0, '/': 0,
    '0': 1, '1': 1, '2': 1, '3': 1, '4': 1, '5': 1, '6': 1, '7': 1,
    '8': 2, '9': 2,
    'A': 3, 'B': 3, 'C': 3, 'D': 3, 'E': 3, 'F': 3, 'G': 3,
    'H': 4, 'I': 4, 'J': 4, 'K': 4, 'L': 4, 'M': 4, 'N': 4, 'O': 4,
    'P': 5, 'Q': 5, 'R': 5, 'S': 5, 'T': 5, 'U': 5, 'V': 5, 'W': 5,
    'X': 6, 'Y': 6, 'Z': 6,
    'a': 7, 'b': 7, 'c': 7, 'd': 7, 'e': 7, 'f': 7, 'g': 7,
    'h': 8, 'i': 8, 'j': 8, 'k': 8, 'l': 8, 'm': 8, 'n': 8, 'o': 8,
    'p': 9, 'q': 9, 'r': 9, 's': 9, 't': 9, 'u': 9, 'v': 9, 'w': 9,
    'x': 10, 'y': 10, 'z': 10
}

default_length_2048 = {
    "length_n": 2048,
    "length_e": 17,
    "length_p": 1024,
    "length_q": 1024,
    "length_d": 2048,
    "length_dp": 1024,
    "length_dq": 1024
}

default_length_4096 = {
    "length_n": 4096,
    "length_e": 17,
    "length_p": 2048,
    "length_q": 2048,
    "length_d": 4096,
    "length_dp": 2048,
    "length_dq": 2048
}

error_probability = 0.35
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


def get_b64_start_offset_from_byte_offset(byte_start_offset):
    b64_start_offset = int(byte_start_offset / 3) * 4
    symbol_shift = 'no_shift'
    if byte_start_offset % 3 == 1:
        b64_start_offset += 2
        symbol_shift = 'upper_2'
    if byte_start_offset % 3 == 2:
        b64_start_offset += 3
        symbol_shift = 'upper_4'

    return {'b64_start_offset': b64_start_offset, 'symbol_shift': symbol_shift}


def get_b64_end_offset_from_byte_offset(byte_end_offset):
    b64_end_offset = int(byte_end_offset / 3) * 4
    symbol_shift = 'no_shift'
    if byte_end_offset % 3 == 1:
        b64_end_offset += 1
        symbol_shift = 'lower_4'
    if byte_end_offset % 3 == 2:
        b64_end_offset += 2
        symbol_shift = 'lower_2'

    return {'b64_end_offset': b64_end_offset, 'symbol_shift': symbol_shift}


def get_offset_indexes_4096bitkey(param_length):

    return {
        'offset_n': 0,
        'length_n': 0,
        'offset_e_inc': 0,
        'length_e': 0,
        'offset_d_inc': 0,
        'length_d': 1 if param_length['length_d'] == 4096 else 0,
        'offset_p_inc': 1 if param_length['length_d'] == 4096 else 0,
        'length_p': 1 if param_length['length_p'] == 2048 else 0,
        'offset_q_inc': 1 if param_length['length_p'] == 2048 else 0,
        'length_q': 1 if param_length['length_q'] == 2048 else 0,
        'offset_dp_inc': 1 if param_length['length_q'] == 2048 else 0,
        'length_dp': 1 if param_length['length_dp'] == 2048 else 0,
        'offset_dq_inc': 1 if param_length['length_dp'] == 2048 else 0,
        'length_dq': 1 if param_length['length_dq'] == 2048 else 0,
        'offset_q_inv_inc': 1 if param_length['length_dq'] == 2048 else 0,
        'length_q_inv': 0  # param unused
    }


def translate_to_partition_encoding(base64_enc_string, simulate_missing):
    partition_enc = [mem_jam_partitions_translation[x] for x in list(base64_enc_string)]
    if simulate_missing:
        for p in range(0, len(partition_enc)):
            r = random.uniform(0, 1)
            if r < error_probability:
                partition_enc[p] = INVALID_OBSERVATION

    return partition_enc


def generate_params_observation_encoding(b64_key_string, param_lenghts, simulate_missing):

    offset_indexes = get_offset_indexes_4096bitkey(param_lenghts)
    # Offsets (4096):
    #   n:      11
    #   e:      offset(n)  +  {515}
    #   d:      offset(e)  +  {7}
    #   p:      offset(d)  +  {516, 517}
    #   q:      offset(p)  +  {260, 261}
    #   dp:     offset(q)  +  {260, 261}
    #   dq:     offset(dp) +  {260, 261}
    #   q_inv:  offset(dq) +  {260, 261}
    offsets_bytes_4096bit_key = {
        # https://mbed-tls.readthedocs.io/en/latest/kb/cryptography/asn1-key-structures-in-der-and-pem/
        # 11 byte offset within key structure (7 + 4)
        # 26 byte offset due to PrivateKeyInfo Field (22 + 4)
        # Remember the Null Byte
        'offset_n': [37],
        'length_n': [513],
        'offset_e_inc': [515],
        'length_e': [3],
        'offset_d_inc': [7],
        'length_d': [512, 513],
        'offset_p_inc': [516, 517],
        'length_p': [256, 257],
        'offset_q_inc': [260, 261],
        'length_q': [256, 257],
        'offset_dp_inc': [260, 261],
        'length_dp': [256, 257],
        'offset_dq_inc': [260, 261],
        'length_dq': [256, 257],
        'offset_q_inv_inc': [260, 261],
        'length_q_inv': [256, 257]
    }

    params = {}
    current_offset = offsets_bytes_4096bit_key['offset_n'][offset_indexes['offset_n']]

    # The bytes in ASN.1 are stored in Big Endian Order
    enc_n_start_offset = get_b64_start_offset_from_byte_offset(current_offset
                                                               + offsets_bytes_4096bit_key['length_n'][offset_indexes['length_n']])
    enc_n_end_offset = get_b64_end_offset_from_byte_offset(current_offset)
    current_offset = current_offset + offsets_bytes_4096bit_key['offset_e_inc'][offset_indexes['offset_e_inc']]
    print("#####################################")
    print("n end offset: " + str(enc_n_end_offset))
    print("#####################################")
    print("#####################################")
    print("n start offset: " + str(enc_n_start_offset))
    print("#####################################")

    enc_n = b64_key_string[enc_n_end_offset['b64_end_offset']:enc_n_start_offset['b64_start_offset']]
    params['enc_n'] = translate_to_partition_encoding(enc_n, False)
    params['enc_n_start_shift'] = enc_n_start_offset['symbol_shift']
    params['enc_n_end_shift'] = enc_n_end_offset['symbol_shift']

    enc_e_start_offset = get_b64_start_offset_from_byte_offset(current_offset
                                                               + offsets_bytes_4096bit_key['length_e'][offset_indexes['length_e']])
    enc_e_end_offset = get_b64_end_offset_from_byte_offset(current_offset)
    current_offset = current_offset + offsets_bytes_4096bit_key['offset_d_inc'][offset_indexes['offset_d_inc']]
    print("#####################################")
    print("e end offset: " + str(enc_e_end_offset))
    print("#####################################")
    print("#####################################")
    print("e start offset: " + str(enc_e_start_offset))
    print("#####################################")

    enc_e = b64_key_string[enc_e_end_offset['b64_end_offset']:enc_e_start_offset['b64_start_offset']]
    params['enc_e'] = translate_to_partition_encoding(enc_e, False)
    params['enc_e_start_shift'] = enc_e_start_offset['symbol_shift']
    params['enc_e_end_shift'] = enc_e_end_offset['symbol_shift']

    enc_d_start_offset = get_b64_start_offset_from_byte_offset(
        current_offset + offsets_bytes_4096bit_key['length_d'][offset_indexes['length_d']])
    enc_d_end_offset = get_b64_end_offset_from_byte_offset(current_offset)
    print("#####################################")
    print("d end offset: " + str(enc_d_end_offset))
    print("#####################################")
    print("#####################################")
    print("d start offset: " + str(enc_d_start_offset))
    print("#####################################")
    current_offset = current_offset + offsets_bytes_4096bit_key['offset_p_inc'][offset_indexes['offset_p_inc']]

    enc_d = b64_key_string[enc_d_end_offset['b64_end_offset']:enc_d_start_offset['b64_start_offset']]
    params['enc_d'] = translate_to_partition_encoding(enc_d, simulate_missing)
    params['enc_d_start_shift'] = enc_d_start_offset['symbol_shift']
    params['enc_d_end_shift'] = enc_d_end_offset['symbol_shift']

    enc_p_start_offset = get_b64_start_offset_from_byte_offset(current_offset
                                                               + offsets_bytes_4096bit_key['length_p'][offset_indexes['length_p']])
    enc_p_end_offset = get_b64_end_offset_from_byte_offset(current_offset)
    print("#####################################")
    print("p end offset: " + str(enc_p_end_offset))
    print("#####################################")
    print("#####################################")
    print("p start offset: " + str(enc_p_start_offset))
    print("#####################################")

    current_offset = current_offset + offsets_bytes_4096bit_key['offset_q_inc'][offset_indexes['offset_q_inc']]

    enc_p = b64_key_string[enc_p_end_offset['b64_end_offset']:enc_p_start_offset['b64_start_offset']]
    params['enc_p'] = translate_to_partition_encoding(enc_p, simulate_missing)
    params['enc_p_start_shift'] = enc_p_start_offset['symbol_shift']
    params['enc_p_end_shift'] = enc_p_end_offset['symbol_shift']

    enc_q_start_offset = get_b64_start_offset_from_byte_offset(current_offset
                                                               + offsets_bytes_4096bit_key['length_q'][offset_indexes['length_q']])
    enc_q_end_offset = get_b64_end_offset_from_byte_offset(current_offset)
    print("#####################################")
    print("q end offset: " + str(enc_q_end_offset))
    print("#####################################")
    print("#####################################")
    print("q start offset: " + str(enc_q_start_offset))
    print("#####################################")

    current_offset = current_offset + offsets_bytes_4096bit_key['offset_dp_inc'][offset_indexes['offset_dp_inc']]

    enc_q = b64_key_string[enc_q_end_offset['b64_end_offset']:enc_q_start_offset['b64_start_offset']]
    params['enc_q'] = translate_to_partition_encoding(enc_q, simulate_missing)
    params['enc_q_start_shift'] = enc_q_start_offset['symbol_shift']
    params['enc_q_end_shift'] = enc_q_end_offset['symbol_shift']

    enc_dp_start_offset = get_b64_start_offset_from_byte_offset(current_offset
                                                                + offsets_bytes_4096bit_key['length_dp'][offset_indexes['length_dp']])
    enc_dp_end_offset = get_b64_end_offset_from_byte_offset(current_offset)
    print("#####################################")
    print("dp end offset: " + str(enc_dp_end_offset))
    print("#####################################")
    print("#####################################")
    print("dp start offset: " + str(enc_dp_start_offset))
    print("#####################################")

    current_offset = current_offset + offsets_bytes_4096bit_key['offset_dq_inc'][offset_indexes['offset_dq_inc']]

    enc_dp = b64_key_string[enc_dp_end_offset['b64_end_offset']:enc_dp_start_offset['b64_start_offset']]
    params['enc_dp'] = translate_to_partition_encoding(enc_dp, simulate_missing)
    params['enc_dp_start_shift'] = enc_dp_start_offset['symbol_shift']
    params['enc_dp_end_shift'] = enc_dp_end_offset['symbol_shift']

    enc_dq_start_offset = get_b64_start_offset_from_byte_offset(current_offset
                                                                + offsets_bytes_4096bit_key['length_dq'][offset_indexes['length_dq']])
    enc_dq_end_offset = get_b64_end_offset_from_byte_offset(current_offset)
    print("#####################################")
    print("dq end offset: " + str(enc_dq_end_offset))
    print("#####################################")
    print("#####################################")
    print("dq start offset: " + str(enc_dq_start_offset))
    print("#####################################")

    current_offset = current_offset + offsets_bytes_4096bit_key['offset_q_inv_inc'][offset_indexes['offset_q_inv_inc']]

    enc_dq = b64_key_string[enc_dq_end_offset['b64_end_offset']:enc_dq_start_offset['b64_start_offset']]
    params['enc_dq'] = translate_to_partition_encoding(enc_dq, simulate_missing)
    params['enc_dq_start_shift'] = enc_dq_start_offset['symbol_shift']
    params['enc_dq_end_shift'] = enc_dq_end_offset['symbol_shift']

    return params


def get_integer_param_values(b64_key_string):
    rsa_key = RSA.importKey(b64_key_string)
    dp = getModInverse(rsa_key.e, rsa_key.p - 1)
    dq = getModInverse(rsa_key.e, rsa_key.q - 1)

    print('Calculated param length: ')
    print('n: ' + str(len(bin(rsa_key.n)[2:])))
    print('e: ' + str(len(bin(rsa_key.e)[2:])))
    print('d: ' + str(len(bin(rsa_key.d)[2:])))
    print('p: ' + str(len(bin(rsa_key.p)[2:])))
    print('q: ' + str(len(bin(rsa_key.q)[2:])))
    print('dp: ' + str(len(bin(dp)[2:])))
    print('dq: ' + str(len(bin(dq)[2:])))

    return {
        'n': rsa_key.n,
        'e': rsa_key.e,
        'p': rsa_key.p,
        'q': rsa_key.q,
        'd': rsa_key.d,
        'dq': dq,
        'dp': dp
    }


def convert_integer_param_to_string(param_dict):

    return {
        'n': 'xd' + str(param_dict['n']),
        'e': 'xd' + str(param_dict['e']),
        'p': 'xd' + str(param_dict['p']),
        'q': 'xd' + str(param_dict['q']),
        'd': 'xd' + str(param_dict['d']),
        'dq': 'xd' + str(param_dict['dq']),
        'dp': 'xd' + str(param_dict['dp'])
    }


def get_assumed_length():
    # return default_length_4096
    ossl_euler_key_4096bit = {
        'length_n': 4096,
        'length_e': 17,
        'length_d': 4090,
        'length_p': 2048,
        'length_q': 2048,
        'length_dp': 2048,
        'length_dq': 2046
    }

    carmichael_validation_key_4096bit = {
        'length_n': 4096,
        'length_e': 17,
        'length_d': 4094,
        'length_p': 2048,
        'length_q': 2048,
        'length_dp': 2048,
        'length_dq': 2048
    }

    carmichael_validation_key_4096bit_2 = {
        'length_n': 4096,
        'length_e': 17,
        'length_d': 4094,
        'length_p': 2048,
        'length_q': 2048,
        'length_dp': 2046,
        'length_dq': 2047
    }

    test_key_4096bit = {
        'length_n': 4096,
        'length_e': 17,
        'length_p': 2048,
        'length_q': 2048,
        'length_d': 4095,
        'length_dp': 2048,
        'length_dq': 2048
    }

    return carmichael_validation_key_4096bit_2


def get_length(integer_params):
    return {
        'length_n': len(bin(integer_params['n'])[2:]),
        'length_e': len(bin(integer_params['e'])[2:]),
        'length_p': len(bin(integer_params['p'])[2:]),
        'length_q': len(bin(integer_params['q'])[2:]),
        'length_d': len(bin(integer_params['d'])[2:]),
        'length_dp': len(bin(integer_params['dp'])[2:]),
        'length_dq': len(bin(integer_params['dq'])[2:])
    }


def get_partitions(partitions, prefix):
    params = {}
    line_nb = 1
    for values in partitions:
        params[prefix + str(line_nb)] = values
        line_nb = line_nb + 1

    return params


def generate(b64_key_string_cleaned, b64_key_string, simulate_missing):
    partitions = memjam_8_byte_partitions

    integer_params = get_integer_param_values(b64_key_string)
    param_length = get_length(integer_params)
    params = generate_params_observation_encoding(b64_key_string_cleaned, param_length, simulate_missing)
    params.update(convert_integer_param_to_string(integer_params))
    params.update(param_length)
    params.update(get_partitions(partitions, 'line'))
    params.update(get_partitions(memjam_8_byte_partitions_lower_4, 'line_lower_4_'))
    params.update(get_partitions(memjam_8_byte_partitions_lower_2, 'line_lower_2_'))
    params.update(get_partitions(memjam_8_byte_partitions_upper_4, 'line_upper_4_'))
    params.update(get_partitions(memjam_8_byte_partitions_upper_2, 'line_upper_2_'))

    for key, value in params.items():
        print(key + ': ' + str(value))

    return params


def main(args):
    with open(args.file, 'r') as key_file:
        key_string = key_file.read()
        key_string_lines = key_string.split('\n')
        key_string_lines = key_string_lines[1:-2]

    key_string_cleaned = "".join(key_string_lines)
    print(key_string_cleaned)

    if args.simulate_missing:
        print("Simluate missing values with probability: " + str(error_probability))
        simulate_missing = True
    else:
        simulate_missing = False

    params = generate(key_string_cleaned, key_string, simulate_missing)

    if args.out:
        out = args.out
    else:
        out = 'example_' + str(args.file) + '.json'

    with open(out, 'w') as f:
        json.dump(params, f)


if __name__ == '__main__':
    default_key_filename = 'euler_keys/ossl_euler_key_4096bit.pem'
    arg_parser = argparse.ArgumentParser(description='Generate input for key reconstruction from pem key')
    arg_parser.add_argument('--file', nargs='?', const=default_key_filename,
                            default=default_key_filename, type=str)
    arg_parser.add_argument('--out', nargs='?', type=str)
    arg_parser.add_argument('--simulate_missing', action='store_true')
    arguments = arg_parser.parse_args()
    main(arguments)

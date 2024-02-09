import argparse
import json
from Crypto.PublicKey import RSA
import math

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

default_length_1024 = {
    "length_n": 1024,
    "length_e": 17,
    "length_p": 512,
    "length_q": 512,
    "length_d": 1024,
    "length_dp": 512,
    "length_dq": 512
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

INVALID_OBSERVATION = 0xFF

partition_dict = {
    "['(', ')', '*', '+', ',', '/', '-', '.']": 0,
    "['0', '1', '2', '3', '4', '5', '6', '7']": 1,
    "|['8', '9', ':', ';', '<', '=', '>', '?']|": 2,
    "['@', 'A', 'B', 'C', 'D', 'E', 'F', 'G']": 3,
    "['H', 'I', 'J', 'K', 'L', 'M', 'N', 'O']": 4,
    "['P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W']": 5,
    "['X', 'Y', 'Z', '[', '\\', ']', '^', '_']": 6,
    "['`', 'a', 'b', 'c', 'd', 'e', 'f', 'g']": 7,
    "['h', 'i', 'j', 'k', 'l', 'm', 'n', 'o']": 8,
    "['p', 'q', 'r', 's', 't', 'u', 'v', 'w']": 9,
    "|['x', 'y', 'z', '{', '||', '}', '~']|": 10
}


def convert_trace_partition_encoding(recorded_trace):
    observed_partitions = []
    count_invalid_observation = 0
    for e in recorded_trace:
        if e != '':
            obs_part = INVALID_OBSERVATION
            for key, value in partition_dict.items():
                if key == e:
                    obs_part = value
                    break
            if obs_part == INVALID_OBSERVATION:
                count_invalid_observation += 1
            observed_partitions.append(obs_part)

    print(len(observed_partitions))
    print(observed_partitions)
    print(count_invalid_observation)

    return observed_partitions


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


def generate_params_observation_encoding(observed_partitions):
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
        # Remeber the Null Byte
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

    offsets_bytes_1024bit_key = {
        'offset_n': [36],
        'length_n': [129],
        'offset_e_inc': [131],
        'length_e': [3],
        'offset_d_inc': [6],
        'length_d': [128, 129],
        'offset_p_inc': [130, 131],
        'length_p': [64, 65],
        'offset_q_inc': [66, 67],
        'length_q': [64, 65],
        'offset_dp_inc': [66, 67],
        'length_dp': [64, 65],
        'offset_dq_inc': [66, 67],
        'length_dq': [64, 65],
        'offset_q_inv_inc': [66, 67],
        'length_q_inv': [64, 65]
    }

    offsets = offsets_bytes_1024bit_key

    params = {}
    current_offset = offsets['offset_n'][0]

    # The bytes in ASN.1 are stored in Big Endian Order
    enc_n_start_offset = get_b64_start_offset_from_byte_offset(current_offset
                                                               + offsets['length_n'][0])
    enc_n_end_offset = get_b64_end_offset_from_byte_offset(current_offset)
    current_offset = current_offset + offsets['offset_e_inc'][0]
    print("#####################################")
    print("n end offset: " + str(enc_n_end_offset))
    print("#####################################")
    print("#####################################")
    print("n start offset: " + str(enc_n_start_offset))
    print("#####################################")

    enc_n = observed_partitions[enc_n_end_offset['b64_end_offset']:enc_n_start_offset['b64_start_offset']]
    params['enc_n'] = enc_n
    params['enc_n_start_shift'] = enc_n_start_offset['symbol_shift']
    params['enc_n_end_shift'] = enc_n_end_offset['symbol_shift']

    enc_e_start_offset = get_b64_start_offset_from_byte_offset(current_offset
                                                               + offsets['length_e'][0])
    enc_e_end_offset = get_b64_end_offset_from_byte_offset(current_offset)
    current_offset = current_offset + offsets['offset_d_inc'][0]
    print("#####################################")
    print("e end offset: " + str(enc_e_end_offset))
    print("#####################################")
    print("#####################################")
    print("e start offset: " + str(enc_e_start_offset))
    print("#####################################")

    enc_e = observed_partitions[enc_e_end_offset['b64_end_offset']:enc_e_start_offset['b64_start_offset']]
    params['enc_e'] = enc_e
    params['enc_e_start_shift'] = enc_e_start_offset['symbol_shift']
    params['enc_e_end_shift'] = enc_e_end_offset['symbol_shift']

    enc_d_start_offset = get_b64_start_offset_from_byte_offset(
        current_offset + offsets['length_d'][0])
    enc_d_end_offset = get_b64_end_offset_from_byte_offset(current_offset)
    print("#####################################")
    print("d end offset: " + str(enc_d_end_offset))
    print("#####################################")
    print("#####################################")
    print("d start offset: " + str(enc_d_start_offset))
    print("#####################################")
    current_offset = current_offset + offsets['offset_p_inc'][0]

    enc_d = observed_partitions[enc_d_end_offset['b64_end_offset']:enc_d_start_offset['b64_start_offset']]
    params['enc_d'] = enc_d
    params['enc_d_start_shift'] = enc_d_start_offset['symbol_shift']
    params['enc_d_end_shift'] = enc_d_end_offset['symbol_shift']

    enc_p_start_offset = get_b64_start_offset_from_byte_offset(current_offset
                                                               + offsets['length_p'][1])
    enc_p_end_offset = get_b64_end_offset_from_byte_offset(current_offset)
    print("#####################################")
    print("p end offset: " + str(enc_p_end_offset))
    print("#####################################")
    print("#####################################")
    print("p start offset: " + str(enc_p_start_offset))
    print("#####################################")

    current_offset = current_offset + offsets['offset_q_inc'][1]

    enc_p = observed_partitions[enc_p_end_offset['b64_end_offset']:enc_p_start_offset['b64_start_offset']]
    params['enc_p'] = enc_p
    params['enc_p_start_shift'] = enc_p_start_offset['symbol_shift']
    params['enc_p_end_shift'] = enc_p_end_offset['symbol_shift']

    enc_q_start_offset = get_b64_start_offset_from_byte_offset(current_offset
                                                               + offsets['length_q'][1])
    enc_q_end_offset = get_b64_end_offset_from_byte_offset(current_offset)
    print("#####################################")
    print("q end offset: " + str(enc_q_end_offset))
    print("#####################################")
    print("#####################################")
    print("q start offset: " + str(enc_q_start_offset))
    print("#####################################")

    current_offset = current_offset + offsets['offset_dp_inc'][1]

    enc_q = observed_partitions[enc_q_end_offset['b64_end_offset']:enc_q_start_offset['b64_start_offset']]
    params['enc_q'] = enc_q
    params['enc_q_start_shift'] = enc_q_start_offset['symbol_shift']
    params['enc_q_end_shift'] = enc_q_end_offset['symbol_shift']

    enc_dp_start_offset = get_b64_start_offset_from_byte_offset(current_offset
                                                                + offsets['length_dp'][0])
    enc_dp_end_offset = get_b64_end_offset_from_byte_offset(current_offset)
    print("#####################################")
    print("dp end offset: " + str(enc_dp_end_offset))
    print("#####################################")
    print("#####################################")
    print("dp start offset: " + str(enc_dp_start_offset))
    print("#####################################")

    current_offset = current_offset + offsets['offset_dq_inc'][0]

    enc_dp = observed_partitions[enc_dp_end_offset['b64_end_offset']:enc_dp_start_offset['b64_start_offset']]
    params['enc_dp'] = enc_dp
    params['enc_dp_start_shift'] = enc_dp_start_offset['symbol_shift']
    params['enc_dp_end_shift'] = enc_dp_end_offset['symbol_shift']

    enc_dq_start_offset = get_b64_start_offset_from_byte_offset(current_offset
                                                                + offsets['length_dq'][1])
    enc_dq_end_offset = get_b64_end_offset_from_byte_offset(current_offset)
    print("#####################################")
    print("dq end offset: " + str(enc_dq_end_offset))
    print("#####################################")
    print("#####################################")
    print("dq start offset: " + str(enc_dq_start_offset))
    print("#####################################")

    current_offset = current_offset + offsets['offset_q_inv_inc'][1]

    enc_dq = observed_partitions[enc_dq_end_offset['b64_end_offset']:enc_dq_start_offset['b64_start_offset']]
    params['enc_dq'] = enc_dq
    params['enc_dq_start_shift'] = enc_dq_start_offset['symbol_shift']
    params['enc_dq_end_shift'] = enc_dq_end_offset['symbol_shift']

    return params


def get_integer_param_values(b64_key_string):
    rsa_key = RSA.importKey(b64_key_string)
    dp = getModInverse(rsa_key.e, rsa_key.p - 1)
    dq = getModInverse(rsa_key.e, rsa_key.q - 1)

    print('n: ' + str(len(bin(rsa_key.n)[2:])))
    print('e: ' + str(len(bin(rsa_key.e)[2:])))
    print('d: ' + str(len(bin(rsa_key.d)[2:])))
    print('p: ' + str(len(bin(rsa_key.p)[2:])))
    print('q: ' + str(len(bin(rsa_key.q)[2:])))
    print('dp: ' + str(len(bin(dp)[2:])))
    print('dq: ' + str(len(bin(dq)[2:])))

    return {
        'n': 'xd' + str(rsa_key.n),
        'e': 'xd' + str(rsa_key.e),
        'p': 'xd' + str(rsa_key.p),
        'q': 'xd' + str(rsa_key.q),
        'd': 'xd' + str(rsa_key.d),
        'dq': 'xd' + str(dq),
        'dp': 'xd' + str(dp)
    }


def get_assumed_length():
    # return default_length_4096

    # Euler
    ossl_euler_key_4096bit = {
        'length_n': 4096,
        'length_e': 17,
        'length_d': 4090,
        'length_p': 2048,
        'length_q': 2048,
        'length_dp': 2048,
        'length_dq': 2046
    }

    # Carmichael
    test_key_4096bit = {
        'length_n': 4096,
        'length_e': 17,
        'length_p': 2048,
        'length_q': 2048,
        'length_d': 4095,
        'length_dp': 2048,
        'length_dq': 2048
    }

    # Euler
    test_key_1024bit = {
        'length_n': 1024,
        'length_e': 17,
        'length_p': 512,
        'length_q': 512,
        'length_d': 1023,
        'length_dp': 510,
        'length_dq': 512
    }

    return test_key_1024bit


def get_partitions(partitions, prefix):
    params = {}
    line_nb = 1
    for values in partitions:
        params[prefix + str(line_nb)] = values
        line_nb = line_nb + 1

    return params


def generate(observed_partitions, original_key_file):
    partitions = memjam_8_byte_partitions
    params = generate_params_observation_encoding(observed_partitions)
    params.update(get_integer_param_values(original_key_file))
    params.update(get_assumed_length())
    params.update(get_partitions(partitions, 'line'))
    params.update(get_partitions(memjam_8_byte_partitions_lower_4, 'line_lower_4_'))
    params.update(get_partitions(memjam_8_byte_partitions_lower_2, 'line_lower_2_'))
    params.update(get_partitions(memjam_8_byte_partitions_upper_4, 'line_upper_4_'))
    params.update(get_partitions(memjam_8_byte_partitions_upper_2, 'line_upper_2_'))

    for key, value in params.items():
        print(key + ': ' + str(value))

    return params


def main(args):
    with open(args.trace_file, 'r') as trace_file:
        trace_string = trace_file.read()
        trace_string_lines = trace_string.split('\n')

    with open(args.original_key_file, 'r') as orig_key_file:
        orig_key_string = orig_key_file.read()

    observed_partitions = convert_trace_partition_encoding(trace_string_lines)

    params = generate(observed_partitions, orig_key_string)

    if args.out:
        out = args.out
    else:
        out = 'example_' + str(args.original_key_file) + '.json'

    with open(out, 'w') as f:
        json.dump(params, f)


if __name__ == '__main__':
    default_key_filename = 'test_key_1024bit.pem'
    default_trace_file_name = 'sweep-10000-2022_10_07_result.csv'
    arg_parser = argparse.ArgumentParser(description='Generate input for key reconstruction from pem key')
    arg_parser.add_argument('--original_key_file', nargs='?', const=default_key_filename,
                            default=default_key_filename, type=str)
    arg_parser.add_argument('--trace_file', nargs='?', const=default_trace_file_name,
                            default=default_key_filename, type=str)
    arg_parser.add_argument('--out', nargs='?', type=str)
    arg_parser.add_argument('--simulate_missing', action='store_true')
    arguments = arg_parser.parse_args()
    main(arguments)

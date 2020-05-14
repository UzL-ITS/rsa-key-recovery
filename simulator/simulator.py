import pem

import OpenSSL.crypto as crypto
import json
import re
import argparse

data_bin2ascii = ['A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
                  'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                  'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
                  '8', '9', '+', '/']

data_ascii2bin = [
    # Cache Line 1 (here we only have to account for 0 - 9)
    # This results in these bit arrays:
    # 11 0100
    # 11 0101
    # 11 0110
    # 11 0111
    # 11 1000
    # 11 1001
    # 11 1010
    # 11 1011
    # 11 1100
    # 11 1101
    # 11 1111
    # 11 1110
    # Bits 5 & 6 are always TRUE. Bits 3 & 4 are TRUE with a probability of 2/3.
    # Maybe we can use the higher probabilities in those bits. For bits 1 & 2 the
    # probability of being true is 1/2, so no information here.
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xE0, 0xF0, 0xFF, 0xFF, 0xF1, 0xFF, 0xFF,  # TAB  LF  CR
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  # SPACE
    0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xF2, 0xFF, 0x3F,  # +  -  /
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,  # 0  1  2  3  4  5  6  7
    0x3C, 0x3D, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,  # 8  9  =
    # Cache Line 2
    # Bit probabilities for indexes 6 to 1 can be found in excel file
    # Bits 5 & 6 are TRUE with a probability of ~ 0.3846. Meaning false with ~ 0.6154
    0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,  # A  B  C  D  E  F  G
    0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,  # H  I  J  K  L  M  N  O
    0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,  # P  Q  R  S  T  U  V  W
    0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  # X  Y  Z
    0xFF, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,  # a  b  c  d  e  f  g
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,  # h  i  j  k  l  m  n  o
    0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,  # p  q  r  s  t  u  v  w
    0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]  # x  y  z

cache_line_record = {
    'n': [],
    'e': [],
    'd': [],
    'p': [],
    'q': [],
    'dmp1': [],
    'dmq1': [],
    'iqmp': []
}

fixed_bits_record = {
    'n': {'bits': '', 'count': 0, 'count_known': 0},
    'e': {'bits': '', 'count': 0, 'count_known': 0},
    'd': {'bits': '', 'count': 0, 'count_known': 0},
    'p': {'bits': '', 'count': 0, 'count_known': 0},
    'q': {'bits': '', 'count': 0, 'count_known': 0},
    'dmp1': {'bits': '', 'count': 0, 'count_known': 0},
    'dmq1': {'bits': '', 'count': 0, 'count_known': 0},
    'iqmp': {'bits': '', 'count': 0, 'count_known': 0},
}

active_decoding_parameter = ''


# Encoding from openssl:
#
# for (i = dlen; i > 0; i -= 3) {
#     if (i >= 3) {
#         l = (((unsigned long)f[0]) << 16L) |
#             (((unsigned long)f[1]) << 8L) | f[2];
#         *(t++) = conv_bin2ascii(l >> 18L, table);
#         *(t++) = conv_bin2ascii(l >> 12L, table);
#         *(t++) = conv_bin2ascii(l >> 6L, table);
#         *(t++) = conv_bin2ascii(l, table);
#     } else {
#         l = ((unsigned long)f[0]) << 16L;
#         if (i == 2)
#             l |= ((unsigned long)f[1] << 8L);
#
#         *(t++) = conv_bin2ascii(l >> 18L, table);
#         *(t++) = conv_bin2ascii(l >> 12L, table);
#         *(t++) = (i == 1) ? '=' : conv_bin2ascii(l >> 6L, table);
#         *(t++) = '=';
#     }
#     ret += 4;
#     f += 3;
# }
#
# *t = '\0';
def ossl_b64_enc(n_b):
    # n_b = int(number).to_bytes((int(number).bit_length() + 7) // 8, byteorder='big')
    # print(n_b)
    global data_bin2ascii

    i = len(n_b)
    j = 0
    target = ''
    while i > 0:
        if i >= 3:
            val = n_b[j] << 16 | n_b[j + 1] << 8 | n_b[j + 2]
            target += data_bin2ascii[(val >> 18) & 0x3F]
            target += data_bin2ascii[(val >> 12) & 0x3F]
            target += data_bin2ascii[(val >> 6) & 0x3F]
            target += data_bin2ascii[val & 0x3F]
        else:
            val = n_b[j] << 16
            if i == 2:
                val = val | (n_b[j + 1] << 8)

            target += data_bin2ascii[(val >> 18) & 0x3F]
            target += data_bin2ascii[(val >> 12) & 0x3F]

            if i == 1:
                target += '='
            else:
                target += data_bin2ascii[(val >> 6) & 0x3F]
            target += '='

        i = i - 3
        j = j + 3

    return target


# Decoding from openssl:
#
#   for (i = 0; i < n; i += 4) {
#         a = conv_ascii2bin(*(f++), table);
#         b = conv_ascii2bin(*(f++), table);
#         c = conv_ascii2bin(*(f++), table);
#         d = conv_ascii2bin(*(f++), table);
#         if ((a & 0x80) || (b & 0x80) || (c & 0x80) || (d & 0x80))
#             return -1;
#         l = ((((unsigned long)a) << 18L) |
#              (((unsigned long)b) << 12L) |
#              (((unsigned long)c) << 6L) | (((unsigned long)d)));
#         *(t++) = (unsigned char)(l >> 16L) & 0xff;
#         *(t++) = (unsigned char)(l >> 8L) & 0xff;
#         *(t++) = (unsigned char)(l) & 0xff;
#         ret += 3;
#   }
def _ossl_b64_dec(b64_string):
    target = bytes()
    i = 0
    while i < len(b64_string):
        a = _translate_ascii2bin(ord(b64_string[i]))
        b = _translate_ascii2bin(ord(b64_string[i + 1]))
        c = _translate_ascii2bin(ord(b64_string[i + 2]))
        d = _translate_ascii2bin(ord(b64_string[i + 3]))
        if (a & 0x80) or (b & 0x80) or (c & 0x80) or (d & 0x80):
            print('Invalid input!')
            exit(-1)
        val = (a << 18) | (b << 12) | (c << 6) | d
        target += ((val >> 16) & 0xff).to_bytes(length=1, byteorder='big')
        target += ((val >> 8) & 0xff).to_bytes(length=1, byteorder='big')
        target += (val & 0xff).to_bytes(length=1, byteorder='big')

        i += 4

    return target


def _translate_ascii2bin(index):
    global active_decoding_parameter
    global cache_line_record
    global fixed_bits_record
    global data_ascii2bin

    fixed_bits_record[active_decoding_parameter]['count'] += 6
    if index < 64:
        # print('Cache Line 1')
        cache_line_record[active_decoding_parameter].append(1)
        fixed_bits_record[active_decoding_parameter]['bits'] += '11xxxx'
        fixed_bits_record[active_decoding_parameter]['count_known'] += 2
    else:
        # print('Cache Line 2')
        cache_line_record[active_decoding_parameter].append(2)
        fixed_bits_record[active_decoding_parameter]['bits'] += 'xxxxxx'
        fixed_bits_record[active_decoding_parameter]['count_known'] += 0
    return data_ascii2bin[index]


def _get_hex_key_param_from_dump_between(marker_start, marker_end, dump):
    param = bytes()
    start_index = (dump.index(marker_start) + len(marker_start))
    if marker_end != -1:
        end_index = (dump.index(marker_end))
    else:
        end_index = len(dump)
    for e in dump[start_index:end_index] \
            .replace('\\n    ', '').replace('\\n', '').replace("'", '').split(':'):
        param += int(e, base=16).to_bytes(length=1, byteorder='big')
    return param


def _get_pub_exp_from_dump(dump):
    param = bytes()
    start_index = (dump.index('publicExponent:') + len('publicExponent:'))
    end_index = dump.index('privateExponent:')
    hex_str = dump[start_index:end_index].replace('\\n    ', '').replace('\\n', '').replace("'", '').split('(')[1][2:-1]
    param_int = int(hex_str, base=16)
    param += param_int.to_bytes(length=((param_int.bit_length() + 7) // 8), byteorder='big')
    return param


def _parse_ossl_key_dump(str_dump):
    dump = str(str_dump)
    n = _get_hex_key_param_from_dump_between('modulus:', 'publicExponent:', dump)
    e = _get_pub_exp_from_dump(dump)
    d = _get_hex_key_param_from_dump_between('privateExponent:', 'prime1:', dump)
    p = _get_hex_key_param_from_dump_between('prime1:', 'prime2:', dump)
    q = _get_hex_key_param_from_dump_between('prime2:', 'exponent1:', dump)
    dmp1 = _get_hex_key_param_from_dump_between('exponent1:', 'exponent2:', dump)
    dmq1 = _get_hex_key_param_from_dump_between('exponent2:', 'coefficient:', dump)
    iqmp = _get_hex_key_param_from_dump_between('coefficient:', -1, dump)

    return {'n': n, 'e': e, 'd': d, 'p': p, 'q': q, 'dmp1': dmp1, 'dmq1': dmq1, 'iqmp': iqmp}


def _b64_encode(key_parameters):
    n_b64 = ossl_b64_enc(key_parameters['n'])
    e_b64 = ossl_b64_enc(key_parameters['e'])
    d_b64 = ossl_b64_enc(key_parameters['d'])
    p_b64 = ossl_b64_enc(key_parameters['p'])
    q_b64 = ossl_b64_enc(key_parameters['q'])
    dmp1_b64 = ossl_b64_enc(key_parameters['dmp1'])
    dmq1_b64 = ossl_b64_enc(key_parameters['dmq1'])
    iqmp_b64 = ossl_b64_enc(key_parameters['iqmp'])

    result = {'n_b64': n_b64, 'e_b64': e_b64, 'd_b64': d_b64, 'p_b64': p_b64, 'q_b64': q_b64, 'dmp1_b64': dmp1_b64,
              'dmq1_b64': dmq1_b64, 'iqmp_b64': iqmp_b64}

    return result


def _b64_decode(key_parameters_b64):
    global active_decoding_parameter

    # print('--------------- Decoding n ---------------')
    active_decoding_parameter = 'n'
    n = _ossl_b64_dec(key_parameters_b64['n_b64'])
    # print('--------------- Decoding e ---------------')
    active_decoding_parameter = 'e'
    e = _ossl_b64_dec(key_parameters_b64['e_b64'])
    # print('--------------- Decoding d ---------------')
    active_decoding_parameter = 'd'
    d = _ossl_b64_dec(key_parameters_b64['d_b64'])
    # print('--------------- Decoding p ---------------')
    active_decoding_parameter = 'p'
    p = _ossl_b64_dec(key_parameters_b64['p_b64'])
    # print('--------------- Decoding q ---------------')
    active_decoding_parameter = 'q'
    q = _ossl_b64_dec(key_parameters_b64['q_b64'])
    # print('--------------- Decoding dmp1 ---------------')
    active_decoding_parameter = 'dmp1'
    dmp1 = _ossl_b64_dec(key_parameters_b64['dmp1_b64'])
    # print('--------------- Decoding dmq1 ---------------')
    active_decoding_parameter = 'dmq1'
    dmq1 = _ossl_b64_dec(key_parameters_b64['dmq1_b64'])
    # print('--------------- Decoding iqmp ---------------')
    active_decoding_parameter = 'iqmp'
    iqmp = _ossl_b64_dec(key_parameters_b64['iqmp_b64'])

    return {'n': n, 'e': e, 'd': d, 'p': p, 'q': q, 'dmp1': dmp1, 'dmq1': dmq1, 'iqmp': iqmp}


def recode_private_key_openssl(file_name):
    pem_b64 = pem.parse_file(file_name)
    # print(pem_b64[0].as_text())
    pkey = crypto.load_privatekey(type=crypto.FILETYPE_PEM, buffer=pem_b64[0].as_text())
    key_parameters = _parse_ossl_key_dump(crypto.dump_privatekey(crypto.FILETYPE_TEXT, pkey))
    key_parameters_b64 = _b64_encode(key_parameters)
    key_parameters_decoded = _b64_decode(key_parameters_b64)

    global cache_line_record
    global fixed_bits_record

    target_name_prefix = (file_name.split('/')[-1]).split('.')[0]
    _print_results('data/results/' + target_name_prefix + '_analysis_simulation.txt')
    _print_base64_encoded_params(key_parameters_b64, 'data/results/' + target_name_prefix + '_b64_data.txt')


def _print_base64_encoded_params(data, target):
    # 'data/results/b64_data.txt'
    with open(target, 'w') as f:
        for key, value in data.items():
            f.write(str(key) + ': ' + str(value) + '\n')


def _print_results(target):
    global cache_line_record
    global fixed_bits_record

    # 'data/results/analysis_simulation.txt'
    with open(target, 'w') as f:
        f.write('Cache Line Access:\n')
        clr_str = json.dumps(cache_line_record, indent=2)
        clr_str = re.sub(r'([12]),\s+', r'\1, ', clr_str)
        f.write(clr_str)
        f.write('\n\n')
        f.write('Known Bits:\n')
        json.dump(fixed_bits_record, fp=f, indent=2)


if __name__ == '__main__':
    arg_parser = argparse.ArgumentParser(description='Simulate base64 decoding of RSA keys - Optimal case')
    arg_parser.add_argument('--keyfile', nargs='?', const='data/private_key_4096.pem',
                            default='data/private_key_4096.pem', type=str)
    args = arg_parser.parse_args()
    print('Simulating decode process of ' + args.keyfile + '.')
    recode_private_key_openssl(args.keyfile)

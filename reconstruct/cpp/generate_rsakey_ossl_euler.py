from Crypto.PublicKey import RSA
import os
import subprocess
import math


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


def read_rsa_params(pem_file):
    with open(pem_file, 'r') as key_file:
        key_string = key_file.read()

    rsa_key = RSA.importKey(key_string)
    dp = get_mod_inverse(rsa_key.e, rsa_key.p - 1)
    dq = get_mod_inverse(rsa_key.e, rsa_key.q - 1)

    return {'n': rsa_key.n, 'e': rsa_key.e, 'p': rsa_key.p, 'q': rsa_key.q, 'd': rsa_key.d, 'dp': dp, 'dq': dq}


def main():
    # os.mkdir("euler_keys")

    ed = 0
    while ed != 1:
        print(subprocess.run(['openssl', 'genpkey', '-algorithm', 'RSA', '-pkeyopt', 'rsa_keygen_bits:4096',
                              '-out', 'euler_keys/ossel_euler_key_4096bit.pem']))

        key_params = read_rsa_params('euler_keys/ossel_euler_key_4096bit.pem')
        ed = (key_params['e'] * key_params['d']) % ((key_params['p'] - 1) * (key_params['q'] - 1))
        print("GCD: " + str(math.gcd(key_params['p'] - 1, key_params['q'] - 1)))
        print('e * d % phi(n) = ' + str(ed))

    print("Success!")


if __name__ == '__main__':
    main()

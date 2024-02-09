# reverse_example_4096.json
    * Aligned encoding 
    * 4096 bit
    * generated with custom script (generate_example_keys.py) -> Euler Phi function

# reverse_openssl_old_construction_test_key_4096bit.pem.json
    * Aligned encoding
    * 4096 bit
    * generated from parameters of an OpenSSL key (generate_example_from_given_parameters.py; from test_key_4096bit.pem) -> Carmichael ( with lcm(p-1, q-1) = (p - 1)*(q - 1) / 2 )

# reverse_ossl_old_generation_euler_key_4096bit.json
    * Aligned encoding
    * 4096 bit
    * generated from parameters of an OpenSSL Key with Euler Phi function (generate_example_from_given_parameters.py; euler_keys/ossel_euler_key_4096bit.pem)

# example_test_key_4096bit.pem.json
    * Unaligned encoding (as in PEM file)
    * 4096 bit
    * Not committed to repository, has to be generated with `generate_input_from_b64_enc_key.py` (on branch `feature/variable-start-block-size`) from test_key_4096bit.pem
    

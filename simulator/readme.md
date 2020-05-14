# OSSL Base64 Decoding simulator
This part of the algorithm can be used to create synthetic traces for testing purposes. 

## Requirements

The Python packages `pem, OpenSSL, json, re, argparse` have to be installed.
If they are not, use pip to do so.

## Running

Given a pem file containing a private RSA key, use the following command to construct a synthetic trace
`
 python3 simulator.py --keyfile=data/private_key_1024.pem
`

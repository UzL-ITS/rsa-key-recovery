# Lattice Solver
This part of the algorithm is used to recover the remaining half of the bits of a candidate, i.e. if we have already reconstructed half of the bits, we can use a lattice solver to reconstruct the remaining half in one swoop. 

## Installation
Make sure that [sage](http://sagemath.org) is installed and reachable from the command line.


## Configuration
The lattice solver is configured via a json file. 
In the simplest case, this json file contains the following fields
* `n` the public modulus
* `p_len` the length of n (usually, log(n)/2)
* `p_number_of_known_bits` the number of bits already reconstructed
* `number_of_workers` the number of parallel workers
* `host` the hostname used for listening

See [config.json](config.json) for a concrete example.

For test cases, it might also contain the addition field _p_ containing the secret factor. See [testConfig.json](testConfig.json) for a concrete example.

## Running the algorithm
The algorithm was run and tested on Ubuntu 20.04.

To run the lattice solver, use 

    $ ~/lattices$ sage rsa.sage --run [CONFIG]


To test the algorithm with a known secret factor, use 

    $ ~/lattices$ sage rsa.sage --test [CONFIG] [NUMBER OF ITERATIONS]


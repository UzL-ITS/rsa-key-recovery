# Key Reconstruction
This repository contains the key recovery part of the paper "Util::Lookup: Exploiting key decoding in cryptographic libraries" by Florian Sieck, Sebastian Berndt, Jan Wichelmann, and Thomas Eisenbarth (CCS 2021). 
It consists of three parts
* `simulator` contains a script to generate synthetic traces from a given pem file
* `reconstruct/cpp` contains the branch-and-prune algorithm to reconstruct all possible candidates up to a desired depth
* `reconstruct/lattices` contains the [sage](http://sagemath.org) code to reconstruct the most significant half of the bits via a lattice solver if the least significant half of the bits is already reconstructed. 

Each part contains its own readme file that documents the necessary steps to run the algorithm. 

## Attack code and paper

For the attack code and information about the publication please refer to [this repository](https://github.com/UzL-ITS/util-lookup).


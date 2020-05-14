# Configuration
All parameters needed for reconstruction (desired depth, input file, address of lattice solver) are set in [Config.cpp](src/Config.cpp). In more details, the parameters are as follows

* `intermediateDepth` sets the depth of the candidates produced via the breadth-first-approach
* `numberOfThreads` sets the number of used threads
* `finalDepth` sets the maximum depth for a candidate. If it is set to -1, it chooses the shortest possible parameter needed for complete reconstruction.
* `transmitToChainedSolver` sets whether a the final candidates should be transmitted to another solver (e.g. a lattice solver)
* `destinationHost` sets the hostname of the subsequent solver
* `destinationPort` sets the port number of the subsequent solver
* `observationFileName` sets the file name containing the observations, i.e., the entries `n, e, enc_p, enc_q, enc_d, enc_dp, enc_dq, length_p, length_q, length_d, length_dp, length_dq, line1` and `line2`. See `reverse_example_256.json` for a concrete example.
* `validationFileName` sets the file containing the secret information for validation, i.e., the entries `p, q, d, dp` and `dq`.  See `reverse_example_256.json` for a concrete example.
* `terminationFileName`

# Compilation

You can use the following sequence of commands to compile the C++ part of the reconstruction algorithm. 


    ~/cpp$ git submodule init
    ~/cpp$ git submodule update
    ~/cpp$ mkdir build && cd build && cmake ..
    ~/cpp/build$ make && cd ..


# Running the algorithm

If `transmitToChainedSolver` is set to true, make sure that the subsequent solver is already started. Then run

    ~/cpp$ ./reconstruct

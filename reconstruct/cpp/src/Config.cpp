//
// Created by florian on 22.09.20.
//

#include "Config.h"

unsigned int Config::intermediateDepth = 36;
unsigned int Config::numberOfThreads = 4;

// -1 means maximum expand depth for shortest param of candidate
// Else this parameter can be used to restrict the depth
int Config::finalDepth = 75;

// If you want to transmit candidates via a socket to a chained solver (e.g. lattice solver)
// set this to true
bool Config::transmitToChainedSolver = true;
std::string Config::destinationHost = "localhost";
std::string Config::destinationPort = "8888";

std::string Config::observationFileName = "reverse_example_256.json";
std::string Config::validationFileName = "reverse_example_256.json";
std::string Config::terminationFileName = "reverse_example_256.json";

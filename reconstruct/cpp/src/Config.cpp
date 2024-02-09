//
// Created by florian on 22.09.20.
//

#include "Config.h"

unsigned int Config::intermediateDepth = 1; // 36
unsigned int Config::numberOfThreads = 32;

// -1 means maximum expand depth for shortest param of candidate
// Else this parameter can be used to restrict the depth
// 75
int Config::finalDepth = -1;

// If you want to transmit candidates via a socket to a chained solver (e.g. lattice solver)
// set this to true
bool Config::transmitToChainedSolver = false;
std::string Config::destinationHost = "localhost";
std::string Config::destinationPort = "8888";

std::string Config::observationFileName = "paper_eval_keys/json/CM_0.pem.json";
std::string Config::validationFileName = "paper_eval_keys/json/CM_0.pem.json";
std::string Config::terminationFileName = "paper_eval_keys/json/CM_0.pem.json";

// Memjam: 11
// Cache Attack: 2 / 3
const int Config::observationPartitions = 11;
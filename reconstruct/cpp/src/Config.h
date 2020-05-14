//
// Created by florian on 22.09.20.
//

#ifndef RECONSTRUCT_CONFIG_H
#define RECONSTRUCT_CONFIG_H


#include <string>

class Config {
public:
  static unsigned int intermediateDepth;
  static unsigned int numberOfThreads;

  static std::string observationFileName;
  static std::string validationFileName;
  static std::string terminationFileName;

  static int finalDepth;

  static bool transmitToChainedSolver;
  static std::string destinationHost;
  static std::string destinationPort;
};


#endif //RECONSTRUCT_CONFIG_H

//
// Created by florian on 15.09.20.
//

#ifndef RECONSTRUCT_PARAMETERTERMINATIONHANDLER_H
#define RECONSTRUCT_PARAMETERTERMINATIONHANDLER_H


#include <cstddef>
#include <string>
#include <gmp.h>

class ParameterTerminationHandler {
public:

  typedef enum {
    P, Q, D, Dp, Dq
  } ParameterName;

  virtual size_t getLengthOfP() = 0;
  virtual size_t getLengthOfQ() = 0;
  virtual size_t getLengthOfD() = 0;
  virtual size_t getLengthOfDp() = 0;
  virtual size_t getLengthOfDq() = 0;
  virtual size_t getMaxDepth() = 0;

  virtual size_t getShortestParamLength() = 0;
  virtual ParameterName getShortestExpandParamName() = 0;
  virtual size_t getShortestParamExpandLength() = 0;

  virtual void getTerminalValue(mpz_t val, mpz_t parameter, unsigned int depth) = 0;
};


#endif //RECONSTRUCT_PARAMETERTERMINATIONHANDLER_H

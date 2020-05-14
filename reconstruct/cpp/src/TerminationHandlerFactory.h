//
// Created by florian on 16.09.20.
//

#ifndef RECONSTRUCT_TERMINATIONHANDLERFACTORY_H
#define RECONSTRUCT_TERMINATIONHANDLERFACTORY_H


#include <memory>
#include "ParameterTerminationHandler.h"
#include "Candidate.h"

class TerminationHandlerFactory {
public:

  static std::shared_ptr<ParameterTerminationHandler> getParameterTerminationHandler(Candidate &c);

private:

  static bool initialized;
  static size_t lengthOfN;
  static size_t lengthOfDq;
  static size_t lengthOfDp;
  static size_t lengthOfD;
  static size_t lengthOfQ;
  static size_t lengthOfP;

  static void loadParameters(std::string file_name);
};


#endif //RECONSTRUCT_TERMINATIONHANDLERFACTORY_H

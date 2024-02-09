//
// Created by florian on 15.09.20.
//

#ifndef RECONSTRUCT_SIMPLEEXAMPLETERMINATIONHANDLER_H
#define RECONSTRUCT_SIMPLEEXAMPLETERMINATIONHANDLER_H


#include <memory>
#include "ParameterTerminationHandler.h"
#include "TerminationHandlerFactory.h"
#include "Candidate.h"


/**
 * Intentionally using anti pattern singleton for simplicity.
 */
class SimpleExampleTerminationHandler: public ParameterTerminationHandler {

  friend std::shared_ptr<ParameterTerminationHandler> TerminationHandlerFactory::getParameterTerminationHandler(Candidate &c);

public:

  virtual ~SimpleExampleTerminationHandler();

  size_t getLengthOfP() override;
  size_t getLengthOfQ() override;
  size_t getLengthOfD() override;
  size_t getLengthOfDp() override;
  size_t getLengthOfDq() override;
  size_t getMaxDepth() override;

  size_t getShortestParamLength() override;
  size_t getShortestParamExpandLength() override;
  ParameterName getShortestExpandParamName() override;

  void getTerminalValue(mpz_t val, mpz_t parameter, unsigned int depth) override;

private:

  size_t lengthOfP;
  size_t lengthOfQ;
  size_t lengthOfD;
  size_t lengthOfDq;
  size_t lengthOfDp;
  size_t expandLengthForShortestParam;

  size_t lengthShortestParam;
  ParameterName shortestParam;

  size_t maxDepth;

  SimpleExampleTerminationHandler();
  SimpleExampleTerminationHandler(size_t lengthOfP, size_t lengthOfQ, size_t lengthOfD,
                                  size_t lengthOfDp, size_t lengthOfDq, size_t lengthShortestParam,
                                  ParameterName shortestParam, size_t maxDepth, size_t expandLengthForShortestParam);
};


#endif //RECONSTRUCT_SIMPLEEXAMPLETERMINATIONHANDLER_H

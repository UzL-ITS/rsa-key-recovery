//
// Created by florian on 15.09.20.
//

#include "SimpleExampleTerminationHandler.h"
#include "BinaryHelper.h"

SimpleExampleTerminationHandler::SimpleExampleTerminationHandler(size_t lengthOfP, size_t lengthOfQ,
                                                                 size_t lengthOfD,
                                                                 size_t lengthOfDp,
                                                                 size_t lengthOfDq,
                                                                 size_t lengthShortestParam,
                                                                 ParameterName shortestParam,
                                                                 size_t maxDepth,
                                                                 size_t expandLengthForShortestParam):
                                                             lengthOfP(lengthOfP),
                                                             lengthOfQ(lengthOfQ),
                                                             lengthOfD(lengthOfD),
                                                             lengthOfDp(lengthOfDp),
                                                             lengthOfDq(lengthOfDq),
                                                             lengthShortestParam(lengthShortestParam),
                                                             shortestParam(shortestParam),
                                                             maxDepth(maxDepth),
                                                             expandLengthForShortestParam(expandLengthForShortestParam) {

}


SimpleExampleTerminationHandler::SimpleExampleTerminationHandler() {}

SimpleExampleTerminationHandler::~SimpleExampleTerminationHandler() {

}

size_t SimpleExampleTerminationHandler::getLengthOfP() {
  return lengthOfP;
}

size_t SimpleExampleTerminationHandler::getLengthOfQ() {
  return lengthOfQ;
}

void SimpleExampleTerminationHandler::getTerminalValue(mpz_t val, mpz_t parameter,
    unsigned int depth) {
  size_t bit_length = mpz_sizeinbase(parameter, 2);
  BinaryHelper::most_significant_bits(val, parameter, bit_length - ((depth / 6) * 6));
}

size_t SimpleExampleTerminationHandler::getLengthOfD() {
  return lengthOfD;
}

size_t SimpleExampleTerminationHandler::getLengthOfDp() {
  return lengthOfDp;
}

size_t SimpleExampleTerminationHandler::getLengthOfDq() {
  return lengthOfDq;
}

ParameterTerminationHandler::ParameterName SimpleExampleTerminationHandler::getShortestExpandParamName() {
  return shortestParam;
}

size_t SimpleExampleTerminationHandler::getShortestParamLength() {
  return lengthShortestParam;
}

size_t SimpleExampleTerminationHandler::getMaxDepth() {
  return maxDepth;
}

size_t SimpleExampleTerminationHandler::getShortestParamExpandLength() {
    return expandLengthForShortestParam;
}


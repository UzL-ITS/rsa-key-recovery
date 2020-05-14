//
// Created by florian on 16.09.20.
//

#include <fstream>
#include <algorithm>

#include <json/json.h>

#include "TerminationHandlerFactory.h"
#include "SimpleExampleTerminationHandler.h"
#include "Constants.h"
#include "Config.h"

bool TerminationHandlerFactory::initialized = false;
size_t TerminationHandlerFactory::lengthOfN;
size_t TerminationHandlerFactory::lengthOfDq;
size_t TerminationHandlerFactory::lengthOfDp;
size_t TerminationHandlerFactory::lengthOfD;
size_t TerminationHandlerFactory::lengthOfQ;
size_t TerminationHandlerFactory::lengthOfP;

std::shared_ptr<ParameterTerminationHandler>
TerminationHandlerFactory::getParameterTerminationHandler(Candidate &c) {

  if(!initialized) {
    loadParameters(Config::terminationFileName);
  }

  size_t expandLengthForShortestParam = std::min({lengthOfP, lengthOfQ, lengthOfD - c.getTauK(),
                                            lengthOfDp - c.getTauKp(), lengthOfDq - c.getTauKq()});
  size_t shortestParam = std::min({lengthOfP, lengthOfQ, lengthOfD,
                             lengthOfDp, lengthOfDq});

  ParameterTerminationHandler::ParameterName shortestParamName;

  if (shortestParam == lengthOfP) {
    shortestParamName = ParameterTerminationHandler::ParameterName::P;
  } else if (shortestParam == lengthOfQ) {
    shortestParamName = ParameterTerminationHandler::ParameterName::Q;
  } else if (shortestParam == (lengthOfD)) {
    shortestParamName = ParameterTerminationHandler::ParameterName::D;
  } else if (shortestParam == (lengthOfDp)) {
    shortestParamName = ParameterTerminationHandler::ParameterName::Dp;
  } else if (shortestParam == (lengthOfDq)) {
    shortestParamName = ParameterTerminationHandler::ParameterName::Dq;
  }

  size_t maxParamLength = ((shortestParam / Constants::BITS_PER_B64_SYMBOL) + 1) * 6;


  std::shared_ptr<ParameterTerminationHandler> handler = std::shared_ptr<SimpleExampleTerminationHandler>(
      new SimpleExampleTerminationHandler(lengthOfP, lengthOfQ, lengthOfD, lengthOfDp, lengthOfDq,
          shortestParam, shortestParamName, maxParamLength, expandLengthForShortestParam));

  return handler;
}

void TerminationHandlerFactory::loadParameters(std::string file_name) {
  std::ifstream ifs(file_name);
  Json::Value root;

  ifs >> root;

  lengthOfN = root["length_n"].asUInt();
  lengthOfP = root["length_p"].asUInt();
  lengthOfQ = root["length_q"].asUInt();
  lengthOfD = root["length_d"].asUInt();
  lengthOfDp = root["length_dp"].asUInt();
  lengthOfDq = root["length_dq"].asUInt();
}

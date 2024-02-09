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

  if (!initialized) {
    loadParameters(Config::terminationFileName);
  }

  size_t shortestParamExpandLength = std::min(
      {lengthOfP - c.getTauGamma(), lengthOfQ - c.getTauGamma(), lengthOfD - c.getTauK(),
       lengthOfDp - c.getTauKp() - c.getTauGamma(),
       lengthOfDq - c.getTauKq() - c.getTauGamma()});
  size_t shortestParam = std::min({lengthOfP, lengthOfQ, lengthOfD,
                                  lengthOfDp, lengthOfDq});

  ParameterTerminationHandler::ParameterName shortestExpandParamName;

  if (shortestParamExpandLength == lengthOfP - c.getTauGamma()) {
    shortestExpandParamName = ParameterTerminationHandler::ParameterName::P;
  } else if (shortestParamExpandLength == lengthOfQ - c.getTauGamma()) {
    shortestExpandParamName = ParameterTerminationHandler::ParameterName::Q;
  } else if (shortestParamExpandLength == (lengthOfD) - c.getTauK()) {
    shortestExpandParamName = ParameterTerminationHandler::ParameterName::D;
  } else if (shortestParamExpandLength == (lengthOfDp)  - c.getTauKp() - c.getTauGamma()) {
    shortestExpandParamName = ParameterTerminationHandler::ParameterName::Dp;
  } else if (shortestParamExpandLength == (lengthOfDq) - c.getTauKq() - c.getTauGamma()) {
    shortestExpandParamName = ParameterTerminationHandler::ParameterName::Dq;
  }

  size_t maxParamLength = ((shortestParam / Constants::BITS_PER_B64_SYMBOL) + 1) * 6;


  std::shared_ptr<ParameterTerminationHandler> handler = std::shared_ptr<SimpleExampleTerminationHandler>(
      new SimpleExampleTerminationHandler(lengthOfP, lengthOfQ, lengthOfD, lengthOfDp, lengthOfDq,
                                          shortestParam, shortestExpandParamName, maxParamLength,
                                          shortestParamExpandLength));

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

  initialized = true;
}

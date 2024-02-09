//
// Created by florian on 12/1/22.
//

#include <fstream>
#include "LengthValues.h"
#include "json/json.h"
#include "Config.h"

LengthValues::LengthValues() {
  loadParameters();
}

void LengthValues::loadParameters() {
  std::ifstream ifs(Config::terminationFileName);
  Json::Value root;

  ifs >> root;

  lengthOfN = root["length_n"].asUInt();
  lengthOfP = root["length_p"].asUInt();
  lengthOfQ = root["length_q"].asUInt();
  lengthOfD = root["length_d"].asUInt();
  lengthOfDp = root["length_dp"].asUInt();
  lengthOfDq = root["length_dq"].asUInt();
}

size_t LengthValues::getLengthOfN() const {
  return lengthOfN;
}

size_t LengthValues::getLengthOfDq() const {
  return lengthOfDq;
}

size_t LengthValues::getLengthOfDp() const {
  return lengthOfDp;
}

size_t LengthValues::getLengthOfD() const {
  return lengthOfD;
}

size_t LengthValues::getLengthOfQ() const {
  return lengthOfQ;
}

size_t LengthValues::getLengthOfP() const {
  return lengthOfP;
}



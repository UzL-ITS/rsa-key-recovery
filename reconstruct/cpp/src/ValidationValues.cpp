//
// Created by florian on 16.09.20.
//
#include <fstream>
#include <iostream>

#include "ValidationValues.h"
#include "Config.h"

#include <json/json.h>

ValidationValues::ValidationValues() {
  mpz_inits(n, e, p ,q, d, dp, dq, NULL);
  load(Config::validationFileName);
}

ValidationValues::~ValidationValues() {
  mpz_clears(n, e, p ,q, d, dp, dq, NULL);
}

void ValidationValues::load(std::string file_name) {
  std::ifstream ifs(file_name);
  Json::Value root;

  ifs >> root;

  std::string nString = root["n"].asString();
  nString = nString.substr(2);
  std::string eString = root["e"].asString();
  eString = eString.substr(2);
  std::string pString = root["p"].asString();
  pString = pString.substr(2);
  std::string qString = root["q"].asString();
  qString = qString.substr(2);
  std::string dString = root["d"].asString();
  dString = dString.substr(2);
  std::string dqString = root["dq"].asString();
  dqString = dqString.substr(2);
  std::string dpString = root["dp"].asString();
  dpString = dpString.substr(2);

  mpz_set_str(this->n, nString.c_str(), 10);
  mpz_set_str(this->e, eString.c_str(), 10);
  mpz_set_str(this->p, pString.c_str(), 10);
  mpz_set_str(this->q, qString.c_str(), 10);
  mpz_set_str(this->d, dString.c_str(), 10);
  mpz_set_str(this->dp, dpString.c_str(), 10);
  mpz_set_str(this->dq, dqString.c_str(), 10);
}

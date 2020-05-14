//
// Created by florian on 16.09.20.
//

#ifndef RECONSTRUCT_VALIDATIONVALUES_H
#define RECONSTRUCT_VALIDATIONVALUES_H


#include <string>
#include <gmp.h>

class ValidationValues {
public:
  ValidationValues();

  virtual ~ValidationValues();

  void load(std::string file_name);

  mpz_t n;
  mpz_t e;
  mpz_t p;
  mpz_t q;
  mpz_t d;
  mpz_t dp;
  mpz_t dq;
};


#endif //RECONSTRUCT_VALIDATIONVALUES_H

//
// Created by florian on 15.09.20.
//

#include "BinaryHelper.h"
#include "MathUtils.h"

void MathUtils::modularInverse(mpz_t inverse, mpz_t number, mpz_t modulus) {
  mpz_invert(inverse, number, modulus);
}

unsigned int MathUtils::tau(mpz_t parameter) {
  size_t exp2Div = 0;
  mpz_t rm;
  mpz_init_set_ui(rm, 0);

  while (mpz_cmp_ui(rm, 0) == 0) {
    exp2Div++;
    mpz_tdiv_r_2exp (rm, parameter, exp2Div);
  }

  mpz_clear(rm);

  return exp2Div - 1;
}
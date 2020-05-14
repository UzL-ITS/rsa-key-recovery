//
// Created by florian on 15.09.20.
//

#include "Constants.h"
#include "Candidate.h"
#include "BinaryHelper.h"

void BinaryHelper::least_significant_bits(mpz_t lsb, mpz_t parameter,
                                          unsigned int nb_of_bits) {
  size_t bit_length = mpz_sizeinbase(parameter, 2);
  mpz_tdiv_r_2exp(lsb, parameter, nb_of_bits);
}

void BinaryHelper::most_significant_bits(mpz_t msb, mpz_t parameter,
                                         unsigned int nb_of_bits) {
  size_t bit_length = mpz_sizeinbase(parameter, 2);
  mpz_tdiv_q_2exp(msb, parameter, bit_length - nb_of_bits);
}
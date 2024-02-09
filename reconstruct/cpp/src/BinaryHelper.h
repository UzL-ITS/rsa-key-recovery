//
// Created by florian on 15.09.20.
//

#ifndef RECONSTRUCT_BINARYHELPER_H
#define RECONSTRUCT_BINARYHELPER_H


#include <boost/multiprecision/gmp.hpp>


class BinaryHelper {

public:
  static void least_significant_bits(mpz_t lsb, mpz_t parameter, unsigned int nb_of_bits);

  static void most_significant_bits(mpz_t msb, mpz_t parameter, unsigned int nb_of_bits);
};


#endif //RECONSTRUCT_BINARYHELPER_H

//
// Created by florian on 15.09.20.
//

#ifndef RECONSTRUCT_MATHUTILS_H
#define RECONSTRUCT_MATHUTILS_H


#include <boost/multiprecision/gmp.hpp>

class MathUtils {

public:
  static void modularInverse(mpz_t inverse, mpz_t number, mpz_t modulus);

  static unsigned int tau(mpz_t parameter);
};


#endif //RECONSTRUCT_MATHUTILS_H

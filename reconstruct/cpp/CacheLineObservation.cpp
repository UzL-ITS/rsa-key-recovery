//
// Created by florian on 27.08.20.
//

#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

#include <json/json.h>

#include "CacheLineObservation.h"
#include "Constants.h"
#include "Config.h"
#include "MathUtils.h"

CacheLineObservation::CacheLineObservation() {

  mpz_inits(n, e, NULL);

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 64; j++) {
      cacheLines[i][j] = 0xFF;
    }
  }

  CacheLineObservation::load(Config::observationFileName);
}

CacheLineObservation::~CacheLineObservation() {
  mpz_clears(n, e, NULL);
}

void CacheLineObservation::load(std::string file_name) {
  std::ifstream ifs(file_name);
  Json::Value root;

  ifs >> root;

  std::string nString = root["n"].asString();
  nString = nString.substr(2);
  std::string eString = root["e"].asString();
  eString = eString.substr(2);

  mpz_set_str(this->n, nString.c_str(), 10);
  mpz_set_str(this->e, eString.c_str(), 10);

  Json::Value line1 = root["line1"];
  Json::Value line2 = root["line2"];

  for (int i = 0; i < line1.size(); i++) {
    int index = line1[i].asInt();
    if(index < 64) {
      cacheLines[0][index] = 0;
    }
  }
  for (int i = 0; i < line2.size(); i++) {
    int index = line2[i].asInt();
    if(index < 64) {
      cacheLines[1][index] = 0;
    }
  }

  Json::Value p = root["enc_p"];
  loadParameter(p, this->p);
  Json::Value q = root["enc_q"];
  loadParameter(q, this->q);
  Json::Value d = root["enc_d"];
  loadParameter(d, this->d);
  Json::Value dp = root["enc_dp"];
  loadParameter(dp, this->dp);
  Json::Value dq = root["enc_dq"];
  loadParameter(dq, this->dq);
  Json::Value qp = root["enc_qp"];
  loadParameter(qp, this->qp);
}

void CacheLineObservation::loadParameter(Json::Value &source, std::vector<uint8_t> &target) {
  for (int i = 0; i < source.size(); i++) {
    uint8_t val = static_cast<uint8_t> (source[i].asInt());
    target.push_back(val);
  }
}

bool CacheLineObservation::isInCacheLine(CacheLineObservation::CacheLines l, uint8_t value) {
  if (value < 64) {
    if (cacheLines[l][value] == 0) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

uint8_t CacheLineObservation::getPAt(unsigned int index) {
  size_t size = p.size();
  if(index < size) {
    return p.at(size - index - 1);
  } else {
    return 0xFF;
  }
}

uint8_t CacheLineObservation::getQAt(unsigned int index) {
  size_t size = q.size();
  if(index < size) {
    return q.at(size - index - 1);
  } else {
    return 0xFF;
  }
}

uint8_t CacheLineObservation::getDpAt(unsigned int index) {
  size_t size = dp.size();
  if(index < size) {
    return dp.at(size - index - 1);
  } else {
    return 0xFF;
  }
}

uint8_t CacheLineObservation::getDAt(unsigned int index) {
  size_t size = d.size();
  if(index < size) {
    return d.at(size - index - 1);
  } else {
    return 0xFF;
  }
}

uint8_t CacheLineObservation::getDqAt(unsigned int index) {
  size_t size = dq.size();
  if(index < size) {
    return dq.at(size - index - 1);
  } else {
    return 0xFF;
  }
}

// ToDo
bool CacheLineObservation::checkDkAgainstObservation(mpz_t dk) {
  size_t number_of_msb_to_check;
  mpz_t dk_block, mask;
  mpz_inits(dk_block, mask, NULL);
  bool returnVal = false;

  number_of_msb_to_check = mpz_sizeinbase(n, 2);
  number_of_msb_to_check = ((number_of_msb_to_check / 2) - 2) / 6;

  mpz_set_ui(mask, 0x3F);

  size_t dk_bit_length = mpz_sizeinbase(dk, 2);
  size_t pos = 0;

  std::vector<uint8_t> dk_cache_line_enc;

  while (pos < dk_bit_length) {

    uint8_t dk_block_ui;

    // ToDo: Does this also work as expected when bit_len(mask) > bit_len(dk) ?
    mpz_and(dk_block, dk, mask);
    mpz_tdiv_q_2exp(dk_block, dk_block, pos);
    dk_block_ui = static_cast<uint8_t > (mpz_get_ui(dk_block));

    if(cacheLines[0][dk_block_ui] == 0) {
      dk_cache_line_enc.push_back(0);
    } else if(cacheLines[1][dk_block_ui] == 0) {
      dk_cache_line_enc.push_back(1);
    } else {
      std::cerr << "Invalid encoding for dk" << std::endl;
    }

    mpz_mul_2exp(mask, mask, Constants::BITS_PER_B64_SYMBOL);
    pos += Constants::BITS_PER_B64_SYMBOL;
  }

  std::reverse(dk_cache_line_enc.begin(), dk_cache_line_enc.end());

  if (number_of_msb_to_check < d.size()) {
    returnVal = true;
    for (int i = 0; i < number_of_msb_to_check; i++) {
      if (dk_cache_line_enc.at(i) != d.at(i)) {
        returnVal = false;
        break;
      }
    }
  }

  mpz_clears(dk_block, mask, NULL);
  return returnVal;
}

void CacheLineObservation::generateK(std::vector<std::shared_ptr<KFactors_t>> &ks) {
  unsigned int e = mpz_get_ui(this->e);
  for (unsigned int k = 1; k < e; k++) {
    mpz_t dk;
    mpz_init(dk);
    calcDk(dk, k);

    if (checkDkAgainstObservation(dk)) {
      std::shared_ptr<KFactors_t> kFactors1 = std::make_shared<KFactors_t>();
      mpz_set_ui(kFactors1->k, k);

      generateKpKq(kFactors1->k_p, kFactors1->k_q, kFactors1->k);

      ks.push_back(kFactors1);

      std::shared_ptr<KFactors_t> kFactors2 = std::make_shared<KFactors_t>();
      mpz_set(kFactors2->k_p, kFactors1->k_q);
      mpz_set(kFactors2->k_q, kFactors1->k_p);
      mpz_set_ui(kFactors2->k, k);

      ks.push_back(kFactors2);
    }

    mpz_clear(dk);
  }
}


void CacheLineObservation::calcDk(mpz_t dk, unsigned int k) {
  mpz_add_ui(dk, this->n, 1);
  mpz_mul_ui(dk, dk, k);
  mpz_add_ui(dk, dk, 1);

  mpq_t dk_rational;
  mpq_init(dk_rational);
  mpq_set_z(dk_rational, dk);
  mpq_t e_rational;
  mpq_init(e_rational);
  mpq_set_z(e_rational, this->e);

  mpq_div(dk_rational, dk_rational, e_rational);

  mpz_set_q(dk, dk_rational);
  mpq_clears(dk_rational, e_rational, NULL);
}

void CacheLineObservation::generateKpKq(mpz_t k_p, mpz_t k_q, mpz_t k) {
  mpz_t solutionOne, solutionTwo, intermediate;
  mpz_inits(solutionOne, solutionTwo, intermediate, NULL);

  // Compute a=k*(N-1)+1
  mpz_mod(solutionOne, k, this->e);
  mpz_mod(intermediate, this->n, this->e);
  mpz_sub_ui(intermediate, intermediate, 1);
  mpz_mul(solutionOne, solutionOne, intermediate);
  mpz_add_ui(solutionOne, solutionOne, 1);
  mpz_mod(solutionOne, solutionOne, this->e);

  
  // Compute b=\sqrt{[k*(N-1)+1]^2 +4*k}
  mpz_sub_ui(solutionTwo, this->n, 1);
  mpz_mul(solutionTwo, solutionTwo, k);
  mpz_add_ui(solutionTwo, solutionTwo, 1);
  mpz_mul(solutionTwo, solutionTwo, solutionTwo);
  mpz_mul_ui(intermediate, k, 4);
  mpz_add(solutionTwo, solutionTwo, intermediate);
  quadraticResidue(solutionTwo, solutionTwo, this->e);

  // Compute modular inverse of 2
  mpz_init(intermediate);
  mpz_add_ui(intermediate, intermediate, 2);
  MathUtils::modularInverse(intermediate, intermediate, this->e);

  // Compute k_p=(a+b)/2
  mpz_add(k_p, solutionOne, solutionTwo);
  mpz_mul(k_p, k_p, intermediate);
  mpz_mod(k_p, k_p, this->e);

  char *tmp = mpz_get_str(nullptr, 10, k_p);
  std::cout <<"New k_p " << tmp << "\n";


  // if((mpz_odd_p(solutionOne) && mpz_odd_p(solutionTwo)) || mpz_even_p(solutionOne) && mpz_even_p(solutionTwo)){
  //   mpz_tdiv_q_ui(k_p, k_p, 2);
  //   mpz_mod(k_p, k_p, this->e);
  // }
  // else{
  //   mpz_mod(k_p, k_p, this->e);
  //   if(mpz_odd_p(k_p)){
  //     char *tmp = mpz_get_str(nullptr, 10, k_p);


  //     std::cout <<"k_p " << tmp << "is odd!\n";
  //     exit(1);

  //   }
  //   mpz_tdiv_q_ui(k_p, k_p, 2);
  // }
  
  

  // Compute k_q=(a-b)/2
  mpz_sub(k_q, solutionOne, solutionTwo);
  mpz_mul(k_q, k_q, intermediate);
  mpz_mod(k_q, k_q, this->e);
  *tmp = mpz_get_str(nullptr, 10, k_p);
  std::cout <<"New k_q " << tmp << "\n";

  // if((mpz_odd_p(solutionOne) && mpz_odd_p(solutionTwo)) || mpz_even_p(solutionOne) && mpz_even_p(solutionTwo)){
  //   mpz_tdiv_q_ui(k_q, k_q, 2);
  //   mpz_mod(k_q, k_q, this->e);
  // }
  // else{
  //   mpz_mod(k_q, k_q, this->e);
  //   if(mpz_odd_p(k_q)){
  //     char *tmp = mpz_get_str(nullptr, 10, k_q);
  //     std::cout <<"k_q " << tmp << "is odd!\n";
  //     exit(1);

  //   }
  //   mpz_tdiv_q_ui(k_q, k_q, 2);
  // }
  
  
  
  mpz_clears(solutionOne, solutionTwo, intermediate, NULL);
}

// Source: https://gmplib.org/list-archives/gmp-discuss/2013-April/005303.html
//
// find x^2 = q mod n
// return
// -1 q is quadratic non-residue mod n
//  1 q is quadratic residue mod n
//  0 q is congruent to 0 mod n
//
int CacheLineObservation::quadraticResidue(mpz_t x, mpz_t q, mpz_t n) {
  int leg;
  mpz_t tmp, ofac, nr, t, r, c, b;
  unsigned int mod4;
  mp_bitcnt_t twofac = 0, m, i, ix;

  mod4 = mpz_tstbit(n, 0);
  if (!mod4) // must be odd
    return 0;

  mod4 += 2 * mpz_tstbit(n, 1);

  leg = mpz_legendre(q, n);
  if (leg != 1)
    return leg;

  mpz_init_set(tmp, n);

  if (mod4 == 3) // directly, x = q^(n+1)/4 mod n
  {
    mpz_add_ui(tmp, tmp, 1UL);
    mpz_tdiv_q_2exp(tmp, tmp, 2);
    mpz_powm(x, q, tmp, n);
    mpz_clear(tmp);
  } else // Tonelli-Shanks
  {
    mpz_inits(ofac, t, r, c, b, NULL);

    // split n - 1 into odd number times power of 2 ofac*2^twofac
    mpz_sub_ui(tmp, tmp, 1UL);
    twofac = mpz_scan1(tmp, twofac); // largest power of 2 divisor
    if (twofac)
      mpz_tdiv_q_2exp(ofac, tmp, twofac); // shift right

    // look for non-residue
    mpz_init_set_ui(nr, 2UL);
    while (mpz_legendre(nr, n) != -1)
      mpz_add_ui(nr, nr, 1UL);

    mpz_powm(c, nr, ofac, n); // c = nr^ofac mod n

    mpz_add_ui(tmp, ofac, 1UL);
    mpz_tdiv_q_2exp(tmp, tmp, 1);
    mpz_powm(r, q, tmp, n); // r = q^(ofac+1)/2 mod n

    mpz_powm(t, q, ofac, n);
    mpz_mod(t, t, n); // t = q^ofac mod n

    if (mpz_cmp_ui(t, 1UL) != 0) // if t = 1 mod n we're done
    {
      m = twofac;
      do {
        i = 2;
        ix = 1;
        while (ix < m) {
          // find lowest 0 < ix < m | t^2^ix = 1 mod n
          mpz_powm_ui(tmp, t, i, n); // repeatedly square t
          if (mpz_cmp_ui(tmp, 1UL) == 0)
            break;
          i <<= 1; // i = 2, 4, 8, ...
          ix++; // ix is log2 i
        }
        mpz_powm_ui(b, c, 1 << (m - ix - 1), n); // b = c^2^(m-ix-1) mod n
        mpz_mul(r, r, b);
        mpz_mod(r, r, n); // r = r*b mod n
        mpz_mul(c, b, b);
        mpz_mod(c, c, n); // c = b^2 mod n
        mpz_mul(t, t, c);
        mpz_mod(t, t, n); // t = t b^2 mod n
        m = ix;
      } while (mpz_cmp_ui(t, 1UL) != 0); // while t mod n != 1
    }
    mpz_set(x, r);
    mpz_clears(tmp, ofac, nr, t, r, c, b, NULL);
  }

  return 1;
}

//
// Created by florian on 27.08.20.
//

#ifndef RECONSTRUCT_CACHELINEOBSERVATION_H
#define RECONSTRUCT_CACHELINEOBSERVATION_H

#include <vector>
#include <cstdint>
#include <string>

#include <boost/multiprecision/gmp.hpp>

using namespace boost::multiprecision;

class CacheLineObservation {
public:

  enum CacheLines {
    LINE_1=0,
    LINE_2=1
  };

  typedef class KFactors{
  public:

    KFactors() {
      mpz_inits(k, k_p, k_q, NULL);
    }
    virtual ~KFactors() {
      mpz_clears(k, k_p, k_q, NULL);
    }

    mpz_t k;
    mpz_t k_p;
    mpz_t k_q;
  } KFactors_t;

  CacheLineObservation();

  virtual ~CacheLineObservation();

  void load(std::string file_name);

  bool isInCacheLine(CacheLines l, uint8_t value);

  uint8_t getPAt(unsigned int index);
  uint8_t getQAt(unsigned int index);
  uint8_t getDAt(unsigned int index);
  uint8_t getDpAt(unsigned int index);
  uint8_t getDqAt(unsigned int index);

  void generateK(std::vector<std::shared_ptr<KFactors_t>> &ks);

  mpz_t n;
  mpz_t e;

private:

  bool checkDkAgainstObservation(mpz_t dk);
  void generateKpKq(mpz_t k_p, mpz_t k_q, mpz_t k);

  // ToDo: Verify with modular multiplication
  static int quadraticResidue(mpz_t x, mpz_t q, mpz_t n);

  void calcDk(mpz_t dk, unsigned int k);

  static void loadParameter(Json::Value &source, std::vector<uint8_t> &target);

  std::vector<uint8_t> p;
  std::vector<uint8_t> q;
  std::vector<uint8_t> d;
  std::vector<uint8_t> dp;
  std::vector<uint8_t> dq;
  std::vector<uint8_t> qp;

  uint8_t cacheLines[2][64];
};


#endif //RECONSTRUCT_CACHELINEOBSERVATION_H

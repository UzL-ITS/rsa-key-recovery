//
// Created by florian on 10/10/22.
//

#ifndef RECONSTRUCT_OBSERVATION_H
#define RECONSTRUCT_OBSERVATION_H


#include <vector>
#include <cstdint>
#include <string>

#include <json/json.h>
#include <boost/multiprecision/gmp.hpp>

#include "Config.h"
#include "LengthValues.h"

using namespace boost::multiprecision;

class Observation {
public:

  static const int NB_OF_OBSERVATION_PARTITIONS;
  static const int LOWER_2_BITS = 2;
  static const int LOWER_4_BITS = 4;
  static const int UPPER_2_BITS = -4;
  static const int UPPER_4_BITS = -2;
  static const int NO_SHIFT = 0;

  typedef class KFactors{
  public:

    KFactors() {
      mpz_inits(k, k_p, k_q, NULL);
    }
    virtual ~KFactors() {
      mpz_clears(k, k_p, k_q, NULL);
    }

    mpz_t k{};
    mpz_t k_p{};
    mpz_t k_q{};
  } KFactors_t;

  explicit Observation();

  virtual ~Observation();

  void load(std::string file_name);

  bool isInObservationPartition(unsigned int item_idx, uint8_t value);
  bool isInObservationPartitionLower4(unsigned int item_idx, uint8_t value);
  bool isInObservationPartitionLower2(unsigned int item_idx, uint8_t value);
  bool isInObservationPartitionUpper4(unsigned int item_idx, uint8_t value);
  bool isInObservationPartitionUpper2(unsigned int item_idx, uint8_t value);

  uint8_t getPAt(unsigned int index);
  uint8_t getQAt(unsigned int index);
  uint8_t getDAt(unsigned int index);
  uint8_t getDpAt(unsigned int index);
  uint8_t getDqAt(unsigned int index);

  int getPBitsShiftStart() const;

  int getPBitsShiftEnd() const;

  int getQBitsShiftStart() const;

  int getQBitsShiftEnd() const;

  int getDBitsShiftStart() const;

  int getDBitsShiftEnd() const;

  int getDpBitsShiftStart() const;

  int getDpBitsShiftEnd() const;

  int getDqBitsShiftStart() const;

  int getDqBitsShiftEnd() const;

  int getQpBitsShiftStart() const;

  int getQpBitsShiftEnd() const;

  void generateK(std::vector<std::shared_ptr<KFactors_t>> &ks);

  static int getBitStartOffset(int start_offset);

  mpz_t n;
  mpz_t e;

private:

  static bool isInObservationPartition(unsigned int item_idx,
                                       uint8_t value, uint8_t **observation_partitions);

  bool checkDkAgainstObservation(mpz_t dk, LengthValues lengthValues);
  void generateKpKq(mpz_t k_p, mpz_t k_q, mpz_t k);

  // ToDo: Verify with modular multiplication
  static int quadraticResidue(mpz_t x, mpz_t q, mpz_t n);

  void calcDk(mpz_t dk, unsigned int k);

  static void loadParameter(Json::Value &source, std::vector<uint8_t> &target);

  static int getParameterBitShift(std::string shift);

  std::vector<uint8_t> p;
  int p_bits_shift_start;
  int p_bits_shift_end;
  std::vector<uint8_t> q;
  int q_bits_shift_start;
  int q_bits_shift_end;
  std::vector<uint8_t> d;
  int d_bits_shift_start;
  int d_bits_shift_end;
  std::vector<uint8_t> dp;
  int dp_bits_shift_start;
  int dp_bits_shift_end;
  std::vector<uint8_t> dq;
  int dq_bits_shift_start;
  int dq_bits_shift_end;
  std::vector<uint8_t> qp;
  int qp_bits_shift_start;
  int qp_bits_shift_end;

  uint8_t **observationPartitions;
  uint8_t **observationPartitions_lower2bit;
  uint8_t **observationPartitions_lower4bit;
  uint8_t **observationPartitions_upper2bit;
  uint8_t **observationPartitions_upper4bit;

  void read_partitions(const Json::Value &root, uint8_t **partitions, std::string prefix);
};



#endif //RECONSTRUCT_OBSERVATION_H

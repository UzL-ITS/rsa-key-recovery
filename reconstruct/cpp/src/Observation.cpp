//
// Created by florian on 10/10/22.
//

#include "Observation.h"

#include <fstream>

#include <json/json.h>

#include "Constants.h"
#include "Config.h"
#include "MathUtils.h"
#include "ParameterTerminationHandler.h"


const int Observation::NB_OF_OBSERVATION_PARTITIONS = Config::observationPartitions;

Observation::Observation()
{

  mpz_inits(n, e, NULL);
  observationPartitions = new uint8_t* [NB_OF_OBSERVATION_PARTITIONS];
  observationPartitions_lower2bit = new uint8_t* [NB_OF_OBSERVATION_PARTITIONS];
  observationPartitions_lower4bit = new uint8_t* [NB_OF_OBSERVATION_PARTITIONS];
  observationPartitions_upper2bit = new uint8_t* [NB_OF_OBSERVATION_PARTITIONS];
  observationPartitions_upper4bit = new uint8_t* [NB_OF_OBSERVATION_PARTITIONS];

  for (int i = 0; i < NB_OF_OBSERVATION_PARTITIONS; i++) {
    observationPartitions[i] = new uint8_t [64];
    observationPartitions_lower2bit[i] = new uint8_t [64];
    observationPartitions_lower4bit[i] = new uint8_t [64];
    observationPartitions_upper2bit[i] = new uint8_t [64];
    observationPartitions_upper4bit[i] = new uint8_t [64];
  }

  for (int i = 0; i < NB_OF_OBSERVATION_PARTITIONS; i++) {
    for (int j = 0; j < 64; j++) {
      observationPartitions[i][j] = 0xFF;
      observationPartitions_lower2bit[i][j] = 0xFF;
      observationPartitions_lower4bit[i][j] = 0xFF;
      observationPartitions_upper2bit[i][j] = 0xFF;
      observationPartitions_upper4bit[i][j] = 0xFF;
    }
  }

  Observation::load(Config::observationFileName);
}

Observation::~Observation() {
  for (int i = 0; i < NB_OF_OBSERVATION_PARTITIONS; i++) {
    delete [] observationPartitions[i];
  }
  delete [] observationPartitions;
  mpz_clears(n, e, NULL);
}

void Observation::load(std::string file_name) {
  std::ifstream ifs(file_name);
  Json::Value root;

  ifs >> root;

  std::string nString = root["n"].asString();
  nString = nString.substr(2);
  std::string eString = root["e"].asString();
  eString = eString.substr(2);

  mpz_set_str(this->n, nString.c_str(), 10);
  mpz_set_str(this->e, eString.c_str(), 10);

  read_partitions(root, observationPartitions, "line");
  read_partitions(root, observationPartitions_lower2bit, "line_lower_2_");
  read_partitions(root, observationPartitions_lower4bit, "line_lower_4_");
  read_partitions(root, observationPartitions_upper2bit, "line_upper_2_");
  read_partitions(root, observationPartitions_upper4bit, "line_upper_4_");

  loadParameter(root["enc_p"], this->p);
  this->p_bits_shift_start = getParameterBitShift(root["enc_p_start_shift"].asString());
  this->p_bits_shift_end = getParameterBitShift(root["enc_p_end_shift"].asString());

  loadParameter(root["enc_q"], this->q);
  this->q_bits_shift_start = getParameterBitShift(root["enc_q_start_shift"].asString());
  this->q_bits_shift_end = getParameterBitShift(root["enc_q_end_shift"].asString());

  loadParameter(root["enc_d"], this->d);
  this->d_bits_shift_start = getParameterBitShift(root["enc_d_start_shift"].asString());
  this->d_bits_shift_end = getParameterBitShift(root["enc_d_end_shift"].asString());

  loadParameter(root["enc_dp"], this->dp);
  this->dp_bits_shift_start = getParameterBitShift(root["enc_dp_start_shift"].asString());
  this->dp_bits_shift_end = getParameterBitShift(root["enc_dp_end_shift"].asString());

  loadParameter(root["enc_dq"], this->dq);
  this->dq_bits_shift_start = getParameterBitShift(root["enc_dq_start_shift"].asString());
  this->dq_bits_shift_end = getParameterBitShift(root["enc_dq_end_shift"].asString());

  loadParameter(root["enc_qp"], this->qp);
  this->qp_bits_shift_start = getParameterBitShift(root["enc_qp_start_shift"].asString());
  this->qp_bits_shift_end = getParameterBitShift(root["enc_qp_end_shift"].asString());
}

void Observation::read_partitions(const Json::Value &root, uint8_t **partitions, std::string prefix) {
  Json::Value items [NB_OF_OBSERVATION_PARTITIONS];
  for (int i = 0; i < NB_OF_OBSERVATION_PARTITIONS; i++) {
    items[i] = root[prefix + std::to_string(i + 1)];
  }

  for (int partitions_idx = 0; partitions_idx < NB_OF_OBSERVATION_PARTITIONS; partitions_idx++) {
    for (int i = 0; i < items[partitions_idx].size(); i++) {
      int index = items[partitions_idx][i].asInt();
      if(index < 64) {
        partitions[partitions_idx][index] = 0;
      }
    }
  }
}

void Observation::loadParameter(Json::Value &source, std::vector<uint8_t> &target) {
  for (int i = 0; i < source.size(); i++) {
    uint8_t val = static_cast<uint8_t> (source[i].asInt());
    target.push_back(val);
  }
}

bool Observation::isInObservationPartition(unsigned int item_idx,
                                           uint8_t value) {
  return isInObservationPartition(item_idx, value, observationPartitions);
}

bool Observation::isInObservationPartitionLower4(unsigned int item_idx, uint8_t value) {
  return isInObservationPartition(item_idx, value, observationPartitions_lower4bit);
}

bool Observation::isInObservationPartitionLower2(unsigned int item_idx, uint8_t value) {
  return isInObservationPartition(item_idx, value, observationPartitions_lower2bit);
}

bool Observation::isInObservationPartitionUpper4(unsigned int item_idx, uint8_t value) {
  return isInObservationPartition(item_idx, value, observationPartitions_upper4bit);
}

bool Observation::isInObservationPartitionUpper2(unsigned int item_idx, uint8_t value) {
  return isInObservationPartition(item_idx, value, observationPartitions_upper2bit);
}

bool Observation::isInObservationPartition(unsigned int item_idx,
                                           uint8_t value,
                                           uint8_t **observation_partitions) {


  if(item_idx == Constants::INVALID_OBSERVATION) {
    return true;
  }

  if (value < 64 && item_idx < NB_OF_OBSERVATION_PARTITIONS) {

    if (observation_partitions[item_idx][value] == 0) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

uint8_t Observation::getPAt(unsigned int index) {
  size_t size = p.size();
  if(index < size) {
    return p.at(size - index - 1);
  } else {
    return 0xFF;
  }
}

uint8_t Observation::getQAt(unsigned int index) {
  size_t size = q.size();
  if(index < size) {
    return q.at(size - index - 1);
  } else {
    return 0xFF;
  }
}

uint8_t Observation::getDpAt(unsigned int index) {
  size_t size = dp.size();
  if(index < size) {
    return dp.at(size - index - 1);
  } else {
    return 0xFF;
  }
}

uint8_t Observation::getDAt(unsigned int index) {
  size_t size = d.size();
  if(index < size) {
    return d.at(size - index - 1);
  } else {
    return 0xFF;
  }
}

uint8_t Observation::getDqAt(unsigned int index) {
  size_t size = dq.size();
  if(index < size) {
    return dq.at(size - index - 1);
  } else {
    return 0xFF;
  }
}

// ToDo
bool Observation::checkDkAgainstObservation(mpz_t dk, LengthValues lengthValues) {
  size_t number_of_msb_to_check;
  mpz_t dk_block, mask;
  mpz_inits(dk_block, mask, NULL);
  bool returnVal = false;

  number_of_msb_to_check = mpz_sizeinbase(n, 2);
  number_of_msb_to_check = ((number_of_msb_to_check / 2) - 2) / 6;


  size_t dk_bit_length = mpz_sizeinbase(dk, 2);
  size_t dk_bit_length_byte_aligned = dk_bit_length + ((8 - (lengthValues.getLengthOfD() % 8)) % 8);
  size_t pos = 0;

  unsigned int bit_offset = 0;
  uint8_t **partial_observationPartitions = observationPartitions;
  if (d_bits_shift_end == LOWER_2_BITS) {
    bit_offset = 2;
    partial_observationPartitions = observationPartitions_lower2bit;
  } else if (d_bits_shift_end == LOWER_4_BITS) {
    bit_offset = 4;
    partial_observationPartitions = observationPartitions_lower4bit;
  }

  pos = dk_bit_length_byte_aligned - bit_offset;

  mpz_set_ui(mask, 0x3F);
  mpz_mul_2exp(mask, mask, pos);

  // We need a list of lists as encoding with partial observation blocks is not
  // unambiguous
  std::vector<std::vector<uint8_t>> dk_cache_line_enc;

  while (pos >= Constants::BITS_PER_B64_SYMBOL) {

    uint8_t dk_block_ui;

    // ToDo: Does this also work as expected when bit_len(mask) > bit_len(dk) ?
    //  I think: Yes!
    mpz_and(dk_block, dk, mask);
    mpz_tdiv_q_2exp(dk_block, dk_block, pos);
    dk_block_ui = static_cast<uint8_t> (mpz_get_ui(dk_block));

    bool item_found = false;
    for (int observation_partitions_idx = 0;
         observation_partitions_idx < NB_OF_OBSERVATION_PARTITIONS;
         observation_partitions_idx++) {
      if (pos == (dk_bit_length_byte_aligned - bit_offset)) {
        if (partial_observationPartitions[observation_partitions_idx][dk_block_ui] == 0) {
          if (dk_cache_line_enc.empty()) {
            dk_cache_line_enc.push_back(std::vector<uint8_t>());
          }
          dk_cache_line_enc[0].push_back(observation_partitions_idx);
          item_found = true;
        }
      } else {
        if (observationPartitions[observation_partitions_idx][dk_block_ui] == 0) {
          std::vector<uint8_t> obs;
          obs.push_back(observation_partitions_idx);
          dk_cache_line_enc.push_back(obs);
          item_found = true;
          break;
        }
      }
    }
    if (!item_found) {
      std::cerr << "Invalid encoding for dk" << std::endl;
    }

    mpz_tdiv_q_2exp(mask, mask, Constants::BITS_PER_B64_SYMBOL);
    pos -= Constants::BITS_PER_B64_SYMBOL;
  }

  if (number_of_msb_to_check < d.size()) {
    returnVal = true;
    for (int i = 0; i < number_of_msb_to_check; i++) {
      if (d.at(i) != Constants::INVALID_OBSERVATION) {
        bool found = false;
        for (auto element : dk_cache_line_enc.at(i)) {
          if (element == d.at(i)) {
            found = true;
            break;
          }
        }
        if (!found) {
          returnVal = false;
          break;
        }
      }
    }
  }

  mpz_clears(dk_block, mask, NULL);
  return returnVal;
}

void Observation::generateK(std::vector<std::shared_ptr<KFactors_t>> &ks) {
  unsigned int e = mpz_get_ui(this->e);
  LengthValues lengthValues;

  for (unsigned int k = 1; k < Constants::GAMMA * e; k++) {
    mpz_t dk;
    mpz_init(dk);
    calcDk(dk, k);

    if (checkDkAgainstObservation(dk, lengthValues)) {
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


void Observation::calcDk(mpz_t dk, unsigned int k) {
  mpz_add_ui(dk, this->n, 1);
  mpz_mul_ui(dk, dk, k);
  mpz_add_ui(dk, dk, Constants::GAMMA);

  mpq_t dk_rational;
  mpq_init(dk_rational);
  mpq_set_z(dk_rational, dk);
  mpq_t gamma_e_rational;
  mpq_init(gamma_e_rational);
  mpz_t gamma_e;
  mpz_init(gamma_e);
  mpz_mul_ui(gamma_e, this->e, Constants::GAMMA);
  mpq_set_z(gamma_e_rational, gamma_e);

  mpq_div(dk_rational, dk_rational, gamma_e_rational);

  mpz_set_q(dk, dk_rational);
  mpq_clears(dk_rational, gamma_e_rational, NULL);
}

void Observation::generateKpKq(mpz_t k_p, mpz_t k_q, mpz_t k) {
  mpz_t solutionOne, solutionTwo, intermediate;
  mpz_inits(solutionOne, solutionTwo, intermediate, NULL);

  // Compute k * (N - 1) + 1 * \gamma
  mpz_mod(solutionOne, k, this->e);
  mpz_mod(intermediate, this->n, this->e);
  mpz_sub_ui(intermediate, intermediate, 1);
  mpz_mul(solutionOne, solutionOne, intermediate);
  mpz_add_ui(solutionOne, solutionOne, 1 * Constants::GAMMA);
  mpz_mod(solutionOne, solutionOne, this->e);


  // Compute \sqrt{ [k * (N - 1) + 1 * \gamma]^2 + 4 * k * \gamma }
  mpz_sub_ui(solutionTwo, this->n, 1);
  mpz_mul(solutionTwo, solutionTwo, k);
  mpz_add_ui(solutionTwo, solutionTwo, 1 * Constants::GAMMA);
  mpz_mul(solutionTwo, solutionTwo, solutionTwo);
  mpz_mul_ui(intermediate, k, 4 * Constants::GAMMA);
  mpz_add(solutionTwo, solutionTwo, intermediate);
  quadraticResidue(solutionTwo, solutionTwo, this->e);


  mpz_init(intermediate);
  mpz_add_ui(intermediate, intermediate, 2 * Constants::GAMMA);
  MathUtils::modularInverse(intermediate, intermediate, this->e);


  mpz_add(k_p, solutionOne, solutionTwo);
  mpz_mul(k_p, k_p, intermediate);
  mpz_mod(k_p, k_p, this->e);

  mpz_sub(k_q, solutionOne, solutionTwo);
  mpz_mul(k_q, k_q, intermediate);
  mpz_mod(k_q, k_q, this->e);

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
int Observation::quadraticResidue(mpz_t x, mpz_t q, mpz_t n) {
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

int Observation::getBitStartOffset(int start_offset) {
  if(start_offset == UPPER_2_BITS) {
    return 2;
  } else if (start_offset == UPPER_4_BITS) {
    return 4;
  } else {
    return 0;
  }
}

int Observation::getParameterBitShift(std::string shift) {
  if (shift.compare("upper_2") == 0) {
    // -4 because we do not consider the lower 4 bit
    return UPPER_2_BITS;
  } else if (shift.compare("upper_4") == 0) {
    // -2 because we do not consider the lower 2 bit
    return UPPER_4_BITS;
  } else if (shift.compare("lower_2") == 0) {
    return LOWER_2_BITS;
  } else if (shift.compare("lower_4") == 0) {
    return LOWER_4_BITS;
  } else {
    return NO_SHIFT;
  }
}

int Observation::getPBitsShiftStart() const {
  return p_bits_shift_start;
}

int Observation::getPBitsShiftEnd() const {
  return p_bits_shift_end;
}

int Observation::getQBitsShiftStart() const {
  return q_bits_shift_start;
}

int Observation::getQBitsShiftEnd() const {
  return q_bits_shift_end;
}

int Observation::getDBitsShiftStart() const {
  return d_bits_shift_start;
}

int Observation::getDBitsShiftEnd() const {
  return d_bits_shift_end;
}

int Observation::getDpBitsShiftStart() const {
  return dp_bits_shift_start;
}

int Observation::getDpBitsShiftEnd() const {
  return dp_bits_shift_end;
}

int Observation::getDqBitsShiftStart() const {
  return dq_bits_shift_start;
}

int Observation::getDqBitsShiftEnd() const {
  return dq_bits_shift_end;
}

int Observation::getQpBitsShiftStart() const {
  return qp_bits_shift_start;
}

int Observation::getQpBitsShiftEnd() const {
  return qp_bits_shift_end;
}

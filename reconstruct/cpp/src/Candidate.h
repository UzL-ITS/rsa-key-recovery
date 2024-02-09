//
// Created by florian on 27.08.20.
//

#ifndef RECONSTRUCT_CANDIDATE_H
#define RECONSTRUCT_CANDIDATE_H


#include <vector>
#include <cstdint>

#include <boost/multiprecision/gmp.hpp>
#include <json/value.h>

#include "Observation.h"
#include "ParameterTerminationHandler.h"

using namespace boost::multiprecision;

class Candidate {

public:

  Candidate(mpz_t p, mpz_t q, mpz_t d, mpz_t dp, mpz_t dq, mpz_t k, mpz_t kP, mpz_t kQ,
            unsigned int tau_k, unsigned int tau_kp, unsigned int tau_kq, unsigned int depth,
            unsigned int tau_gamma, std::shared_ptr<ParameterTerminationHandler> tHandler);

  Candidate(mpz_t k, mpz_t kP, mpz_t kQ, mpz_t p, mpz_t q, mpz_t d, Observation &o,
            std::shared_ptr<ParameterTerminationHandler> tHandler);

  virtual ~Candidate();

  bool check(Observation &o);

  unsigned int getDepth() const;

  bool checkCandidateExtension(uint8_t p_bit, uint8_t q_bit, uint8_t d_bit, uint8_t dp_bit,
                               uint8_t dq_bit, Observation &o);

  std::shared_ptr<Candidate> extend(uint8_t p_bit, uint8_t q_bit,
                                    uint8_t d_bit, uint8_t dp_bit, uint8_t dq_bit);

  void setTerminationHandler(std::shared_ptr<ParameterTerminationHandler> tHandler);
  const std::shared_ptr<ParameterTerminationHandler> getTerminationHandler() const;

  unsigned int getTauK() const;

  unsigned int getTauKp() const;

  unsigned int getTauKq() const;

  unsigned int getTauGamma() const;

  std::string getP();


  void toJson(Json::Value &value);

private:

  void extractBlockFromP(mpz_t blockP, bool terminalValue);

  void extractBlockFromQ(mpz_t blockQ, bool terminalValue);

  void extractBlockFromD(mpz_t blockD, bool terminalValue);

  void extractBlockFromDP(mpz_t blockDP, bool terminalValue);

  void extractBlockFromDQ(mpz_t blockDQ, bool terminalValue);

  mpz_t p;
  mpz_t q;
  mpz_t d;
  mpz_t dp;
  mpz_t dq;
  mpz_t qp;

  mpz_t k;
  mpz_t k_p;
  mpz_t k_q;

  unsigned int tau_k;
  unsigned int tau_kp;
  unsigned int tau_kq;
  unsigned int tau_gamma;

  unsigned int depth;

  std::shared_ptr<ParameterTerminationHandler> tHandler;

private:

  static bool checkCandidateParameterBlockAgainstObservation(
      mpz_t candidateParameterBlock,
      uint8_t observationParameterValue,
      Observation &o);

  bool checkCandidateExtension_term1(uint8_t p_bit, uint8_t q_bit, const Observation &o) const;

  bool
  checkCandidateExtension_term2(uint8_t bit, uint8_t bit1, uint8_t bit2, Observation &observation);

  bool checkCandidateExtension_term3(uint8_t bit, uint8_t bit1, Observation &observation);

  bool checkCandidateExtension_term4(uint8_t bit, uint8_t bit1, Observation &observation);

  void checkTerminalValue(bool &terminalValueP, bool &terminalValueQ, bool &terminalValueD,
                          bool &terminalValueDp, bool &terminalValueDq) const;

  bool validateShortestExpandParameter() const;

  static void extractPartialStartBlockFrom(mpz_t block, mpz_t parameter, int bits);

  static bool checkCandidateParameterBlockAgainstObservationUpper4Bit(mpz_t val, uint8_t at, Observation &observation);

  static bool checkCandidateParameterBlockAgainstObservationUpper2Bit(mpz_t val, uint8_t at, Observation &observation);
};


#endif //RECONSTRUCT_CANDIDATE_H

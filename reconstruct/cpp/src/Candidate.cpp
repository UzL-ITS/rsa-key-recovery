//
// Created by florian on 27.08.20.
//

#include "Candidate.h"
#include "Constants.h"
#include "BinaryHelper.h"
#include "MathUtils.h"
#include "SimpleExampleTerminationHandler.h"
#include "ValidationValuesFactory.h"

Candidate::Candidate(mpz_t p, mpz_t q, mpz_t d, mpz_t dp, mpz_t dq, mpz_t k, mpz_t kP, mpz_t kQ,
                     unsigned int tau_k, unsigned int tau_kp, unsigned int tau_kq,
                     unsigned int depth,
                     std::shared_ptr<ParameterTerminationHandler> tHandler) {

  mpz_inits(this->p, this->q, this->d, this->dp, this->dq, this->qp, this->k, this->k_p, this->k_q,
            NULL);

  mpz_set(this->p, p);
  mpz_set(this->q, q);
  mpz_set(this->d, d);
  mpz_set(this->dp, dp);
  mpz_set(this->dq, dq);
  mpz_set(this->k, k);
  mpz_set(this->k_p, kP);
  mpz_set(this->k_q, kQ);
  this->tau_k = tau_k;
  this->tau_kp = tau_kp;
  this->tau_kq = tau_kq;
  this->depth = depth;

  this->tHandler = tHandler;
}

Candidate::Candidate(mpz_t k, mpz_t kP, mpz_t kQ, CacheLineObservation &o,
                     std::shared_ptr<ParameterTerminationHandler> tHandler) : depth(1) {

  mpz_inits(this->p, this->q, this->d, this->dp, this->dq, this->qp, this->k, this->k_p, this->k_q,
            NULL);

  mpz_t intermediate;
  mpz_init(intermediate);

  mpz_set_ui(this->p, 1);
  mpz_set_ui(this->q, 1);
  mpz_set(this->k, k);

  this->tau_k = MathUtils::tau(k);

  mpz_ui_pow_ui(intermediate, 2, 1 + this->tau_k);
  MathUtils::modularInverse(this->d, o.e, intermediate);

  mpz_set(this->k_p, kP);
  this->tau_kp = MathUtils::tau(kP);

  mpz_ui_pow_ui(intermediate, 2, 1 + this->tau_kp);
  MathUtils::modularInverse(this->dp, o.e, intermediate);

  mpz_set(this->k_q, kQ);
  this->tau_kq = MathUtils::tau(kQ);

  mpz_ui_pow_ui(intermediate, 2, 1 + this->tau_kq);
  MathUtils::modularInverse(this->dq, o.e, intermediate);

  this->tHandler = tHandler;

  mpz_clear(intermediate);
}

Candidate::~Candidate() {
  mpz_clears(this->p, this->q, this->d, this->dp, this->dq, this->qp, this->k, this->k_p, this->k_q,
             NULL);
}


bool Candidate::check(CacheLineObservation &o) {
  bool result = false;
  mpz_t val;
  mpz_init(val);

  if (this->depth == (tHandler->getExpandLengthForShortestParam() + 1)) {
    result = validateShortestParameter();
  } else if (this->depth % Constants::BITS_PER_B64_SYMBOL == 0) {

    bool terminalValueP = false;
    bool terminalValueQ = false;
    bool terminalValueD = false;
    bool terminalValueDp = false;
    bool terminalValueDq = false;

    unsigned int observationIndex = (this->depth - 1) / Constants::BITS_PER_B64_SYMBOL;
    checkTerminalValue(terminalValueP, terminalValueQ, terminalValueD, terminalValueDp,
                       terminalValueDq);

    extractBlockFromP(val, terminalValueP);
    result = checkCandidateParameterBlockAgainstObservation(val, o.getPAt(observationIndex), o);
    if (!result) {
      mpz_clear(val);
      return result;
    }

    extractBlockFromQ(val, terminalValueQ);
    result = checkCandidateParameterBlockAgainstObservation(val, o.getQAt(observationIndex), o);
    if (!result) {
      mpz_clear(val);
      return result;
    }

    extractBlockFromD(val, terminalValueD);
    result = checkCandidateParameterBlockAgainstObservation(val, o.getDAt(observationIndex), o);
    if (!result) {
      mpz_clear(val);
      return result;
    }

    extractBlockFromDP(val, terminalValueDp);
    result = checkCandidateParameterBlockAgainstObservation(val, o.getDpAt(observationIndex), o);
    if (!result) {
      mpz_clear(val);
      return result;
    }

    extractBlockFromDQ(val, terminalValueDq);
    result = checkCandidateParameterBlockAgainstObservation(val, o.getDqAt(observationIndex), o);
    if (!result) {
      mpz_clear(val);
      return result;
    }
  } else if (this->depth > tHandler->getMaxDepth()) {
    result = false;
  } else {
    result = true;
  }

  mpz_clear(val);
  return result;
}

bool Candidate::validateShortestParameter() const {
  bool result;
  ParameterTerminationHandler::ParameterName shortestParameterName
      = tHandler->getShortestParam();
  auto validationValues = ValidationValuesFactory::getValidationValues();

  if (shortestParameterName == ParameterTerminationHandler::P) {
    if(mpz_cmp(validationValues->p, p) == 0) {
      result = true;
    } else {
      result = false;
    }
  } else if (shortestParameterName == ParameterTerminationHandler::Q) {
    if(mpz_cmp(validationValues->q, q) == 0) {
      result = true;
    } else {
      result = false;
    }
  } else if (shortestParameterName == ParameterTerminationHandler::D) {
    if(mpz_cmp(validationValues->d, d) == 0) {
      result = true;
    } else {
      result = false;
    }
  } else if (shortestParameterName == ParameterTerminationHandler::Dp) {
    if(mpz_cmp(validationValues->dp, dp) == 0) {
      result = true;
    } else {
      result = false;
    }
  } else if (shortestParameterName == ParameterTerminationHandler::Dq) {
    if(mpz_cmp(validationValues->dq, dq) == 0) {
      result = true;
    } else {
      result = false;
    }
  } else {
    result = false;
  }
  return result;
}

void Candidate::checkTerminalValue(bool &terminalValueP, bool &terminalValueQ, bool &terminalValueD,
                                   bool &terminalValueDp, bool &terminalValueDq) const {
  if (depth >= tHandler->getShortestParamLength()) {
    if (depth >= tHandler->getLengthOfP()) {
      terminalValueP = true;
    }
    if (depth >= tHandler->getLengthOfQ()) {
      terminalValueQ = true;
    }
    if (depth >= tHandler->getLengthOfD()) {
      terminalValueD = true;
    }
    if (depth >= tHandler->getLengthOfDp()) {
      terminalValueDp = true;
    }
    if (depth >= tHandler->getLengthOfDq()) {
      terminalValueDq = true;
    }
  }
}

unsigned int Candidate::getDepth() const {
  return depth;
}

bool Candidate::checkCandidateParameterBlockAgainstObservation(
    mpz_t candidateParameterBlock,
    uint8_t observationValue, CacheLineObservation &o) {

  unsigned long cpb = mpz_get_ui(candidateParameterBlock);
  if (observationValue > 1) {
    return false;
  }

  return o.isInCacheLine(static_cast<CacheLineObservation::CacheLines>(observationValue),
                         cpb);
}

void Candidate::extractBlockFromP(mpz_t blockP,
                                  bool terminalValue) {
  if (!terminalValue) {
    size_t bit_length = mpz_sizeinbase(this->p, 2);
    BinaryHelper::most_significant_bits(blockP, this->p,
                                        Constants::BITS_PER_B64_SYMBOL + bit_length - depth);
  } else {
    tHandler->getTerminalValue(blockP, this->p, depth);
  }
}

void Candidate::extractBlockFromQ(mpz_t blockQ,
                                  bool terminalValue) {
  if (!terminalValue) {
    size_t bit_length = mpz_sizeinbase(this->q, 2);
    BinaryHelper::most_significant_bits(blockQ, this->q,
                                        Constants::BITS_PER_B64_SYMBOL + bit_length - depth);
  } else {
    tHandler->getTerminalValue(blockQ, this->q, depth);
  }
}

void Candidate::extractBlockFromD(mpz_t blockD,
                                  bool terminalValue) {
  if (!terminalValue) {
    size_t bit_length = mpz_sizeinbase(this->d, 2);
    BinaryHelper::most_significant_bits(blockD, this->d,
                                        Constants::BITS_PER_B64_SYMBOL + bit_length - depth);
    BinaryHelper::least_significant_bits(blockD, blockD, Constants::BITS_PER_B64_SYMBOL);
  } else {
    tHandler->getTerminalValue(blockD, this->d, depth);
  }
}

void Candidate::extractBlockFromDP(mpz_t blockDP,
                                   bool terminalValue) {
  if (!terminalValue) {
    size_t bit_length = mpz_sizeinbase(this->dp, 2);
    BinaryHelper::most_significant_bits(blockDP, this->dp,
                                        Constants::BITS_PER_B64_SYMBOL + bit_length - depth);
    BinaryHelper::least_significant_bits(blockDP, blockDP, Constants::BITS_PER_B64_SYMBOL);
  } else {
    tHandler->getTerminalValue(blockDP, this->dp, depth);
  }
}

void Candidate::extractBlockFromDQ(mpz_t blockDQ,
                                   bool terminalValue) {
  if (!terminalValue) {
    size_t bit_length = mpz_sizeinbase(this->dq, 2);
    BinaryHelper::most_significant_bits(blockDQ, this->dq,
                                        Constants::BITS_PER_B64_SYMBOL + bit_length - depth);
    BinaryHelper::least_significant_bits(blockDQ, blockDQ, Constants::BITS_PER_B64_SYMBOL);
  } else {
    tHandler->getTerminalValue(blockDQ, this->dq, depth);
  }
}

std::shared_ptr<Candidate>
Candidate::extend(uint8_t p_bit, uint8_t q_bit, uint8_t d_bit, uint8_t dp_bit, uint8_t dq_bit,
                  CacheLineObservation &o) {

  mpz_t new_p, new_q, new_d, new_dp, new_dq, tmp;
  mpz_inits(new_p, new_q, new_d, new_dp, new_dq, tmp, NULL);

  mpz_ui_pow_ui(tmp, 2, depth);

  mpz_mul_ui(new_p, tmp, p_bit);
  mpz_add(new_p, p, new_p);

  mpz_mul_ui(new_q, tmp, q_bit);
  mpz_add(new_q, q, new_q);

  mpz_ui_pow_ui(tmp, 2, depth + tau_k);
  mpz_mul_ui(new_d, tmp, d_bit);
  mpz_add(new_d, d, new_d);

  mpz_ui_pow_ui(tmp, 2, depth + tau_kp);
  mpz_mul_ui(new_dp, tmp, dp_bit);
  mpz_add(new_dp, dp, new_dp);

  mpz_ui_pow_ui(tmp, 2, depth + tau_kq);
  mpz_mul_ui(new_dq, tmp, dq_bit);
  mpz_add(new_dq, dq, new_dq);

  auto newCandidate = std::make_shared<Candidate>(new_p, new_q, new_d, new_dp, new_dq,
                                                  k, k_p, k_q, tau_k, tau_kp, tau_kq, depth + 1,
                                                  tHandler);

  mpz_clears(new_p, new_q, new_d, new_dp, new_dq, tmp, NULL);
  return newCandidate;
}

bool Candidate::checkCandidateExtension(uint8_t p_bit, uint8_t q_bit, uint8_t d_bit, uint8_t dp_bit,
                                        uint8_t dq_bit, CacheLineObservation &o) {
  if (!checkCandidateExtension_term1(p_bit, q_bit, o)) {
    return false;
  }
  if (!checkCandidateExtension_term2(p_bit, q_bit, d_bit, o)) {
    return false;
  }
  if (!checkCandidateExtension_term3(p_bit, dp_bit, o)) {
    return false;
  }
  if (!checkCandidateExtension_term4(q_bit, dq_bit, o)) {
    return false;
  }

  return true;
}

bool
Candidate::checkCandidateExtension_term1(uint8_t p_bit, uint8_t q_bit,
                                         const CacheLineObservation &o) const {
  mpz_t term;
  mpz_init(term);
  mpz_mul(term, p, q);
  mpz_sub(term, o.n, term);

  uint8_t newVal = (p_bit + q_bit) % 2;
  uint8_t expectedVal = mpz_tstbit(term, depth);

  mpz_clear(term);
  return (newVal == expectedVal);
}

bool Candidate::checkCandidateExtension_term2(uint8_t p_bit, uint8_t q_bit, uint8_t d_bit,
                                              CacheLineObservation &o) {
  mpz_t term, tmp;
  mpz_init(term);
  mpz_init(tmp);

  mpz_add_ui(term, o.n, 1);
  mpz_mul(term, term, k);
  mpz_add_ui(term, term, 1);

  mpz_add(tmp, p, q);
  mpz_mul(tmp, tmp, k);
  mpz_sub(term, term, tmp);

  mpz_mul(tmp, o.e, d);
  mpz_sub(term, term, tmp);

  uint8_t newVal = (p_bit + q_bit + d_bit) % 2;
  uint8_t expectedVal = mpz_tstbit(term, depth + tau_k);

  mpz_clears(term, tmp, NULL);
  return (newVal == expectedVal);
}

bool
Candidate::checkCandidateExtension_term3(uint8_t p_bit, uint8_t dp_bit, CacheLineObservation &o) {
  mpz_t term, tmp;
  mpz_init(term);
  mpz_init(tmp);

  mpz_sub_ui(term, p, 1);
  mpz_mul(term, term, k_p);
  mpz_add_ui(term, term, 1);

  mpz_mul(tmp, o.e, dp);

  mpz_sub(term, term, tmp);

  uint8_t newVal = (p_bit + dp_bit) % 2;
  uint8_t expectedVal = mpz_tstbit(term, depth + tau_kp);

  mpz_clears(term, tmp, NULL);
  return (newVal == expectedVal);
}

bool
Candidate::checkCandidateExtension_term4(uint8_t q_bit, uint8_t dq_bit, CacheLineObservation &o) {
  mpz_t term, tmp;
  mpz_init(term);
  mpz_init(tmp);

  mpz_sub_ui(term, q, 1);
  mpz_mul(term, term, k_q);
  mpz_add_ui(term, term, 1);

  mpz_mul(tmp, o.e, dq);

  mpz_sub(term, term, tmp);

  uint8_t newVal = (q_bit + dq_bit) % 2;
  uint8_t expectedVal = mpz_tstbit(term, depth + tau_kq);

  mpz_clears(term, tmp, NULL);
  return (newVal == expectedVal);
}

void Candidate::toJson(Json::Value &value) {

  char *tmp = mpz_get_str(nullptr, 10, this->p);
  value["p"] = std::string(tmp);
  free(tmp);

  tmp = mpz_get_str(nullptr, 10, this->q);
  value["q"] = std::string(tmp);
  free(tmp);

  tmp = mpz_get_str(nullptr, 10, this->d);
  value["d"] = std::string(tmp);
  free(tmp);

  tmp = mpz_get_str(nullptr, 10, this->d);
  value["d"] = std::string(tmp);
  free(tmp);

  tmp = mpz_get_str(nullptr, 10, this->dp);
  value["dp"] = std::string(tmp);
  free(tmp);

  tmp = mpz_get_str(nullptr, 10, this->dq);
  value["dq"] = std::string(tmp);
  free(tmp);

  tmp = mpz_get_str(nullptr, 10, this->qp);
  value["qp"] = std::string(tmp);
  free(tmp);

  tmp = mpz_get_str(nullptr, 10, this->k);
  value["k"] = std::string(tmp);
  free(tmp);

  tmp = mpz_get_str(nullptr, 10, this->k_p);
  value["k_p"] = std::string(tmp);
  free(tmp);

  tmp = mpz_get_str(nullptr, 10, this->k_q);
  value["k_q"] = std::string(tmp);
  free(tmp);

  value["tau_k"] = this->tau_k;
  value["tau_kp"] = this->tau_kp;
  value["tau_kq"] = this->tau_kq;

  value["depth"] = this->depth;
}

void Candidate::setTerminationHandler(std::shared_ptr<ParameterTerminationHandler> tHandler) {
  this->tHandler = tHandler;
}

const std::shared_ptr<ParameterTerminationHandler> Candidate::getTerminationHandler() const {
  return tHandler;
}

unsigned int Candidate::getTauK() const {
  return tau_k;
}

unsigned int Candidate::getTauKp() const {
  return tau_kp;
}

unsigned int Candidate::getTauKq() const {
  return tau_kq;
}

std::string Candidate::getP() {
  char *p = mpz_get_str(NULL, 10, this->p);
  std::string p_str(p);
  free(p);
  return p_str;
}

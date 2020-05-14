//
// Created by florian on 27.08.20.
//

#include "CandidateGenerator.h"

CandidateGenerator::CandidateGenerator() {
}

CandidateGenerator::~CandidateGenerator() {

}

std::vector<std::shared_ptr<Candidate>> CandidateGenerator::generateStartCandidates(
    CacheLineObservation &o) {

  std::vector<std::shared_ptr<Candidate>> candidates;

  std::vector<std::shared_ptr<CacheLineObservation::KFactors_t>> kFactors;
  o.generateK(kFactors);

  for (std::vector<std::shared_ptr<CacheLineObservation::KFactors_t>>::iterator iter = kFactors.begin();
      iter != kFactors.end(); iter++) {
    auto candidate = std::make_shared<Candidate>((*iter)->k, (*iter)->k_p, (*iter)->k_q, o, nullptr);
    candidates.push_back(candidate);
  }

  return candidates;
}

std::vector<std::shared_ptr<Candidate>> CandidateGenerator::expandCandidate(
    Candidate &c, CacheLineObservation &o) {
  std::vector<std::shared_ptr<Candidate>> newCandidates;

  for (uint8_t p = 0; p < 2; p++) {
    for (uint8_t q = 0; q < 2; q++) {
      for (uint8_t d = 0; d < 2; d++) {
        for (uint8_t dp = 0; dp < 2; dp++) {
          for (uint8_t dq = 0; dq < 2; dq++) {

            if (c.checkCandidateExtension(p, q, d, dp, dq, o)) {
              std::shared_ptr<Candidate> candidate = c.extend(p, q, d, dp, dq, o);
              newCandidates.push_back(candidate);
            }
          }
        }
      }
    }
  }

  return newCandidates;
}



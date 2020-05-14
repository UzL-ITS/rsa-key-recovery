//
// Created by florian on 27.08.20.
//

#ifndef RECONSTRUCT_CANDIDATEGENERATOR_H
#define RECONSTRUCT_CANDIDATEGENERATOR_H


#include <vector>
#include "Candidate.h"
#include "CacheLineObservation.h"
#include "TerminationHandlerFactory.h"

class CandidateGenerator {
public:
  CandidateGenerator();

  virtual ~CandidateGenerator();

  std::vector<std::shared_ptr<Candidate>> generateStartCandidates(CacheLineObservation &o);
  std::vector<std::shared_ptr<Candidate>> expandCandidate(Candidate &c, CacheLineObservation &o);
};


#endif //RECONSTRUCT_CANDIDATEGENERATOR_H

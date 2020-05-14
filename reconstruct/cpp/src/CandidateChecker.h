//
// Created by florian on 27.08.20.
//

#ifndef RECONSTRUCT_CANDIDATECHECKER_H
#define RECONSTRUCT_CANDIDATECHECKER_H


#include "Candidate.h"
#include "CacheLineObservation.h"

class CandidateChecker {
public:
  CandidateChecker();

  virtual ~CandidateChecker();

  static bool checkCandidate(Candidate &c, CacheLineObservation &o);
};


#endif //RECONSTRUCT_CANDIDATECHECKER_H

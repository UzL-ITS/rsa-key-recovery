//
// Created by florian on 27.08.20.
//

#ifndef RECONSTRUCT_CANDIDATECHECKER_H
#define RECONSTRUCT_CANDIDATECHECKER_H


#include "Candidate.h"
#include "Observation.h"

class CandidateChecker {
public:
  CandidateChecker();

  virtual ~CandidateChecker();

  static bool checkCandidate(Candidate &c, Observation &o);
};


#endif //RECONSTRUCT_CANDIDATECHECKER_H

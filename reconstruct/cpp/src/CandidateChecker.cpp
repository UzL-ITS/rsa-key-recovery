//
// Created by florian on 27.08.20.
//

#include "CandidateChecker.h"

CandidateChecker::CandidateChecker() {}

CandidateChecker::~CandidateChecker() {

}

bool CandidateChecker::checkCandidate(Candidate &c, Observation &o) {
  return c.check(o);
}

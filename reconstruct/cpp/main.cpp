#include <iostream>
#include <vector>
#include <fstream>

#include <json/json.h>

#include <future>
#include <mutex>

#include "src/Candidate.h"
#include "src/CandidateGenerator.h"
#include "src/CacheLineObservation.h"
#include "src/CandidateChecker.h"
#include "src/Config.h"
#include "src/CandidateTransmitter.h"

std::mutex finalCandidatesMutex;

void moveCandidates(std::vector<std::shared_ptr<Candidate>> &to,
                    std::vector<std::shared_ptr<Candidate>> &from);

void dumpJson(std::string filename, std::vector<std::shared_ptr<Candidate>> candidates) {

  Json::Value root;
  root["final_candidates"] = Json::Value(Json::arrayValue);
  int i = 0;
  for (auto c = candidates.begin(); c != candidates.end(); c++) {
    Json::Value candidate;
    (**c).toJson(candidate);
    root["final_candidates"].insert(i, candidate);
    i++;
  }

  std::ofstream ofs(filename);

  ofs << root;
}

void processCandidatesBreadthFirst(std::vector<std::shared_ptr<Candidate>> candidates,
                                   std::vector<std::shared_ptr<Candidate>> &finalCandidates,
                                   unsigned int depth, CacheLineObservation &o) {
  CandidateGenerator candidateGenerator;
  std::vector<std::shared_ptr<Candidate>> candidatesNextRound;

  do {

    for (auto candidate = candidatesNextRound.begin(); candidate != candidatesNextRound.end();
         candidate++) {
      candidates.push_back(*candidate);
    }

    candidatesNextRound.clear();

    while (candidates.size() > 0) {
      // Reference?
      std::shared_ptr<Candidate> next = candidates.back();
      candidates.pop_back();

      if (next->getDepth() < depth) {
        std::vector<std::shared_ptr<Candidate>> newCandidates =
            candidateGenerator.expandCandidate(*next, o);
        for (std::vector<std::shared_ptr<Candidate>>::iterator c = newCandidates.begin();
             c != newCandidates.end(); c++) {
          if (CandidateChecker::checkCandidate(**c, o)) {
            candidatesNextRound.push_back(*c);
          }
        }
      } else {
        finalCandidates.push_back(next);
      }
    }
  } while (candidatesNextRound.size() > 0);
}

void processCandidatesDepthFirst(std::vector<std::shared_ptr<Candidate>> candidates,
                                 std::vector<std::shared_ptr<Candidate>> &finalCandidates,
                                 CacheLineObservation &o) {

  CandidateGenerator candidateGenerator;
  std::vector<std::shared_ptr<Candidate>> localFinalCandidates;
  unsigned int depth;

  std::unique_lock<std::mutex> lk(finalCandidatesMutex, std::defer_lock);

  try {
    while (candidates.size() > 0) {
      // Reference?
      std::shared_ptr<Candidate> next = candidates.back();
      candidates.pop_back();

      if (Config::finalDepth == -1) {
        depth = next->getTerminationHandler()->getExpandLengthForShortestParam();
      } else {
        depth = Config::finalDepth;
      }

      if (next->getDepth() <= depth) {
        std::vector<std::shared_ptr<Candidate>> newCandidates =
            candidateGenerator.expandCandidate(*next, o);
        for (std::vector<std::shared_ptr<Candidate>>::iterator c = newCandidates.begin();
             c != newCandidates.end(); c++) {
          if (CandidateChecker::checkCandidate(**c, o)) {
            candidates.push_back(*c);
          }
        }
      } else {
        localFinalCandidates.push_back(next);
        if (localFinalCandidates.size() > 1000) {
          lk.lock();
          moveCandidates(finalCandidates, localFinalCandidates);
          lk.unlock();
        }
      }
    }
    lk.lock();
    moveCandidates(finalCandidates, localFinalCandidates);
    lk.unlock();
  } catch (const std::exception &e) {
    lk.unlock();
    std::cerr << "An error occurred during depth first search." << std::endl << e.what()
              << std::endl;
  } catch (...) {
    lk.unlock();
    std::cerr << "An unknown error occurred during depth first search. "
                 "The thread terminated!" << std::endl;
  }
}

void moveCandidates(std::vector<std::shared_ptr<Candidate>> &to,
                    std::vector<std::shared_ptr<Candidate>> &from) {

  to.reserve(to.size() + from.size());
  std::move(std::begin(from), std::end(from),
            std::back_inserter(to));
  from.clear();
}

void scheduleTask(std::vector<std::shared_ptr<Candidate>> &intermediateCandidates,
                  std::future<void> *futures, int i,
                  CacheLineObservation &o,
                  std::vector<std::shared_ptr<Candidate>> &finalCandidates) {

  std::vector<std::shared_ptr<Candidate>> threadCandidates;

  threadCandidates.push_back(intermediateCandidates.back());
  intermediateCandidates.pop_back();

  futures[i] = std::async(std::launch::async,
                          processCandidatesDepthFirst, threadCandidates, std::ref(finalCandidates),
                          std::ref(o));

  std::cout << "Intermediate candidates left: " << intermediateCandidates.size() << std::endl;
}

void printConfig() {
  std::cout << "Config: " << std::endl
            << "\tFinal Depth: " << Config::finalDepth << std::endl
            << "\tIntermediate Depth: " << Config::intermediateDepth << std::endl
            << "\tNumber of Threads: " << Config::numberOfThreads << std::endl
            << "\tObservation File: " << Config::observationFileName << std::endl
            << "\tValidation File: " << Config::validationFileName << std::endl
            << "\tTermination File: " << Config::terminationFileName << std::endl
            << "\tTransmit to chained solver: " << Config::transmitToChainedSolver << std::endl
            << "\tDestination Host: " << Config::destinationHost << std::endl
            << "\tDestination Port: " << Config::destinationPort << std::endl << std::endl;
}

void tryTransmit(bool done,
                 unsigned long long &statusCounter,
                 std::unique_lock<std::mutex> &lk,
                 std::shared_ptr<CandidateTransmitter> transmitter,
                 std::vector<std::shared_ptr<Candidate>> &finalCandidates) {

  std::vector<std::shared_ptr<Candidate>> candidatesToSend;

  lk.lock();
  if (finalCandidates.size() >= 1000 || done) {
    moveCandidates(candidatesToSend, finalCandidates);
  }
  lk.unlock();

  if (candidatesToSend.size() > 0) {
    transmitter->sendCandidates(candidatesToSend);
    if (transmitter->getTransmissionCounter() > (statusCounter + 10000)) {
      statusCounter = transmitter->getTransmissionCounter();
      std::cout << "Transmitted " << statusCounter << " candidates" << std::endl;
    }
  }
}

int main() {

  printConfig();

  bool done = false;
  unsigned long long statusCounter = 0;

  std::unique_lock<std::mutex> lk(finalCandidatesMutex, std::defer_lock);

  CacheLineObservation o;
  CandidateGenerator candidateGenerator;

  std::shared_ptr<CandidateTransmitter> transmitter = nullptr;
  if (Config::transmitToChainedSolver) {
    transmitter = std::make_shared<CandidateTransmitter>(
        Config::destinationHost, Config::destinationPort);
  }

  std::vector<std::shared_ptr<Candidate>> startCandidates =
      candidateGenerator.generateStartCandidates(o);

  for (auto candidate = startCandidates.begin(); candidate != startCandidates.end(); candidate++) {
    std::shared_ptr<ParameterTerminationHandler> parameterTerminationHandler =
        TerminationHandlerFactory::getParameterTerminationHandler(**candidate);
    (**candidate).setTerminationHandler(parameterTerminationHandler);
  }

  std::vector<std::shared_ptr<Candidate>> finalCandidates;
  std::vector<std::shared_ptr<Candidate>> intermediateCandidates;

  processCandidatesBreadthFirst(startCandidates, intermediateCandidates, Config::intermediateDepth,
                                o);

  std::cout << "Number of intermediate Candidates: " << intermediateCandidates.size() << std::endl;

  std::future<void> futures[Config::numberOfThreads];

  for (int i = 0; i < Config::numberOfThreads; i++) {
    scheduleTask(intermediateCandidates, futures, i, o, finalCandidates);
  }

  while (true) {

    for (int i = 0; i < Config::numberOfThreads; i++) {
      if (futures[i].wait_for(std::chrono::milliseconds(100)) == std::future_status::ready
          && intermediateCandidates.size() > 0) {
        scheduleTask(intermediateCandidates, futures, i, o, finalCandidates);
      }
    }

    if (intermediateCandidates.size() == 0) {
      for (int i = 0; i < Config::numberOfThreads; i++) {
        while (futures[i].wait_for(std::chrono::milliseconds(100)) != std::future_status::ready) {
          if (transmitter != nullptr) {
            tryTransmit(done, statusCounter, lk, transmitter, finalCandidates);
          }
        }
      }
      done = true;
    }

    if (transmitter != nullptr) {
      tryTransmit(done, statusCounter, lk, transmitter, finalCandidates);
    }

    if (done) {
      break;
    }
  }

  if (transmitter != nullptr) {
    std::cout << "Transmitted " << transmitter->getTransmissionCounter() << " candidates in total"
              << std::endl;
  }
  lk.lock();
  dumpJson("finalCandidates.json", finalCandidates);
  std::cout << "Number of final Candidates: " << finalCandidates.size() << std::endl;
  std::cout << "Finished!" << std::endl;
  lk.unlock();
}

//
// Created by florian on 29.09.20.
//

#ifndef RECONSTRUCT_CANDIDATETRANSMITTER_H
#define RECONSTRUCT_CANDIDATETRANSMITTER_H


#include <string>
#include <memory>
#include <vector>

#include <sys/socket.h>
#include <netdb.h>

#include "Candidate.h"

class CandidateTransmitter {

public:
  CandidateTransmitter(const std::string &destinationHost, std::string destinationPort);

  virtual ~CandidateTransmitter();

  void sendCandidates(std::vector<std::shared_ptr<Candidate>> &finalCandidates);

  unsigned long long int getTransmissionCounter() const;

private:
  std::string destinationHost;
  std::string destinationPort;

  struct addrinfo hints;
  struct addrinfo *results;

  int socket_fd;
  unsigned long long transmissionCounter;

  void makeConnection();
};


#endif //RECONSTRUCT_CANDIDATETRANSMITTER_H

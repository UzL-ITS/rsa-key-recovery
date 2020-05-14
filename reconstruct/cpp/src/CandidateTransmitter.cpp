//
// Created by florian on 29.09.20.
//

#include "CandidateTransmitter.h"

CandidateTransmitter::CandidateTransmitter(const std::string &destinationHost,
                                           std::string destinationPort)
    : destinationHost(destinationHost), destinationPort(destinationPort),
      results(NULL), socket_fd(-1), transmissionCounter(0) {}

void
CandidateTransmitter::sendCandidates(std::vector<std::shared_ptr<Candidate>> &finalCandidates) {

  if (socket_fd == -1) {
    makeConnection();
  }

  int writeResult = -1;
  int failCounter = 0;
  unsigned int finalCandidateSizeBefore = finalCandidates.size();

  while (finalCandidates.size() > 0) {

    std::shared_ptr<Candidate> candidate = finalCandidates.back();
    finalCandidates.pop_back();

    std::string p = (*candidate).getP() + ";";
    writeResult = write(socket_fd, p.c_str(), p.size());

    if (writeResult == -1 && failCounter < 10) {
      failCounter++;
      close(socket_fd);
      makeConnection();
    }
  }

  transmissionCounter += (finalCandidateSizeBefore - finalCandidates.size());
}

void CandidateTransmitter::makeConnection() {
  memset(&this->hints, 0, sizeof(struct addrinfo));

  this->hints.ai_family = AF_INET;
  this->hints.ai_socktype = SOCK_STREAM;
  this->hints.ai_flags = 0;
  this->hints.ai_protocol = IPPROTO_TCP;

  int addrInfoReturn = getaddrinfo(this->destinationHost.c_str(), this->destinationPort.c_str(),
                                   &this->hints, &this->results);

  if (addrInfoReturn != 0) {
    std::cerr << "Could not get addr info!!!" << std::endl;
    exit(-1);
  }

  struct addrinfo *result;

  for (result = this->results; result != NULL; result = result->ai_next) {
    this->socket_fd = socket(result->ai_family, result->ai_socktype,
                             result->ai_protocol);
    if (this->socket_fd == -1)
      continue;

    if (connect(this->socket_fd, result->ai_addr, result->ai_addrlen) != -1)
      break;

    close(this->socket_fd);
  }

  if (result == NULL) {
    std::cerr << "Could not connect" << std::endl;
    exit(-1);
  }

  freeaddrinfo(this->results);
}

CandidateTransmitter::~CandidateTransmitter() {

  if (socket_fd != -1) {
    close(socket_fd);
  }
}

unsigned long long int CandidateTransmitter::getTransmissionCounter() const {
  return transmissionCounter;
}

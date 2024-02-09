//
// Created by florian on 27.08.20.
//

#include "CandidateGenerator.h"
#include "MathUtils.h"
#include "Constants.h"
#include <algorithm>

CandidateGenerator::CandidateGenerator() {
}

CandidateGenerator::~CandidateGenerator() {

}

std::vector<std::shared_ptr<Candidate>> CandidateGenerator::generateStartCandidates(
    Observation &o) {

  std::vector<std::shared_ptr<Candidate>> candidates;

  std::vector<std::shared_ptr<Observation::KFactors_t>> kFactors;
  o.generateK(kFactors);

  for (std::vector<std::shared_ptr<Observation::KFactors_t>>::iterator iter = kFactors.begin();
       iter != kFactors.end(); iter++) {

    // d:
    //
    // Compute lower bits of d by solving:
    //  (\gamma * e * d) mod 2^(2+\tau(k)) = \gamma mod 2^(2+\tau(k))
    //    Since the modulus is not prime, gamma does not cancel out -> we have no enumerate all
    //    possible d
    //
    //
    // p, q:
    //
    // Compute lower bit of p and q by enumerating all 2^(2*\tau(\gamma)) possibilities.
    // The lowest bit still has to be 1.

    unsigned int mod_2_pow_tau_k_ui, mod_2_pow_tau_gamma_ui, mod_2_pow_1_tau_kp_tau_gamma_ui,
        mod_2_pow_1_tau_kq_tau_gamma_ui, mod_2_pow_1_tau_kq_ui, mod_2_pow_1_tau_kp_ui;

    mpz_t mod_2_pow_2_tau_k, mod_2_pow_2_tau_gamma, d, dp_candidate_lower, dq_candidate_lower, gamma,
        mod_2_pow_1_tau_kp, mod_2_pow_1_tau_kq, mod_2_pow_1_tau_kp_tau_gamma, mod_2_pow_1_tau_kq_tau_gamma;

    mpz_inits(mod_2_pow_2_tau_k, mod_2_pow_2_tau_gamma, mod_2_pow_1_tau_kp, mod_2_pow_1_tau_kq,
              mod_2_pow_1_tau_kp_tau_gamma, mod_2_pow_1_tau_kq_tau_gamma, d, dp_candidate_lower, dq_candidate_lower,
              gamma, NULL);

    unsigned int tau_k = MathUtils::tau((*iter)->k);
    unsigned int tau_kp = MathUtils::tau((*iter)->k_p);
    unsigned int tau_kq = MathUtils::tau((*iter)->k_q);

    mpz_ui_pow_ui(mod_2_pow_2_tau_k, 2, 2 + tau_k);
    mod_2_pow_tau_k_ui = mpz_get_ui(mod_2_pow_2_tau_k);
    mpz_set_ui(gamma, Constants::GAMMA);
    unsigned int tau_gamma = MathUtils::tau(gamma);
    mpz_ui_pow_ui(mod_2_pow_2_tau_gamma, 2, 2 + tau_gamma);
    mod_2_pow_tau_gamma_ui = mpz_get_ui(mod_2_pow_2_tau_gamma);

    mpz_ui_pow_ui(mod_2_pow_1_tau_kp, 2, 1 + tau_kp);
    mod_2_pow_1_tau_kp_ui = mpz_get_ui(mod_2_pow_1_tau_kp);
    MathUtils::modularInverse(dp_candidate_lower, o.e, mod_2_pow_1_tau_kp);

    mpz_ui_pow_ui(mod_2_pow_1_tau_kq, 2, 1 + tau_kq);
    mod_2_pow_1_tau_kq_ui = mpz_get_ui(mod_2_pow_1_tau_kq);
    MathUtils::modularInverse(dq_candidate_lower, o.e, mod_2_pow_1_tau_kq);

    mpz_ui_pow_ui(mod_2_pow_1_tau_kp_tau_gamma, 2, 1 + tau_kp + tau_gamma);
    mod_2_pow_1_tau_kp_tau_gamma_ui = mpz_get_ui(mod_2_pow_1_tau_kp_tau_gamma);
    mpz_ui_pow_ui(mod_2_pow_1_tau_kq_tau_gamma, 2, 1 + tau_kq + tau_gamma);
    mod_2_pow_1_tau_kq_tau_gamma_ui = mpz_get_ui(mod_2_pow_1_tau_kq_tau_gamma);

    for (unsigned int d_candidate = 0; d_candidate < mod_2_pow_tau_k_ui; d_candidate++) {
      mpz_t left, right;
      mpz_inits(left, right, NULL);
      mpz_set_ui(d, d_candidate);

      mpz_mul_ui(left, o.e, Constants::GAMMA);
      mpz_mul(left, left, d);
      mpz_mod(left, left, mod_2_pow_2_tau_k);

      mpz_mod(right, gamma, mod_2_pow_2_tau_k);

      if (0 == mpz_cmp(right, left)) {

        for (unsigned int p_candidate = 1; p_candidate < mod_2_pow_tau_gamma_ui; p_candidate++) {
          for (unsigned int q_candidate = 1; q_candidate < mod_2_pow_tau_gamma_ui; q_candidate++) {
            for (unsigned int dp_candidate_upper = 0;
                 dp_candidate_upper < std::max<unsigned>(1, mod_2_pow_1_tau_kp_tau_gamma_ui - mod_2_pow_1_tau_kp_ui); dp_candidate_upper++) {
              for (unsigned int dq_candidate_upper = 0;
                   dq_candidate_upper <
                       std::max<unsigned>(1, mod_2_pow_1_tau_kq_tau_gamma_ui - mod_2_pow_1_tau_kq_ui); dq_candidate_upper++) {

                mpz_t q, p, dp, dq, intermediate;
                mpz_inits(p, q, dp, dq, intermediate, NULL);
                mpz_set_ui(q, q_candidate);
                mpz_set_ui(p, p_candidate);

                mpz_mul_ui(intermediate, mod_2_pow_1_tau_kp, dp_candidate_upper);
                mpz_add(dp, dp_candidate_lower, intermediate);

                mpz_mul_ui(intermediate, mod_2_pow_1_tau_kq, dq_candidate_upper);
                mpz_add(dq, dq_candidate_lower, intermediate);

                auto candidate = std::make_shared<Candidate>(p, q, d, dp, dq,
                                                             (*iter)->k, (*iter)->k_p, (*iter)->k_q,
                                                             tau_k, tau_kp, tau_kq, 1, tau_gamma, nullptr);

                candidates.push_back(candidate);

                mpz_clears(p, q, dp, dq, intermediate, NULL);
              }
            }
          }
        }
      }

      mpz_clears(left, right, NULL);
    }

    mpz_clears(mod_2_pow_2_tau_k, mod_2_pow_2_tau_gamma, d, dp_candidate_lower, dq_candidate_lower, gamma,
               mod_2_pow_1_tau_kp, mod_2_pow_1_tau_kq,
               mod_2_pow_1_tau_kp_tau_gamma, mod_2_pow_1_tau_kq_tau_gamma, NULL);
  }

  return candidates;
}

std::vector<std::shared_ptr<Candidate>> CandidateGenerator::expandCandidate(
    Candidate &c, Observation &o) {
  std::vector<std::shared_ptr<Candidate>> newCandidates;

  for (uint8_t p = 0; p < 2; p++) {
    for (uint8_t q = 0; q < 2; q++) {
      for (uint8_t d = 0; d < 2; d++) {
        for (uint8_t dp = 0; dp < 2; dp++) {
          for (uint8_t dq = 0; dq < 2; dq++) {

            if (c.checkCandidateExtension(p, q, d, dp, dq, o)) {
              std::shared_ptr<Candidate> candidate = c.extend(p, q, d, dp, dq);
              newCandidates.push_back(candidate);
            }
          }
        }
      }
    }
  }

  return newCandidates;
}



#include <TLorentzVector.h>
#include <vector>

#ifndef HAL_PARTICLE
#define HAL_PARTICLE

namespace HAL {

class Particle {
public:
  Int_t PDG_ID;
  TLorentzVector *P;
  std::vector<Int_t> motherIndexList;
  std::vector<Particle> motherList;
  std::vector<Int_t> daughterIndexList;
  std::vector<Particle> daughterList;
};

}

#endif
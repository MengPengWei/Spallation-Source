#ifndef FUSIONNEUTRONGUN_HH
#define FUSIONNEUTRONGUN_HH

#include "G4ParticleGun.hh"
#include "G4Event.hh"

class FusionNeutronGun
{
public:
    explicit FusionNeutronGun(G4ParticleGun* particleGun);
    ~FusionNeutronGun() = default;

    void Configure();
    void GeneratePrimaries(G4Event* event);

private:
    G4ParticleGun* fParticleGun;
};

#endif // FUSIONNEUTRONGUN_HH

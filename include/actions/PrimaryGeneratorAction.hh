#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4Types.hh"

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
    PrimaryGeneratorAction();
    ~PrimaryGeneratorAction() override;

    void GeneratePrimaries(G4Event*) override;

private:
    G4ParticleGun* fParticleGun;

    /// 均匀圆形束流半径（mm）。
    /// > 0 时在 GeneratePrimaries 中对 (x,y) 做均匀圆面采样；
    /// = 0 时退化为点源（默认行为）。
    G4double fBeamRadius;
};

#endif
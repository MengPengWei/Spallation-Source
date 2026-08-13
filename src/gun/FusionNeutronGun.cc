#include "FusionNeutronGun.hh"

#include "FusionConstants.hh"

#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4ios.hh"

#include <cmath>

FusionNeutronGun::FusionNeutronGun(G4ParticleGun* particleGun)
    : fParticleGun(particleGun)
{
}

void FusionNeutronGun::Configure()
{
    if (!fParticleGun) return;

    auto* neutron = G4ParticleTable::GetParticleTable()->FindParticle("neutron");
    fParticleGun->SetParticleDefinition(neutron);
    fParticleGun->SetParticleEnergy(C_FUSION_USE_DT ? C_FUSION_DT_Energy : C_FUSION_DD_Energy);

    G4cout << "\n[FusionNeutronGun] Fusion source configured\n"
           << "  Reaction: " << (C_FUSION_USE_DT ? "DT" : "DD") << "\n"
           << "  Neutron energy: "
           << (C_FUSION_USE_DT ? C_FUSION_DT_Energy : C_FUSION_DD_Energy) / MeV
           << " MeV\n" << G4endl;
}

void FusionNeutronGun::GeneratePrimaries(G4Event* event)
{
    if (!fParticleGun) return;

    const G4double u = G4UniformRand();
    const G4double v = G4UniformRand();
    const G4double w = G4UniformRand();

    const G4double r = C_FUSION_SourceR * std::cbrt(u);
    const G4double costh = 2.0 * v - 1.0;
    const G4double sinth = std::sqrt(1.0 - costh * costh);
    const G4double phi = CLHEP::twopi * w;

    const G4double x = C_FUSION_Center.x() + r * sinth * std::cos(phi);
    const G4double y = C_FUSION_Center.y() + r * sinth * std::sin(phi);
    const G4double z = C_FUSION_Center.z() + r * costh;

    const G4double udir = G4UniformRand();
    const G4double vdir = G4UniformRand();
    const G4double costhDir = 2.0 * udir - 1.0;
    const G4double sinthDir = std::sqrt(1.0 - costhDir * costhDir);
    const G4double phiDir = CLHEP::twopi * vdir;

    fParticleGun->SetParticlePosition(G4ThreeVector(x, y, z));
    fParticleGun->SetParticleMomentumDirection(
        G4ThreeVector(sinthDir * std::cos(phiDir),
                      sinthDir * std::sin(phiDir),
                      costhDir));

    fParticleGun->GeneratePrimaryVertex(event);
}

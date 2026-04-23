// src/gun/ProtonGun.cc
#include "gun/ProtonGun.hh"

#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4ios.hh"

#include <cmath>
#include <algorithm>

ProtonGun::ProtonGun(G4ParticleGun* particleGun)
    : fParticleGun(particleGun)
{
}

void ProtonGun::Configure()
{
    if (!fParticleGun) return;

    auto* proton =
        G4ParticleTable::GetParticleTable()->FindParticle("proton");

    fParticleGun->SetParticleDefinition(proton);
    fParticleGun->SetParticleEnergy(C_PG_Energy);
    fParticleGun->SetParticleMomentumDirection(Direction());
    fParticleGun->SetParticlePosition(
        G4ThreeVector(C_PG_Center_X, C_PG_Center_Y, C_PG_Position_Z));

    G4cout << "\n[ProtonGun] Proton beam configured\n"
           << "  Energy: " << C_PG_Energy / MeV << " MeV\n"
           << "  Center: (" << C_PG_Center_X / mm << ", "
                          << C_PG_Center_Y / mm << ") mm\n"
           << "  Z0:     " << C_PG_Position_Z / mm << " mm\n"
           << "  Dist:   2D Gaussian (FWHM=" << C_PG_FWHM / mm
           << " mm), truncated square halfSide=" << C_PG_HalfSide / mm
           << " mm\n" << G4endl;
}

void ProtonGun::SampleTransverseOffset(G4double& x_rel, G4double& y_rel) const
{
    const G4double halfSide = C_PG_HalfSide;
    const G4double sigma    = C_PG_FWHM / 2.35482; // FWHM->sigma

    while (true)
    {
        // 2个均匀数生成一对独立高斯 (Box-Muller)
        G4double u1 = G4UniformRand();
        G4double u2 = G4UniformRand();

        // 防止 log(0)
        u1 = std::max(u1, 1e-12);

        const G4double R  = sigma * std::sqrt(-2.0 * std::log(u1));//R是高斯分布的半径，通过后续的if进行截断
        const G4double th = CLHEP::twopi * u2;

        x_rel = R * std::cos(th);
        y_rel = R * std::sin(th);

        if (std::fabs(x_rel) <= halfSide && std::fabs(y_rel) <= halfSide)
            return;
    }
}

void ProtonGun::GeneratePrimaries(G4Event* event)
{
    if (!fParticleGun) return;

    G4double x_rel = 0.0;
    G4double y_rel = 0.0;
    SampleTransverseOffset(x_rel, y_rel);

    const G4double x0 = C_PG_Center_X + x_rel;
    const G4double y0 = C_PG_Center_Y + y_rel;
    const G4double z0 = C_PG_Position_Z;

    fParticleGun->SetParticlePosition(G4ThreeVector(x0, y0, z0));
    fParticleGun->GeneratePrimaryVertex(event);
}
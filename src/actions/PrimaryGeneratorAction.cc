#include "PrimaryGeneratorAction.hh"
#include "Run.hh"
#include "Constants.hh"
#include "TtConstants.hh"
#include "PgConstants.hh"

#include "G4ParticleTable.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4ios.hh"

#include <cmath>

PrimaryGeneratorAction::PrimaryGeneratorAction()
    : fBeamRadius(0.0)
{
    fParticleGun = new G4ParticleGun(1);

    if (is_TT)
    {
        G4ParticleDefinition* proton =
            G4ParticleTable::GetParticleTable()->FindParticle("proton");
        fParticleGun->SetParticleDefinition(proton);

        // 改用 PgConstants.hh
        fParticleGun->SetParticleEnergy(C_PG_Energy);
        fParticleGun->SetParticleMomentumDirection(C_PG_Direction);
        fParticleGun->SetParticlePosition(
            G4ThreeVector(C_PG_Center_X, C_PG_Center_Y, C_PG_Position_Z));

        // TT 模式下不再用均匀圆束
        fBeamRadius = 0.0;

        G4cout << "\n[PrimaryGeneratorAction] TT(PG) 模式已激活" << G4endl;
        G4cout << "  粒子:   proton" << G4endl;
        G4cout << "  动能:   " << C_PG_Energy/MeV << " MeV" << G4endl;
        G4cout << "  束斑中心: (" << C_PG_Center_X/mm << ", "
                                << C_PG_Center_Y/mm << ") mm" << G4endl;
        G4cout << "  起始Z:  " << C_PG_Position_Z/mm << " mm" << G4endl;
        G4cout << "  分布:   2D Gaussian (FWHM=" << C_PG_FWHM/mm
               << " mm) truncated in square halfSide=" << C_PG_HalfSide/mm
               << " mm\n" << G4endl;
    }
    else
    {
        // 默认中子束流（保留你原来的）
        G4ParticleDefinition* neutron =
            G4ParticleTable::GetParticleTable()->FindParticle("neutron");
        fParticleGun->SetParticleDefinition(neutron);
        fParticleGun->SetParticleEnergy(1.0 * MeV);
        fParticleGun->SetParticlePosition(G4ThreeVector(0., 0., -40.*mm));
        fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));
        fBeamRadius = 0.0;
    }
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fParticleGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
    if (is_TT)
    {
        const G4double halfSide = C_PG_HalfSide;
        const G4double sigma    = C_PG_FWHM / 2.35482;

        G4double x_rel = 0.0;
        G4double y_rel = 0.0;

        // 拒绝采样，确保落入方形范围
        while (true)
        {
            const G4double u1 = G4UniformRand();
            const G4double u2 = G4UniformRand();
            const G4double u3 = G4UniformRand();
            const G4double u4 = G4UniformRand();

            x_rel = sigma * std::sqrt(-2.0 * std::log(u1)) * std::cos(CLHEP::twopi * u2);
            y_rel = sigma * std::sqrt(-2.0 * std::log(u3)) * std::cos(CLHEP::twopi * u4);

            if (std::fabs(x_rel) <= halfSide && std::fabs(y_rel) <= halfSide)
                break;
        }

        const G4double x0 = C_PG_Center_X + x_rel;
        const G4double y0 = C_PG_Center_Y + y_rel;
        const G4double z0 = C_PG_Position_Z;

        fParticleGun->SetParticlePosition(G4ThreeVector(x0, y0, z0));
    }
    else
    {
        // 非TT：如果你还想保留均匀圆束，则在这里写；否则不需要改
        // （你原来的 fBeamRadius 圆束采样可以留着）
        if (fBeamRadius > 0.)
        {
            G4double r   = fBeamRadius * std::sqrt(G4UniformRand());
            G4double phi = CLHEP::twopi * G4UniformRand();
            G4ThreeVector center = fParticleGun->GetParticlePosition();
            fParticleGun->SetParticlePosition(
                G4ThreeVector(r * std::cos(phi), r * std::sin(phi), center.z()));
        }
    }

    fParticleGun->GeneratePrimaryVertex(event);

    Run* run = static_cast<Run*>(
        G4RunManager::GetRunManager()->GetNonConstCurrentRun());
    if (run)
        run->SetPrimary(fParticleGun->GetParticleDefinition(),
                        fParticleGun->GetParticleEnergy());
}
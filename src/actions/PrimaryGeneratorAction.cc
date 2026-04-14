#include "PrimaryGeneratorAction.hh"
#include "Run.hh"
#include "Constants.hh"
#include "TtConstants.hh"

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
        // ----------------------------------------------------------------
        // 钽靶模式：70 MeV 均匀质子束，沿 +Z 方向入射
        // 束流圆心位于靶面入口前方 1 mm 处（z = C_TT_Pos.z - C_TT_Water_HalfL - 1 mm）
        // 均匀分布半径：C_TT_Beam_Radius（见 TtConstants.hh，单位 mm）
        // 束流强度：由宏文件中的 /run/beamOn 控制事件数；
        //   实际物理强度请在宏/分析中结合粒子权重（particles/s）换算，
        //   典型参考值：1e13 p/s
        // ----------------------------------------------------------------
        G4ParticleDefinition* proton =
            G4ParticleTable::GetParticleTable()->FindParticle("proton");
        fParticleGun->SetParticleDefinition(proton);
        fParticleGun->SetParticleEnergy(C_TT_Beam_Energy);  // 70 MeV

        G4double beamZ = C_TT_Pos.z() - C_TT_Water_HalfL - 1.0*mm;
        fParticleGun->SetParticlePosition(G4ThreeVector(0., 0., beamZ));
        fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));

        fBeamRadius = C_TT_Beam_Radius;  // 均匀圆面分布半径

        G4cout << "\n[PrimaryGeneratorAction] 钽靶模式已激活" << G4endl;
        G4cout << "  粒子:   proton" << G4endl;
        G4cout << "  动能:   " << C_TT_Beam_Energy/MeV << " MeV" << G4endl;
        G4cout << "  起始Z:  " << beamZ/mm << " mm" << G4endl;
        G4cout << "  束流半径(均匀圆面): " << fBeamRadius/mm << " mm\n" << G4endl;
    }
    else
    {
        // ----------------------------------------------------------------
        // 默认中子束流（活化片照射）
        // ----------------------------------------------------------------
        G4ParticleDefinition* neutron =
            G4ParticleTable::GetParticleTable()->FindParticle("neutron");
        fParticleGun->SetParticleDefinition(neutron);
        fParticleGun->SetParticleEnergy(1.0 * MeV);
        fParticleGun->SetParticlePosition(G4ThreeVector(0., 0., -40.*mm));
        fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));

        fBeamRadius = 0.0;  // 点源
    }
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fParticleGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
    // 均匀圆形横向分布：在半径 fBeamRadius 内对 (x, y) 做均匀采样
    // 采用极坐标拒绝采样（等效：r = R*sqrt(U) 可保证面积均匀分布）
    if (fBeamRadius > 0.)
    {
        G4double r   = fBeamRadius * std::sqrt(G4UniformRand());
        G4double phi = CLHEP::twopi * G4UniformRand();
        G4ThreeVector center = fParticleGun->GetParticlePosition();
        fParticleGun->SetParticlePosition(
            G4ThreeVector(r * std::cos(phi), r * std::sin(phi), center.z()));
    }

    fParticleGun->GeneratePrimaryVertex(event);

    // 通知 Run 记录初级粒子信息（供 EndOfRun 打印）
    Run* run = static_cast<Run*>(
        G4RunManager::GetRunManager()->GetNonConstCurrentRun());
    if (run)
        run->SetPrimary(fParticleGun->GetParticleDefinition(),
                        fParticleGun->GetParticleEnergy());
}


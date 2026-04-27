#include "SteppingAction.hh"
#include "EventAction.hh"
#include "DetectorConstruction.hh"
#include "Run.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4StepPoint.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4VProcess.hh"
#include "G4SystemOfUnits.hh"
#include "G4RootAnalysisManager.hh"
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "Constants.hh"

SteppingAction::SteppingAction(DetectorConstruction* detector, EventAction* event)
    : G4UserSteppingAction(),
      fDetector(detector),
      fEvent(event)
{}

SteppingAction::~SteppingAction() = default;

void SteppingAction::UserSteppingAction(const G4Step* step)
{
    if (!step) return;
    auto track = step->GetTrack();
    if (!track) return;

    const G4StepPoint* pre  = step->GetPreStepPoint();
    const G4StepPoint* post = step->GetPostStepPoint();

    // Guard against missing volumes (e.g. world boundary exit)
    auto preVol = pre->GetTouchableHandle()->GetVolume();
    if (!preVol) return;
    G4LogicalVolume* preLV = preVol->GetLogicalVolume();
    const G4String&  lvName = preLV->GetName();

    // 快速排除：只关注两个 source 探测器逻辑体
    const bool inProtonDet  = (lvName == "SD_LV_Proton");
    const bool inNeutronDet = (lvName == "SD_LV_Neutron");
    if (!inProtonDet && !inNeutronDet) return;

    const G4String& partName =
        track->GetParticleDefinition()->GetParticleName();
    auto man = G4RootAnalysisManager::Instance();
    const G4int evtId =
        G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();

    // 步中点位置 (全局坐标, mm)
    const G4ThreeVector midPos = 0.5 * (pre->GetPosition() + post->GetPosition());
    const G4double x_mm = midPos.x() / mm;
    const G4double y_mm = midPos.y() / mm;
    const G4double z_mm = midPos.z() / mm;

    // 步前动能及能量沉积
    const G4double Ekin_MeV = pre->GetKineticEnergy() / MeV;
    const G4double edep_MeV = step->GetTotalEnergyDeposit() / MeV;

    // 步前动量方向 (MeV/c)
    const G4ThreeVector mom = pre->GetMomentum();
    const G4double px_MeV = mom.x() / MeV;
    const G4double py_MeV = mom.y() / MeV;
    const G4double pz_MeV = mom.z() / MeV;

    // ==============================================================
    // 质子探测器：记录质子步信息（用于3D热力分布）
    // ==============================================================
    if (inProtonDet && partName == "proton")
    {
        // --- Ntuple 4 "ProtonDetStep": 逐步信息 ---
        man->FillNtupleIColumn(4, 0, evtId);
        man->FillNtupleDColumn(4, 1, x_mm);
        man->FillNtupleDColumn(4, 2, y_mm);
        man->FillNtupleDColumn(4, 3, z_mm);
        man->FillNtupleDColumn(4, 4, Ekin_MeV);
        man->FillNtupleDColumn(4, 5, edep_MeV);
        man->FillNtupleDColumn(4, 6, px_MeV);
        man->FillNtupleDColumn(4, 7, py_MeV);
        man->FillNtupleDColumn(4, 8, pz_MeV);
        man->AddNtupleRow(4);

        // --- H3 "hProton3D": 质子位置 3D 直方图 ---
        man->FillH3(0, x_mm, y_mm, z_mm);

        // --- 累加到事件级统计 ---
        fEvent->AddProtonStep(edep_MeV);
    }

    // ==============================================================
    // 中子探测器：记录中子步信息（用于产额热力图与能谱）
    // ==============================================================
    if (inNeutronDet && partName == "neutron")
    {
        // --- Ntuple 5 "NeutronDetStep": 逐步信息 ---
        man->FillNtupleIColumn(5, 0, evtId);
        man->FillNtupleDColumn(5, 1, x_mm);
        man->FillNtupleDColumn(5, 2, y_mm);
        man->FillNtupleDColumn(5, 3, z_mm);
        man->FillNtupleDColumn(5, 4, Ekin_MeV);
        man->FillNtupleDColumn(5, 5, px_MeV);
        man->FillNtupleDColumn(5, 6, py_MeV);
        man->FillNtupleDColumn(5, 7, pz_MeV);
        man->AddNtupleRow(5);

        // 仅在中子首次进入探测器时（边界步）填充运行时直方图
        if (pre->GetStepStatus() == fGeomBoundary)
        {
            // --- H1 "hNeutronSpectrum": 中子动能谱 ---
            man->FillH1(0, Ekin_MeV);

            // --- H2 "hNeutronYield2D": 中子产额 x-y 热力图 ---
            // 使用步前位置（边界点）更准确地反映入射位置
            const G4double xEntry = pre->GetPosition().x() / mm;
            const G4double yEntry = pre->GetPosition().y() / mm;
            man->FillH2(0, xEntry, yEntry);

            // --- 累加到事件级统计 ---
            fEvent->AddNeutronEntry();
        }
    }
}


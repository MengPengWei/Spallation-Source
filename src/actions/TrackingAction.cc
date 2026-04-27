#include "TrackingAction.hh"
#include "Run.hh"

#include "G4RunManager.hh"
#include "G4Track.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VProcess.hh"

TrackingAction::TrackingAction(DetectorConstruction* detector)
    : G4UserTrackingAction(),
      fDetector(detector)
{}

TrackingAction::~TrackingAction()
{}

void TrackingAction::PreUserTrackingAction(const G4Track* track)
{
    // 跳过初级粒子（质子），仅统计次级粒子
    if (track->GetTrackID() == 1) return;

    // 获取粒子产生时所在的逻辑体
    const G4VPhysicalVolume* pvol = track->GetVolume();
    if (!pvol) return;
    const G4String& lvName = pvol->GetLogicalVolume()->GetName();

    // 统计钽靶区域（TT_*）内产生的次级粒子信息
    // 这些数据会在 Run::Merge 中汇总到主线程并在 EndOfRun 打印
    if (lvName.find("TT_") == 0)
    {
        Run* run = static_cast<Run*>(
            G4RunManager::GetRunManager()->GetNonConstCurrentRun());
        if (run)
        {
            run->ParticleCount(
                track->GetParticleDefinition()->GetParticleName(),
                track->GetKineticEnergy());
            const G4VProcess* proc = track->GetCreatorProcess();
            if (proc) run->CountProcesses(proc);
        }
    }
}

void TrackingAction::PostUserTrackingAction(const G4Track*)
{}


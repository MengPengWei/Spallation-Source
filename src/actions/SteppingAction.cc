#include "SteppingAction.hh"
#include "EventAction.hh"
#include "DetectorConstruction.hh"
#include "RunAction.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4StepPoint.hh"
#include "G4VPhysicalVolume.hh"
#include "G4SystemOfUnits.hh"
#include "G4VProcess.hh"

SteppingAction::SteppingAction(DetectorConstruction* detector, EventAction* event)
    : G4UserSteppingAction(),
      fDetector(detector),
      fEvent(event)
{
}

SteppingAction::~SteppingAction() = default;

void SteppingAction::UserSteppingAction(const G4Step* step)
{

    if (!step) return;
    auto track = step->GetTrack();
    if (!track) return;

    //记录各process及其发生次数
    const G4StepPoint* PSP = step->GetPostStepPoint();
    const G4VProcess* process = PSP->GetProcessDefinedStep();
    const G4String& particleName = track->GetParticleDefinition()->GetParticleName();









    //填充各物理量
    auto man = G4RootAnalysisManager::Instance();
    //填充衰变链
    man->FillNtupleSColumn(0,0, track->GetParticleDefinition()->GetParticleName());//将当前步骤中跟踪的粒子的名称填充到nTuple的第0列，
    man->FillNtupleSColumn(0,1, process ? process->GetProcessName() : "None");//将当前步骤中涉及的物理过程的名称填充到nTuple的第1列，如果没有涉及任何物理过程，则填充"None"。


}

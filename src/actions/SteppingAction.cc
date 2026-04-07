#include "SteppingAction.hh"
#include "EventAction.hh"
#include "DetectorConstruction.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4StepPoint.hh"
#include "G4VPhysicalVolume.hh"
#include "g4root.hh"
#include "G4SystemOfUnits.hh"

SteppingAction::SteppingAction(DetectorConstruction* detector, EventAction* event)
    : G4UserSteppingAction(),
      fDetector(detector),
      fEvent(event)
{
}

SteppingAction::~SteppingAction() = default;

void SteppingAction::UserSteppingAction(const G4Step* step)
{
    // Only record steps that cross a geometry boundary
    if (step->GetPostStepPoint()->GetStepStatus() != fGeomBoundary) return;

    // Only neutrons
    auto track = step->GetTrack();
    if (track->GetParticleDefinition()->GetParticleName() != "neutron") return;

    // Check the post-step volume is a sample cylinder
    auto postVol = step->GetPostStepPoint()->GetTouchableHandle()->GetVolume();
    if (!postVol) return;
    if (postVol->GetLogicalVolume() != fDetector->GetSampleLV()) return;

    // Fill ntuple row
    auto man = G4AnalysisManager::Instance();
    const auto& pos = step->GetPostStepPoint()->GetPosition();

    man->FillNtupleDColumn(0, track->GetKineticEnergy() / MeV);
    man->FillNtupleDColumn(1, step->GetPostStepPoint()->GetGlobalTime() / ns);
    man->FillNtupleDColumn(2, pos.x() / mm);
    man->FillNtupleDColumn(3, pos.y() / mm);
    man->FillNtupleDColumn(4, pos.z() / mm);
    man->FillNtupleIColumn(5, postVol->GetCopyNo());
    man->AddNtupleRow();
}

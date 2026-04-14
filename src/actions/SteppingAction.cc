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

}

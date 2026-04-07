#include "SteppingAction.hh"
#include "EventAction.hh"
#include "G4Step.hh"

SteppingAction::SteppingAction(DetectorConstruction* detector, EventAction* event)
    : G4UserSteppingAction(),
        fDetector(detector),
        fEvent(event)
{
}

SteppingAction::~SteppingAction() = default;
void SteppingAction::UserSteppingAction(const G4Step* aStep) {}
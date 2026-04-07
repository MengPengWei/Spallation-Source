#ifndef SteppingAction_h
#define SteppingAction_h

#include "G4UserSteppingAction.hh"
#include "G4Step.hh"
#include "DetectorConstruction.hh"

class EventAction;

class SteppingAction : public G4UserSteppingAction
{
public:
    SteppingAction(DetectorConstruction*, EventAction*);
    ~SteppingAction() override;

    void UserSteppingAction(const G4Step*) override;

private:
    EventAction* fEvent = nullptr;
    DetectorConstruction* fDetector = nullptr;
};

#endif
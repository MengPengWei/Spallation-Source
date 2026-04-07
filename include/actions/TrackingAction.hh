#ifndef TrackingAction_h
#define TrackingAction_h

#include "G4UserTrackingAction.hh"
#include "G4Track.hh"
#include "DetectorConstruction.hh"

class TrackingAction : public G4UserTrackingAction
{
public:
    TrackingAction(DetectorConstruction* detector);
    ~TrackingAction() override;

    void PreUserTrackingAction(const G4Track*) override;
    void PostUserTrackingAction(const G4Track*) override;

private:
    DetectorConstruction* fDetector=nullptr;
};

#endif
#include "TrackingAction.hh"

TrackingAction::TrackingAction(DetectorConstruction* detector)
    : G4UserTrackingAction(),
        fDetector(detector)
{}

TrackingAction::~TrackingAction()
{}

void TrackingAction::PreUserTrackingAction(const G4Track*track)
{}

void TrackingAction::PostUserTrackingAction(const G4Track*track)
{}
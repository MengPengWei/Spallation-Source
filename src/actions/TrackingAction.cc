#include "TrackingAction.hh"
#include "Run.hh"

#include "G4RunManager.hh"
#include "G4Track.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"

TrackingAction::TrackingAction(DetectorConstruction* detector)
    : G4UserTrackingAction(),
      fDetector(detector)
{}

TrackingAction::~TrackingAction()
{}

void TrackingAction::PreUserTrackingAction(const G4Track* track)
{
    // Skip the primary particle; we only count secondaries.
    if (track->GetTrackID() == 1) return;

    
}

void TrackingAction::PostUserTrackingAction(const G4Track*)
{}

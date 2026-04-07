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

    // Determine which logical volume this track was created in.
    G4VPhysicalVolume* pvol = track->GetVolume();
    if (!pvol) return;
    G4LogicalVolume* birthLV = pvol->GetLogicalVolume();

    // Only record particles born inside a sample volume.
    G4LogicalVolume* sampleLV = fDetector->GetSampleLV();
    if (!sampleLV || birthLV != sampleLV) return;

    // Accumulate in the per-run statistics.
    Run* run = static_cast<Run*>(
        G4RunManager::GetRunManager()->GetNonConstCurrentRun());
    if (!run) return;

    const G4String& name = track->GetParticleDefinition()->GetParticleName();
    G4double ekin = track->GetKineticEnergy();
    run->ParticleCount(name, ekin);
}

void TrackingAction::PostUserTrackingAction(const G4Track*)
{}

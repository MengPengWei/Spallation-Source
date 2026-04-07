#include "SteppingAction.hh"
#include "EventAction.hh"
#include "DetectorConstruction.hh"

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

    // Only track neutrons
    auto track = step->GetTrack();
    if (!track) return;
    if (track->GetParticleDefinition()->GetParticleName() != "neutron") return;

    // Retrieve sample logical volume (set after Construct())
    G4LogicalVolume* sampleLV = fDetector->GetSampleLV();
    if (!sampleLV) return;

    const G4StepPoint* pre  = step->GetPreStepPoint();
    const G4StepPoint* post = step->GetPostStepPoint();

    // Guard against missing volumes (e.g., world boundary)
    auto preVol  = pre->GetTouchableHandle()->GetVolume();
    if (!preVol) return;
    G4LogicalVolume* preLV = preVol->GetLogicalVolume();

    // Current step is inside the sample if pre-point LV == sampleLV
    bool inSample = (preLV == sampleLV);

    // ------------------------------------------------------------------
    // Case A: neutron just entered the sample (boundary crossing)
    // ------------------------------------------------------------------
    if (pre->GetStepStatus() == fGeomBoundary && inSample) {
        G4int copyNo = pre->GetTouchableHandle()->GetCopyNumber();
        G4int eventId = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();

        G4ThreeVector pos = pre->GetPosition();
        G4ThreeVector mom = pre->GetMomentum();   // MeV/c

        auto man = G4RootAnalysisManager::Instance();
        man->FillNtupleIColumn(0, 0, eventId);
        man->FillNtupleIColumn(0, 1, copyNo);
        man->FillNtupleDColumn(0, 2, pre->GetKineticEnergy() / MeV);
        man->FillNtupleDColumn(0, 3, pre->GetGlobalTime()   / ns);
        man->FillNtupleDColumn(0, 4, pos.x() / mm);
        man->FillNtupleDColumn(0, 5, pos.y() / mm);
        man->FillNtupleDColumn(0, 6, pos.z() / mm);
        man->FillNtupleDColumn(0, 7, mom.x() / MeV);
        man->FillNtupleDColumn(0, 8, mom.y() / MeV);
        man->FillNtupleDColumn(0, 9, mom.z() / MeV);
        man->AddNtupleRow(0);

        fEvent->AddEntry(copyNo);
    }

    // ------------------------------------------------------------------
    // Case B: neutron is in the sample and undergoes a physics process
    //         (anything other than Transportation)
    // ------------------------------------------------------------------
    if (inSample) {
        const G4VProcess* process = post->GetProcessDefinedStep();
        if (!process) return;
        const G4String& procName = process->GetProcessName();
        if (procName == "Transportation" || procName == "CoupledTransportation")
            return;

        G4int copyNo  = pre->GetTouchableHandle()->GetCopyNumber();
        G4int eventId = G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();

        G4double edep    = step->GetTotalEnergyDeposit();
        G4double stepLen = step->GetStepLength();
        G4double preKE   = pre->GetKineticEnergy();
        G4double postKE  = post->GetKineticEnergy();
        G4ThreeVector pos = pre->GetPosition();

        auto man = G4RootAnalysisManager::Instance();

        // --- Ntuple 1: NeutronInteraction ---
        man->FillNtupleIColumn(1, 0, eventId);
        man->FillNtupleIColumn(1, 1, copyNo);
        man->FillNtupleSColumn(1, 2, procName);
        man->FillNtupleDColumn(1, 3, edep    / MeV);
        man->FillNtupleDColumn(1, 4, stepLen / mm);
        man->FillNtupleDColumn(1, 5, preKE   / MeV);
        man->FillNtupleDColumn(1, 6, postKE  / MeV);
        man->FillNtupleDColumn(1, 7, pos.x() / mm);
        man->FillNtupleDColumn(1, 8, pos.y() / mm);
        man->FillNtupleDColumn(1, 9, pos.z() / mm);
        man->AddNtupleRow(1);

        fEvent->AddInteraction(copyNo);
        fEvent->AddEdep(copyNo, edep);
        // Geant4 neutron-capture process name is "nCapture"
        if (procName.find("Capture") != std::string::npos)
            fEvent->AddCapture(copyNo);

        // --- Ntuple 2: secondaries produced in this step ---
        const std::vector<const G4Track*>* secondaries =
            step->GetSecondaryInCurrentStep();
        if (secondaries) {
            for (const G4Track* sec : *secondaries) {
                if (!sec) continue;
                G4ThreeVector secPos = sec->GetPosition();
                man->FillNtupleIColumn(2, 0, eventId);
                man->FillNtupleIColumn(2, 1, copyNo);
                man->FillNtupleSColumn(2, 2,
                    sec->GetParticleDefinition()->GetParticleName());
                man->FillNtupleDColumn(2, 3, sec->GetKineticEnergy() / MeV);
                man->FillNtupleDColumn(2, 4, secPos.x() / mm);
                man->FillNtupleDColumn(2, 5, secPos.y() / mm);
                man->FillNtupleDColumn(2, 6, secPos.z() / mm);
                man->AddNtupleRow(2);
            }
        }
    }
}

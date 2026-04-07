#include "RunAction.hh"
#include "G4Run.hh"

RunAction::RunAction() = default;
RunAction::~RunAction() = default;

void RunAction::BeginOfRunAction(const G4Run*)
{
    auto man = G4AnalysisManager::Instance();
    man->SetFileName("neutrons_output");
    man->OpenFile();

    // Ntuple 0: neutron entries into sample volumes
    man->CreateNtuple("hits", "Neutron boundary crossings into sample");
    man->CreateNtupleDColumn("Ekin");   // col 0 – kinetic energy [MeV]
    man->CreateNtupleDColumn("time");   // col 1 – global time [ns]
    man->CreateNtupleDColumn("x");      // col 2 – position x [mm]
    man->CreateNtupleDColumn("y");      // col 3 – position y [mm]
    man->CreateNtupleDColumn("z");      // col 4 – position z [mm]
    man->CreateNtupleIColumn("copyNo"); // col 5 – sample copy number (0,1,2)
    man->FinishNtuple();
}

void RunAction::EndOfRunAction(const G4Run*)
{
    auto man = G4AnalysisManager::Instance();
    man->Write();
    man->CloseFile();
}

#include "RunAction.hh"
#include "G4Run.hh"

RunAction::RunAction() = default;
RunAction::~RunAction() = default;

void RunAction::BeginOfRunAction(const G4Run*)
{
    auto man = G4RootAnalysisManager::Instance();
    man->SetFileName("output");
    man->OpenFile();

    // Ntuple 0: neutron entries into sample volumes
    man->CreateNtuple("hits", "ActivationFoil");
    man->CreateNtupleSColumn("PreParticleName");// 0:例子名称
    man->CreateNtupleSColumn("Process");        // 1:物理过程
    man->FinishNtuple();
}

void RunAction::EndOfRunAction(const G4Run*)
{
    auto man = G4RootAnalysisManager::Instance();
    man->Write();
    man->CloseFile();
}

#include "RunAction.hh"
#include "Run.hh"

#include "G4Run.hh"
#include "G4RootAnalysisManager.hh"

RunAction::RunAction() = default;
RunAction::~RunAction() = default;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4Run* RunAction::GenerateRun()
{
    return new Run();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::BeginOfRunAction(const G4Run*)
{
    auto man = G4RootAnalysisManager::Instance();
    man->SetFileName("activation_output");
    man->OpenFile();

    // ------------------------------------------------------------------
    // Ntuple 0 "NeutronEntry": 中子穿入样品边界时的物理量
    //   eventId, sampleCopyNo, Ekin(MeV), time(ns),
    //   x(mm), y(mm), z(mm), px(MeV/c), py(MeV/c), pz(MeV/c)
    // ------------------------------------------------------------------
    man->CreateNtuple("NeutronEntry", "Neutron crossing into sample volume");
    man->CreateNtupleIColumn("eventId");    // 0
    man->CreateNtupleIColumn("copyNo");     // 1
    man->CreateNtupleDColumn("Ekin_MeV");  // 2
    man->CreateNtupleDColumn("time_ns");   // 3
    man->CreateNtupleDColumn("x_mm");      // 4
    man->CreateNtupleDColumn("y_mm");      // 5
    man->CreateNtupleDColumn("z_mm");      // 6
    man->CreateNtupleDColumn("px_MeV");    // 7
    man->CreateNtupleDColumn("py_MeV");    // 8
    man->CreateNtupleDColumn("pz_MeV");    // 9
    man->FinishNtuple(0);

    // ------------------------------------------------------------------
    // Ntuple 1 "NeutronInteraction": 样品内中子发生非输运过程的步骤
    //   eventId, sampleCopyNo, processName,
    //   edep(MeV), stepLen(mm), preKE(MeV), postKE(MeV),
    //   x(mm), y(mm), z(mm)
    // ------------------------------------------------------------------
    man->CreateNtuple("NeutronInteraction", "Neutron interaction steps in sample");
    man->CreateNtupleIColumn("eventId");      // 0
    man->CreateNtupleIColumn("copyNo");       // 1
    man->CreateNtupleSColumn("process");      // 2
    man->CreateNtupleDColumn("edep_MeV");     // 3
    man->CreateNtupleDColumn("stepLen_mm");   // 4
    man->CreateNtupleDColumn("preKE_MeV");    // 5
    man->CreateNtupleDColumn("postKE_MeV");   // 6
    man->CreateNtupleDColumn("x_mm");         // 7
    man->CreateNtupleDColumn("y_mm");         // 8
    man->CreateNtupleDColumn("z_mm");         // 9
    man->FinishNtuple(1);

    // ------------------------------------------------------------------
    // Ntuple 2 "Secondary": 样品内中子反应产生的次级粒子
    //   eventId, sampleCopyNo, particleName, Ekin(MeV),
    //   x(mm), y(mm), z(mm)
    // ------------------------------------------------------------------
    man->CreateNtuple("Secondary", "Secondary particles from neutron interactions");
    man->CreateNtupleIColumn("eventId");      // 0
    man->CreateNtupleIColumn("copyNo");       // 1
    man->CreateNtupleSColumn("particle");     // 2
    man->CreateNtupleDColumn("Ekin_MeV");    // 3
    man->CreateNtupleDColumn("x_mm");        // 4
    man->CreateNtupleDColumn("y_mm");        // 5
    man->CreateNtupleDColumn("z_mm");        // 6
    man->FinishNtuple(2);

    // ------------------------------------------------------------------
    // Ntuple 3 "EventSummary": 每个事件每个样品的汇总统计
    //   eventId, sampleCopyNo, nEnter, nInteract, nCapture, totalEdep(MeV)
    // ------------------------------------------------------------------
    man->CreateNtuple("EventSummary", "Per-event per-sample summary");
    man->CreateNtupleIColumn("eventId");      // 0
    man->CreateNtupleIColumn("copyNo");       // 1
    man->CreateNtupleIColumn("nEnter");       // 2
    man->CreateNtupleIColumn("nInteract");    // 3
    man->CreateNtupleIColumn("nCapture");     // 4
    man->CreateNtupleDColumn("edep_MeV");     // 5
    man->FinishNtuple(3);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::EndOfRunAction(const G4Run* run)
{
    // Write ROOT file
    auto man = G4RootAnalysisManager::Instance();
    man->Write();
    man->CloseFile();

    // Print rdecay02-style run summary to console
    const Run* theRun = static_cast<const Run*>(run);
    theRun->EndOfRun();
}

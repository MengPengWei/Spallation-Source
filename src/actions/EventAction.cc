#include "EventAction.hh"
#include "RunAction.hh"

#include "G4Event.hh"
#include "G4RootAnalysisManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4RunManager.hh"

EventAction::EventAction(RunAction*)
    : G4UserEventAction()
{}

EventAction::~EventAction()
{}

void EventAction::BeginOfEventAction(const G4Event*)
{
    // 每事件开始：重置 source 探测器累加器
    fNProtonSteps    = 0;
    fProtonEdep_MeV  = 0.;
    fNNeutronEntries = 0;
    fNLiGlassEntries = 0;
    fNLiDiamondEntries = 0;
}

void EventAction::EndOfEventAction(const G4Event* event)
{
    // 将每事件 source 探测器汇总写入 Ntuple 6 "SourceDetEvent"
    auto man = G4RootAnalysisManager::Instance();
    G4int evtId = event->GetEventID();

    man->FillNtupleIColumn(6, 0, evtId);
    man->FillNtupleIColumn(6, 1, fNProtonSteps);
    man->FillNtupleIColumn(6, 2, fNNeutronEntries);
    man->FillNtupleDColumn(6, 3, fProtonEdep_MeV);
    man->AddNtupleRow(6);

    man->FillNtupleIColumn(8, 0, evtId);
    man->FillNtupleIColumn(8, 1, fNLiGlassEntries);
    man->FillNtupleIColumn(8, 2, fNLiDiamondEntries);
    man->AddNtupleRow(8);
}

void EventAction::AddEntry(G4int /*copyNo*/)
{
}

void EventAction::AddInteraction(G4int /*copyNo*/)
{
}

void EventAction::AddCapture(G4int /*copyNo*/)
{
}

void EventAction::AddEdep(G4int /*copyNo*/, G4double /*de*/)
{
}

// ---- Source detector methods ----

void EventAction::AddProtonStep(G4double edep_MeV)
{
    ++fNProtonSteps;
    fProtonEdep_MeV += edep_MeV;
}

void EventAction::AddNeutronEntry()
{
    ++fNNeutronEntries;
}

void EventAction::AddLiGlassEntry()
{
    ++fNLiGlassEntries;
}

void EventAction::AddLiDiamondEntry()
{
    ++fNLiDiamondEntries;
}

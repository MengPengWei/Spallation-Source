#include "EventAction.hh"
#include "RunAction.hh"

#include "G4Event.hh"
#include "G4RootAnalysisManager.hh"
#include "G4SystemOfUnits.hh"

EventAction::EventAction(RunAction*)
    : G4UserEventAction()
{}

EventAction::~EventAction()
{}

void EventAction::BeginOfEventAction(const G4Event*)
{
    // 每个事件开始时重置所有样品的计数器
    for (G4int i = 0; i < C_NSamples; ++i) {
        fSamples[i] = SampleAccum{};
    }
}

void EventAction::EndOfEventAction(const G4Event* event)
{
    G4int eventId = event->GetEventID();
    auto man = G4RootAnalysisManager::Instance();

    // Ntuple 3 "EventSummary": per-event per-sample row
    for (G4int i = 0; i < C_NSamples; ++i) {
        man->FillNtupleIColumn(3, 0, eventId);
        man->FillNtupleIColumn(3, 1, i);
        man->FillNtupleIColumn(3, 2, fSamples[i].nEnter);
        man->FillNtupleIColumn(3, 3, fSamples[i].nInteract);
        man->FillNtupleIColumn(3, 4, fSamples[i].nCapture);
        man->FillNtupleDColumn(3, 5, fSamples[i].edep / MeV);
        man->AddNtupleRow(3);
    }
}

void EventAction::AddEntry(G4int copyNo)
{
    if (copyNo >= 0 && copyNo < C_NSamples)
        ++fSamples[copyNo].nEnter;
}

void EventAction::AddInteraction(G4int copyNo)
{
    if (copyNo >= 0 && copyNo < C_NSamples)
        ++fSamples[copyNo].nInteract;
}

void EventAction::AddCapture(G4int copyNo)
{
    if (copyNo >= 0 && copyNo < C_NSamples)
        ++fSamples[copyNo].nCapture;
}

void EventAction::AddEdep(G4int copyNo, G4double de)
{
    if (copyNo >= 0 && copyNo < C_NSamples)
        fSamples[copyNo].edep += de;
}

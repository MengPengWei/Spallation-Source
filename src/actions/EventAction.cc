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
    
}

void EventAction::EndOfEventAction(const G4Event* event)
{

}

void EventAction::AddEntry(G4int copyNo)
{

}

void EventAction::AddInteraction(G4int copyNo)
{

}

void EventAction::AddCapture(G4int copyNo)
{

}

void EventAction::AddEdep(G4int copyNo, G4double de)
{

}

#include "EventAction.hh"
#include "RunAction.hh"

#include "G4Event.hh"
#include "G4Run.hh"
#include "G4UnitsTable.hh"

EventAction::EventAction(RunAction* run)
    : G4UserEventAction()
{
}
EventAction::~EventAction()
{
}

void EventAction::BeginOfEventAction(const G4Event*)
{
    // 可以在这里初始化事件相关的数据
}

void EventAction::EndOfEventAction(const G4Event* event)
{
    
}
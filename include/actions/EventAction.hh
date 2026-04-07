#ifndef EventAction_h
#define EventAction_h

#include "G4UserEventAction.hh"

class RunAction;

class EventAction : public G4UserEventAction
{
public:
    EventAction(RunAction*);
    ~EventAction() override;

    void BeginOfEventAction(const G4Event*) override;
    void EndOfEventAction(const G4Event*) override;
};

#endif
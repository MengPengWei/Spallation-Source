#ifndef RunAction_h
#define RunAction_h

#include "G4UserRunAction.hh"
#include "G4RootAnalysisManager.hh"

class RunAction : public G4UserRunAction
{
public:
    RunAction();
    ~RunAction() override;

    G4Run* GenerateRun() override;

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;
};

#endif

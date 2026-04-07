#ifndef RunAction_h
#define RunAction_h

#include "G4UserRunAction.hh"
#include "G4RootAnalysisManager.hh"
#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"
class RunAction : public G4UserRunAction
{
public:
    RunAction();
    ~RunAction() override;

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;

private:
    DetectorConstruction* fDetector = nullptr;
    PrimaryGeneratorAction* fPrimary = nullptr;
    //Run* fRun = nullptr;
    //HistoManager* fHistoManager = nullptr;

};

#endif